/*
 * File:        MDSocket.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-21
 * Description: Receive MarketData by WebSocket from Binance
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <chrono>
// STL
#include <unordered_map>
// 3rd-lib
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>
// QTP
#include "Macro.h"
#include "MDRing.h"
#include "MDSocket.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern double lastPriceArr[TOTAL_SYMBOL];
extern MDRing mdring[TOTAL_SYMBOL];

extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;

// Static
int MDSocket::mMsgCnt = 0;
std::string MDSocket::mMdUrl;
WSSClient MDSocket::mWSSClient;
websocketpp::connection_hdl MDSocket::mConnHdl;

//##################################################//
//   Constructor
//##################################################//
MDSocket::MDSocket(const std::string& url)
{
    mMdUrl = url;

    try
    {
        // 设置日志输出等级
        mWSSClient.set_access_channels(websocketpp::log::alevel::all);
        mWSSClient.clear_access_channels(websocketpp::log::alevel::frame_payload);
        // 初始化ASIO
        mWSSClient.init_asio();
        // 回调函数需要声明static
        // 注册TLS初始化回调
        mWSSClient.set_tls_init_handler(bind(&OnTlsInit));
        // 注册WS消息处理回调
        mWSSClient.set_message_handler(&OnMessage);
        // 注册WS连接回调
        mWSSClient.set_open_handler(&OnOpen);
        // 注册WS失败回调
        mWSSClient.set_fail_handler(&OnFail);
        // 注册WS关闭回调
        mWSSClient.set_close_handler(&OnClose);
    }
    catch( websocketpp::exception const& e )
    {
        sptrAsyncLogger->error("MDSocket::Init() WSError: {}", e.what());
    }
    catch( const std::exception& e )
    {
        sptrAsyncLogger->error("MDSocket::Init() Error: {}", e.what());
    }
}

//##################################################//
//   Destructor
//##################################################//
MDSocket::~MDSocket()
{
    // ~
}

//##################################################//
//   线程运行实体
//##################################################//
void MDSocket::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 1;   // 1s
    time_to_sleep.tv_nsec = 0;

    while(1)
    {
        try
        {
            // 获取WebSocket连接
            websocketpp::lib::error_code ec;
            WSSClient::connection_ptr con_ptr = mWSSClient.get_connection(mMdUrl, ec);
            if( ec )
            {
                sptrAsyncLogger->error("MDSocket::Run() Error: {}", ec.message());
                return;
            }
            // 连接到服务器
            mWSSClient.connect(con_ptr);
            // 开始ASIO io_service事件循环
            mWSSClient.run();
        }
        catch( websocketpp::exception const& e )
        {
            sptrAsyncLogger->error("MDSocket::Run() WSError: {}", e.what());
        }
        catch( const std::exception& e )
        {
            sptrAsyncLogger->error("MDSocket::Run() Error: {}", e.what());
        }

        // Sleep 1s
        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("WatchDog::Run() nanosleep() failed !");
        
        // 重新初始化
        ReInit();
    }
}

//##################################################//
//   TLS初始化回调函数
//##################################################//
ContextSPtr MDSocket::OnTlsInit()
{
    // 创建TLS上下文
    ContextSPtr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
    try
    {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    }
    catch( std::exception& e )
    {
        sptrAsyncLogger->error("MDSocket::OnTlsInit() Error: {}", e.what());
    }
    return ctx;
}

//##################################################//
//   WebSocket消息回调函数
//##################################################//
void MDSocket::OnMessage(websocketpp::connection_hdl, WSSClient::message_ptr msg)
{
    //sptrAsyncLogger->info("MDSocket::OnMessage() {}", msg->get_payload());

    if( (mMsgCnt++)%3 )
        return;

    rapidjson::Document jsondoc;
    rapidjson::ParseResult jsonret = jsondoc.Parse(msg->get_payload().c_str());
    if(jsonret)
    {
        if( !jsondoc.IsObject() )
        {
            sptrAsyncLogger->error("MDSocket::OnMessage() jsondoc is not Object !");
            return;
        }

        const rapidjson::Value& symbols = jsondoc["data"];
        if( !symbols.IsArray() || symbols.Empty() )
        {
            sptrAsyncLogger->error("MDSocket::OnMessage() data is not Array !");
            return;
        }

        for(const auto& item : symbols.GetArray())
        {
            std::string str_symbol = item["s"].GetString();
            std::string str_price = item["c"].GetString();
            double db_price = stod(str_price);
            if(symbol2idxUMap.find(str_symbol) != symbol2idxUMap.end())
            {
                int symbol_idx = symbol2idxUMap[str_symbol];
                //mdring[symbol_idx].PushMD(db_price);
                lastPriceArr[symbol_idx] = db_price;
            }
        }

        for(const auto& symbol_iter:symbol2idxUMap)
        {
            mdring[symbol_iter.second].PushMD(lastPriceArr[symbol_iter.second]);
        }
    }
    else
    {
        sptrAsyncLogger->error("MDSocket::OnMessage() jsondoc Parse() Error !");
    }
}

//##################################################//
//   WebSocket连接回调函数
//##################################################//
void MDSocket::OnOpen(websocketpp::connection_hdl hdl)
{
    sptrAsyncLogger->info("MDSocket::OnOpen() Called");
    mConnHdl = hdl;
}

//##################################################//
//   WebSocket失败回调函数
//##################################################//
void  MDSocket::OnFail(websocketpp::connection_hdl hdl)
{
    sptrAsyncLogger->info("MDSocket::OnFail() Called");
    // Bad Connection
    //websocketpp::lib::error_code ec;
    //mWSSClient.close(mConnHdl, websocketpp::close::status::normal, "", ec);
    //if( ec )
    //    sptrAsyncLogger->error("MDSocket::OnFail() Error: {}", ec.message());
    //mWSSClient.reset();
}

//##################################################//
//   WebSocket关闭回调函数
//##################################################//
void MDSocket::OnClose(websocketpp::connection_hdl hdl)
{
    sptrAsyncLogger->info("MDSocket::OnClose() Called");
    // invalid state
    //websocketpp::lib::error_code ec;
    //mWSSClient.close(mConnHdl, websocketpp::close::status::normal, "", ec);
    //if( ec )
    //    sptrAsyncLogger->error("MDSocket::OnClose() Error: {}", ec.message());
    //mWSSClient.reset();
}

//##################################################//
//   WebSocket重新初始化
//##################################################//
void MDSocket::ReInit()
{
    try
    {
        // 必须首先重置
        mWSSClient.reset();
        // 设置日志输出等级
        mWSSClient.set_access_channels(websocketpp::log::alevel::all);
        mWSSClient.clear_access_channels(websocketpp::log::alevel::frame_payload);
        // 初始化ASIO
        mWSSClient.init_asio();
        // 回调函数需要声明static
        // 注册TLS初始化回调
        mWSSClient.set_tls_init_handler(bind(&OnTlsInit));
        // 注册WS消息处理回调
        mWSSClient.set_message_handler(&OnMessage);
        // 注册WS连接回调
        mWSSClient.set_open_handler(&OnOpen);
        // 注册WS失败回调
        mWSSClient.set_fail_handler(&OnFail);
        // 注册WS关闭回调
        mWSSClient.set_close_handler(&OnClose);
    }
    catch( websocketpp::exception const& e )
    {
        sptrAsyncLogger->error("MDSocket::ReInit() WSError: {}", e.what());
    }
    catch( const std::exception& e )
    {
        sptrAsyncLogger->error("MDSocket::ReInit() Error: {}", e.what());
    }
}
