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
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>
// QTP
#include "Utils.h"
#include "Macro.h"
#include "BiFilter.h"
#include "MDRing.h"
#include "MDSocket.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern Binance::miniTicker miniTickerArr[TOTAL_SYMBOL];
extern MDRing mdring[TOTAL_SYMBOL];
// AsyncLogger
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;
extern std::shared_ptr<spdlog::async_logger> sptrAsyncOuter;

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
        sptrAsyncLogger->error("MDSocket::MDSocket() WSError: {}", e.what());
    }
    catch( const std::exception& e )
    {
        sptrAsyncLogger->error("MDSocket::MDSocket() Error: {}", e.what());
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

    rapidjson::Document jsondoc;
    rapidjson::ParseResult jsonret = jsondoc.Parse(msg->get_payload().c_str());
    if(jsonret)
    {
        // 确认jsondoc是一个对象
        if( !jsondoc.IsObject() )
        {
            sptrAsyncLogger->error("MDSocket::OnMessage() jsondoc is not Object !");
            return;
        }

        // 确认data是一个非空数组
        const rapidjson::Value& symbols = jsondoc["data"];
        if( !symbols.IsArray() || symbols.Empty() )
        {
            sptrAsyncLogger->error("MDSocket::OnMessage() data is not Array !");
            return;
        }

        // 每1s接收实时行情
        int64_t timestamp;
        std::string symbol;
        std::string price;
        std::string volume;
        std::string amount;
        for(const auto& item : symbols.GetArray())
        {
            if( item.HasMember("E") )
                timestamp = item["E"].GetInt64();
            if( item.HasMember("s") )
                symbol = item["s"].GetString();
            if( item.HasMember("c") )
                price = item["c"].GetString();
            if( item.HasMember("v") )
                volume = item["v"].GetString();
            if( item.HasMember("q") )
                amount = item["q"].GetString();
            
            if(symbol2idxUMap.find(symbol) != symbol2idxUMap.end())
            {
                int symbol_idx = symbol2idxUMap[symbol];
                miniTickerArr[symbol_idx].timestamp = timestamp;
                miniTickerArr[symbol_idx].symbol = symbol;
                miniTickerArr[symbol_idx].price = price;
                miniTickerArr[symbol_idx].volume = volume;
                miniTickerArr[symbol_idx].amount = amount;
            }
        }

        // 每1s落地Outer
        for(const auto& symbol_iter:symbol2idxUMap)
        {
            // think@summer
            miniTickerArr[symbol_iter.second].timestamp = Utils::Timer::GetMillisecondTimestamp();
            sptrAsyncOuter->info("{},{},{},{},{}", miniTickerArr[symbol_iter.second].timestamp,
                                                   miniTickerArr[symbol_iter.second].symbol,
                                                   miniTickerArr[symbol_iter.second].price,
                                                   miniTickerArr[symbol_iter.second].volume,
                                                   miniTickerArr[symbol_iter.second].amount);
        }

        // 每3s推送RingMD
        if( (mMsgCnt++)%3 )
            return;
        else
        {
            for(const auto& symbol_iter:symbol2idxUMap)
            {
                mdring[symbol_iter.second].PushMD(std::stod(miniTickerArr[symbol_iter.second].price));
            }
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
        // 首先必须重置 !
        // 否则状态报错 !
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
