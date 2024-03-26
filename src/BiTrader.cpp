/*
 * File:        BiTrader.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Order Manager
 */

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
// STL
#include <vector>
#include <unordered_map>
// 3rd-lib
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// QTP
#include "Utils/CryptoUtils.hpp"
#include "Utils/StringUtils.hpp"
#include "BiDef.h"
#include "BiFilter.h"
#include "Macro.h"
#include "BiTrader.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern Binance::SymbolFilter symbolFilterArr[TOTAL_SYMBOL];
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;

// Static
std::string BiTrader::mCurlBuffer;

//##################################################//
//   Constructor
//##################################################//
BiTrader::BiTrader(const std::string& url, const std::string& api_key, const std::string& secret_key)
{
    // strategy
    TdUnit = 100.0;

    // key初始化
    mTdUrl = url;
    mTdApiKey = api_key;
    mTdSecretKey = secret_key;

    // curl初始化
    mTdCurl = curl_easy_init();
    if(mTdCurl)
    {
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, ("X-MBX-APIKEY: " + mTdApiKey).c_str());
        curl_easy_setopt(mTdCurl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(mTdCurl, CURLOPT_URL, (mTdUrl+"/api/v3/order").c_str());
        curl_easy_setopt(mTdCurl, CURLOPT_WRITEFUNCTION, TDWriteCallback);
        curl_easy_setopt(mTdCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
    }
    else
    {
        sptrAsyncLogger->error("BiTrader::BiTrader() curl_easy_init() failed: {}", \
                                curl_easy_strerror(mCurlCode));
    }
}

//##################################################//
//   Destructor
//##################################################//
BiTrader::~BiTrader()
{
    curl_easy_cleanup(mTdCurl);
}

//##################################################//
//   线程运行实体
//##################################################//
void BiTrader::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 1;   // 1s
    time_to_sleep.tv_nsec = 0;

	while( true )
	{
        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("BiTrader::Run() nanosleep() failed !");
	}
}

//##################################################//
//   策略报单信号
//##################################################//
void BiTrader::StInsertSignal(int strategyid, \
                              std::string symbol, \
                              Binance::OrderSide side, \
                              double price )
{
    if(side==Binance::OrderSide::BUY)
    {
        double qty = TdUnit/price;
        InsertOrder(strategyid, symbol, side, price, qty, Binance::OrderType::MARKET, Binance::TimeInForce::GTC);
    }
    else if(side==Binance::OrderSide::SELL)
    {
        double qty = 0.0;
        for(auto& order_block : mStrategyOrders[strategyid])
        {
            if(order_block.symbol==symbol)
                qty = order_block.totalQty - order_block.commissionQty;
        }
        InsertOrder(strategyid, symbol, side, price, qty, Binance::OrderType::MARKET, Binance::TimeInForce::GTC);
    }
    else
        sptrAsyncLogger->error("BiTrader::StInsertSignal() Insert side Error !");
}

//##################################################//
//   币安报单
//##################################################//
void BiTrader::InsertOrder(int strategyid, \
                           std::string symbol, \
                           Binance::OrderSide side, \
                           double price, \
                           double qty, \
                           Binance::OrderType type, \
                           Binance::TimeInForce tif)
{
    // 设置Order参数
    int index = symbol2idxUMap[symbol];
    std::string order_side = GetOrderSide(side);
    std::string order_type = GetOrderType(type);
    std::string order_tif = GetTimeInForce(tif);
    std::string order_price = StringUtils::double2string(price, symbolFilterArr[index].GetTickSize());
    std::string order_qty = StringUtils::double2string(qty, symbolFilterArr[index].GetStepSize());
    if( price < symbolFilterArr[index].GetMinPrice() || 
        price > symbolFilterArr[index].GetMaxPrice() )
    {
        sptrAsyncLogger->error("BiTrader::InsertOrder() Price Error: {}", price);
        return;
    }
    if( qty < symbolFilterArr[index].GetMinQty() || 
        qty > symbolFilterArr[index].GetMaxQty() )
    {
        sptrAsyncLogger->error("BiTrader::InsertOrder() Qty Error: {}", qty);
        return;
    }
    if( price*qty < symbolFilterArr[index].GetMinNotional() || 
        price*qty > symbolFilterArr[index].GetMaxNotional() )
    {
        sptrAsyncLogger->error("BiTrader::InsertOrder() Notional Error: {}", price*qty);
        return;
    }
    std::string post_param = "symbol=" + symbol + "&";
    post_param += "side=" + order_side + "&";
    post_param += "type=" + order_type + "&";
    if(type==Binance::OrderType::LIMIT)
        post_param += "timeInForce=" + order_tif + "&";
    if(type!=Binance::OrderType::MARKET)
        post_param += "price=" + order_price + "&";
    post_param += "quantity=" + order_qty + "&";

    // 设置Curl参数
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::string req = post_param+"timestamp="+std::to_string(millis);
    std::string signature = CryptoUtils::GetHMAC_SHA256(mTdSecretKey, req);
    std::string data = req + "&signature=" + signature;
    // curl配置
    curl_easy_setopt(mTdCurl, CURLOPT_POSTFIELDS, data.c_str());

    // 执行POST请求
    mCurlCode = curl_easy_perform(mTdCurl);
    if(mCurlCode != CURLE_OK)
    {
        sptrAsyncLogger->error("BiTrader::InsertOrder() curl_easy_perform() failed: {}", \
                                curl_easy_strerror(mCurlCode));
    }
    else
    {
        sptrAsyncLogger->info("BiTrader::InsertOrder() curl_easy_perform() success: {}", \
                                mCurlBuffer);
        ParseInsertResponse(strategyid, mCurlBuffer);
    }

    mCurlBuffer.clear();
}

//##################################################//
//   ~
//##################################################//
void BiTrader::ParseInsertResponse(int strategyid, std::string rsp)
{
    rapidjson::Document jsondoc;
    rapidjson::ParseResult jsonret = jsondoc.Parse(rsp.c_str());
    // 确认没有解析出错
    if( jsonret )
    {
        // 确认jsondoc是一个对象
        if( !jsondoc.IsObject() )
        {
            sptrAsyncLogger->error("BiTrader::ParseInsertResponse() jsondoc is not Object !");
            return;
        }

        std::string symbol("");
        int64_t order_id = 0;
        std::string side("");
        if( jsondoc.HasMember("symbol") )
            symbol = jsondoc["symbol"].GetString();
        if( jsondoc.HasMember("orderId") )
            order_id = jsondoc["orderId"].GetInt64();
        if( jsondoc.HasMember("side") )
            side = jsondoc["side"].GetString();

        // 访问fills数组
        const rapidjson::Value& fills = jsondoc["fills"];
        if( !fills.IsArray() || fills.Empty() )
        {
            sptrAsyncLogger->error("BiTrader::ParseInsertResponse fills is not Array !");
            return;
        }

        // 访问fills数组
        for(const auto& fill : fills.GetArray())
        {
            //commissionQty
        }

        if(side=="BUY")
        {
            OrderBlock order_block;
            order_block.symbol = symbol;
            order_block.orderId = order_id;
            // TODO
            mStrategyOrders[strategyid].push_back(order_block);
        }
        else if(side=="SELL")
        {
            for( auto it=mStrategyOrders[strategyid].begin(); it!=mStrategyOrders[strategyid].end(); it++ )
            {
                if( it->symbol==symbol)
                {
                    mStrategyOrders[strategyid].erase(it);
                    break;
                }
            }
        }
    }
    else
    {
        sptrAsyncLogger->error("BiHelper::InitSymbolIdxMap() rapidjson Parse() Error !");
    }
}

//##################################################//
//   curl回调函数
//##################################################//
size_t BiTrader::TDWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
