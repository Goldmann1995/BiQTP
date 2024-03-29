/*
 * File:        BiTrader.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Binance Trader
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
extern std::shared_ptr<spdlog::async_logger> sptrAsyncOuter;

// Static
std::string BiTrader::mCurlBuffer;


//##################################################//
//   Constructor
//##################################################//
BiTrader::BiTrader(const std::string& url, const std::string& api_key, const std::string& secret_key)
{
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
        sptrAsyncLogger->error("BiTrader::BiTrader() curl_easy_init() Failed: {}", \
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
//   币安报单
//##################################################//
bool BiTrader::InsertOrder(std::string symbol, \
                           Binance::OrderSide side, \
                           double price, \
                           double qty, \
                           Binance::OrderType type, \
                           Binance::TimeInForce tif, \
                           double& exe_price, \
                           double& exe_qty, \
                           double& commission_qty)
{
#if !_BACK_TEST_
    // 设置Order参数
    int index = symbol2idxUMap[symbol];
    std::string order_side = GetOrderSide(side);
    std::string order_type = GetOrderType(type);
    std::string order_tif = GetTimeInForce(tif);
    std::string order_price = StringUtils::ConvertDouble2String(price, symbolFilterArr[index].GetTickSize());
    std::string order_qty = StringUtils::ConvertDouble2String(qty, symbolFilterArr[index].GetStepSize());
    if( price < symbolFilterArr[index].GetMinPrice() || 
        price > symbolFilterArr[index].GetMaxPrice() )
    {
        if(type!=Binance::OrderType::MARKET)
        {
            sptrAsyncLogger->error("BiTrader::InsertOrder() Price Error: {}", price);
            return false;
        }
    }
    if( qty < symbolFilterArr[index].GetMinQty() || 
        qty > symbolFilterArr[index].GetMaxQty() )
    {
        sptrAsyncLogger->error("BiTrader::InsertOrder() Qty Error: {}", qty);
        return false;
    }
    if( price*qty < symbolFilterArr[index].GetMinNotional() || 
        price*qty > symbolFilterArr[index].GetMaxNotional() )
    {
        sptrAsyncLogger->error("BiTrader::InsertOrder() Notional Error: {}", price*qty);
        return false;
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
        sptrAsyncLogger->error("BiTrader::InsertOrder() curl_easy_perform() Failed: {}", \
                                curl_easy_strerror(mCurlCode));
        sptrAsyncOuter->info("{}", curl_easy_strerror(mCurlCode));
        mCurlBuffer.clear();
        return false;
    }
    else
    {
        sptrAsyncLogger->info("BiTrader::InsertOrder() curl_easy_perform() Success");
        sptrAsyncOuter->info("{}", mCurlBuffer);
        ParseInsertResp(mCurlBuffer, exe_price, exe_qty, commission_qty);
        mCurlBuffer.clear();
        return true;
    }
#else
    return true;
#endif
}

//##################################################//
//   解析报单回报
//##################################################//
void BiTrader::ParseInsertResp(std::string rsp, double& exe_price, double& exe_qty, double& commission_qty)
{
    rapidjson::Document jsondoc;
    rapidjson::ParseResult jsonret = jsondoc.Parse(rsp.c_str());
    // 确认没有解析出错
    if( jsonret )
    {
        // 确认jsondoc是一个对象
        if( !jsondoc.IsObject() )
        {
            sptrAsyncLogger->error("BiTrader::ParseInsertResp() jsondoc is not Object !");
            return;
        }

        /*** 报错处理 ***/
        int code = 0;
        std::string reason = "";
        if( jsondoc.HasMember("code") )
        {
            code = jsondoc["code"].GetInt();
            if( jsondoc.HasMember("msg") )
                reason = jsondoc["msg"].GetString();
            sptrAsyncLogger->error("BiTrader::ParseInsertResp() ErrorRsp: {} {} ", code, reason);
            return;
        }

        /*** 常规处理 ***/
        std::string symbol = "";
        double cummulative_qty = 0.0;
        if( jsondoc.HasMember("symbol") )
            symbol = jsondoc["symbol"].GetString();
        if( jsondoc.HasMember("cummulativeQuoteQty") )
            cummulative_qty = std::stod(jsondoc["cummulativeQuoteQty"].GetString());
        if( jsondoc.HasMember("executedQty") )
            exe_qty = std::stod(jsondoc["executedQty"].GetString());
        exe_price = cummulative_qty/exe_qty;

        // 访问fills数组
        const rapidjson::Value& fills = jsondoc["fills"];
        if( !fills.IsArray() || fills.Empty() )
        {
            sptrAsyncLogger->error("BiTrader::ParseInsertResp() fills is not Array !");
            return;
        }

        // 访问fills数组
        for(const auto& fill : fills.GetArray())
        {
            std::string comm_symbol = "";
            if( fill.HasMember("commissionAsset") )
                comm_symbol = fill["commissionAsset"].GetString();
            if( StringUtils::IsSpecStarting(symbol, comm_symbol) )
                commission_qty += std::stod(fill["commission"].GetString());
        }
    }
    else
    {
        sptrAsyncLogger->error("BiTrader::ParseInsertResp() rapidjson Parse() Error !");
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
