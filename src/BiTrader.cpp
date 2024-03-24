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
#include <unordered_map>
// 3rd-lib
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// QTP
#include <Utils.h>
#include <BiDef.h>
#include <BiFilter.h>
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
    time_to_sleep.tv_sec  = 3;   // 3s
    time_to_sleep.tv_nsec = 0;

	while( true )
	{
        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("OrderManager::Run() nanosleep() failed !");
	}
}

//##################################################//
//   报单
//##################################################//
void BiTrader::InsertOrder(std::string symbol, \
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
    std::string order_price = double2string(price, symbolFilterArr[index].GetTickSize());
    std::string order_qty = double2string(qty, symbolFilterArr[index].GetStepSize());
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
    post_param += "price=" + order_price + "&";
    post_param += "quantity=" + order_qty + "&";

    // 设置Curl参数
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::string req = post_param+"timestamp="+std::to_string(millis);
    std::string signature = Utils::SSL::GetHMAC_SHA256(mTdSecretKey, req);
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
    }

    mCurlBuffer.clear();
}

//##################################################//
//   ~
//##################################################//
std::string BiTrader::double2string(double value, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
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
