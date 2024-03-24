/*
 * File:        BiHelper.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-23
 * Description: Get ExchInfo from Binance
 */

#include <unistd.h>
#include <iostream>
#include <fstream>
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
#include "Utils.h"
#include "Macro.h"
#include "BiFilter.h"
#include "MDRing.h"
#include "BiHelper.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern Binance::SymbolFilter symbolFilterArr[TOTAL_SYMBOL];
extern Binance::miniTicker miniTickerArr[TOTAL_SYMBOL];
extern MDRing mdring[TOTAL_SYMBOL];
// AsyncLogger
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;
extern std::shared_ptr<spdlog::async_logger> sptrAsyncOuter;


// Static
std::string BiHelper::mCurlBuffer;

//##################################################//
//   Constructor
//##################################################//
BiHelper::BiHelper(const std::string& url, const std::string& path)
{
    mInfoPath = path;
    // curl初始化
    mHelpUrl = url;
    mHelpCurl = curl_easy_init();
    if(mHelpCurl)
    {
        curl_easy_setopt(mHelpCurl, CURLOPT_WRITEFUNCTION, HelpWriteCallback);
        curl_easy_setopt(mHelpCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
        curl_easy_setopt(mHelpCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
    else
    {
        sptrAsyncLogger->error("BiHelper::BiHelper() curl_easy_init() failed: {}", \
                                curl_easy_strerror(mCurlCode));
    }
}

//##################################################//
//   Destructor
//##################################################//
BiHelper::~BiHelper()
{
    curl_easy_cleanup(mHelpCurl);
}

//##################################################//
//   初始化所有币种IndexMap
//##################################################//
void BiHelper::InitSymbolIdxMap()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 3;   // 3s to reconnect
    time_to_sleep.tv_nsec = 0;

    // curl配置
    bool init_flag = false;
    std::string url = mHelpUrl + "/api/v3/ticker/24hr";
    curl_easy_setopt(mHelpCurl, CURLOPT_URL, url.c_str());    
    while( !init_flag )
    {
        // 执行GET请求
        mCurlCode = curl_easy_perform(mHelpCurl);
        if(mCurlCode != CURLE_OK)
        {
            sptrAsyncLogger->error("BiHelper::InitSymbolIdxMap() curl_easy_perform() failed: {}", \
                                    curl_easy_strerror(mCurlCode));
        }
        else
        {
            init_flag = true;
            int index = 0;
            rapidjson::Document jsondoc;
            rapidjson::ParseResult jsonret = jsondoc.Parse(mCurlBuffer.c_str());
            // 确认没有解析出错
            if( jsonret && jsondoc.IsArray() )
            {
                for(const auto& item:jsondoc.GetArray())
                {
                    std::string symbol;
                    std::string volume;
                    if( item.HasMember("symbol") )
                        symbol = item["symbol"].GetString();
                    if( item.HasMember("volume") )
                        volume = item["volume"].GetString();
                    // 过滤不活跃币种
                    if(volume == "0.00000000")
                        continue;
                    // 过滤USDT币种对
                    std::string ending = "USDT";
                    if( symbol.find("USDT") != std::string::npos && \
                        symbol.compare(symbol.length()-ending.length(), ending.length(), ending) == 0)
                    {
                        symbol2idxUMap.insert(make_pair(symbol, index));
                        symbolFilterArr[index].SetSymbolName(symbol);
                        mdring[index].SetSymbolName(symbol);
                        sptrAsyncLogger->info("BiHelper::InitSymbolIdxMap() Symbol: {} Volume: {} -> Index: {}", symbol, volume, index);
                        index++;
                    }
                }
            }
            else
            {
                sptrAsyncLogger->error("BiHelper::InitSymbolIdxMap() rapidjson Parse() Error !");
            }

            mCurlBuffer.clear();
        }

        // 重连延迟
        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
        {
            sptrAsyncLogger->error("BiHelper::InitSymbolIdxMap() nanosleep() failed !");
        }
    }  
}

//##################################################//
//   初始化所有币种Outer
//##################################################//
void BiHelper::InitSymbolOuter()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 3;   // 3s to reconnect
    time_to_sleep.tv_nsec = 0;

    // curl配置
    bool init_flag = false;
    std::string url = mHelpUrl + "/api/v3/ticker/24hr";
    curl_easy_setopt(mHelpCurl, CURLOPT_URL, url.c_str());    
    while( !init_flag )
    {
        // 执行GET请求
        mCurlCode = curl_easy_perform(mHelpCurl);
        if(mCurlCode != CURLE_OK)
        {
            sptrAsyncLogger->error("BiHelper::InitSymbolOuter() curl_easy_perform() failed: {}", \
                                    curl_easy_strerror(mCurlCode));
        }
        else
        {
            init_flag = true;
            rapidjson::Document jsondoc;
            rapidjson::ParseResult jsonret = jsondoc.Parse(mCurlBuffer.c_str());
            // 确认没有解析出错
            if( jsonret && jsondoc.IsArray() )
            {
                for(const auto& item:jsondoc.GetArray())
                {
                    std::string symbol;
                    std::string price;
                    std::string volume;
                    std::string amount;
                    int64_t timestamp = Utils::Timer::GetMillisecondTimestamp();
                    if( item.HasMember("symbol") )
                        symbol = item["symbol"].GetString();
                    if( item.HasMember("lastPrice") )
                        price = item["lastPrice"].GetString();
                    if( item.HasMember("volume") )
                        volume = item["volume"].GetString();
                    if( item.HasMember("quoteVolume") )
                        amount = item["quoteVolume"].GetString();
                    
                    if(symbol2idxUMap.find(symbol) != symbol2idxUMap.end())
                    {
                        int symbol_idx = symbol2idxUMap[symbol];
                        miniTickerArr[symbol_idx].timestamp = timestamp;
                        miniTickerArr[symbol_idx].symbol = symbol;
                        miniTickerArr[symbol_idx].price = price;
                        miniTickerArr[symbol_idx].volume = volume;
                        miniTickerArr[symbol_idx].amount = amount;

                        sptrAsyncOuter->info("{},{},{},{},{}", miniTickerArr[symbol_idx].timestamp,
                                                               miniTickerArr[symbol_idx].symbol,
                                                               miniTickerArr[symbol_idx].price,
                                                               miniTickerArr[symbol_idx].volume,
                                                               miniTickerArr[symbol_idx].amount);
                    }
                }
            }
            else
            {
                sptrAsyncLogger->error("BiHelper::InitSymbolOuter() rapidjson Parse() Error !");
            }

            mCurlBuffer.clear();
        }

        // 重连延迟
        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
        {
            sptrAsyncLogger->error("BiHelper::InitSymbolIdxMap() nanosleep() failed !");
        }
    }
}

//##################################################//
//   请求所有币种Filter
//##################################################//
void BiHelper::RequestSymbolFilter()
{
    int req_cnt = 0;
    // 遍历SymbolMap
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        sptrAsyncLogger->info("BiHelper::RequestSymbolFilter() Cnt: {} -> Symbol: {}", \
                               req_cnt++, symbol_iter.first);

        // 打开文件流
        std::string filename = mInfoPath + symbol_iter.first + ".json";
        std::ofstream outputFile(filename, std::ios::out);

        // curl配置
        std::string url = mHelpUrl + "/api/v3/exchangeInfo?symbol=" + symbol_iter.first;
        curl_easy_setopt(mHelpCurl, CURLOPT_URL, url.c_str());

        // 执行GET请求
        mCurlCode = curl_easy_perform(mHelpCurl);
        if(mCurlCode != CURLE_OK)
        {
            sptrAsyncLogger->error("BiHelper::RequestSymbolFilter() curl_easy_perform() failed: {}", \
                                    curl_easy_strerror(mCurlCode));
            return;
        }
        else
        {
            if( outputFile.is_open() )
            {
                outputFile << mCurlBuffer;
                outputFile.close();
                mCurlBuffer.clear();

                // TODO
                sleep(1);
            }
            else
            {
                sptrAsyncLogger->error("BiHelper::RequestSymbolFilter() {} open failed !", symbol_iter.first);
                return;
            }
        }
    }
}

//##################################################//
//   初始化所有币种Filter
//##################################################//
void BiHelper::InitSymbolFilter()
{
    // 遍历SymbolMap
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        // 打开文件流
        std::string filename = mInfoPath + symbol_iter.first + ".json";
        std::ifstream inputFile(filename, std::ios::in);

        if( inputFile.is_open() )
        {
            std::string jsonBuffer;
            while( getline(inputFile, jsonBuffer) )
                continue;
            inputFile.close();

            rapidjson::Document jsondoc;
            rapidjson::ParseResult jsonret = jsondoc.Parse(jsonBuffer.c_str());
            if( jsonret )
            {
                // 确认jsondoc是一个对象
                if( !jsondoc.IsObject() )
                {
                    sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} jsondoc is not Object !", symbol_iter.first);
                    return;
                }

                // 访问symbols数组
                const rapidjson::Value& symbols = jsondoc["symbols"];
                if( !symbols.IsArray() || symbols.Empty() )
                {
                    sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} symbols is not Array !", symbol_iter.first);
                    return;
                }

                const rapidjson::Value& symbol = symbols[0]["symbol"];
                if( !symbol.IsString() )
                {
                    sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} symbol is not String !", symbol_iter.first);
                    return;
                }

                const rapidjson::Value& types = symbols[0]["orderTypes"];
                if( !symbol.IsString() )
                {
                    sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} orderTypes is not Array !", symbol_iter.first);
                    return;
                }

                for(const auto& filter : types.GetArray())
                {
                    symbolFilterArr[symbol_iter.second].AddOrderType(filter.GetString());
                }
                
                // 访问第一个符号的filters数组
                const rapidjson::Value& filters = symbols[0]["filters"];
                if( !filters.IsArray() )
                {
                    sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} filters is not Array !", symbol_iter.first);
                    return;
                }

                // 遍历filters数组
                for(const auto& filter : filters.GetArray())
                {
                    if (std::string(filter["filterType"].GetString()) == "PRICE_FILTER")
                    {
                        symbolFilterArr[symbol_iter.second].SetMinPrice(filter["minPrice"].GetString());
                        symbolFilterArr[symbol_iter.second].SetMaxPrice(filter["maxPrice"].GetString());
                        symbolFilterArr[symbol_iter.second].SetTickSize(filter["tickSize"].GetString());
                    }
                    else if (std::string(filter["filterType"].GetString()) == "LOT_SIZE")
                    {
                        symbolFilterArr[symbol_iter.second].SetMinQty(filter["minQty"].GetString());
                        symbolFilterArr[symbol_iter.second].SetMaxQty(filter["maxQty"].GetString());
                        symbolFilterArr[symbol_iter.second].SetStepSize(filter["stepSize"].GetString());
                    }
                    else if (std::string(filter["filterType"].GetString()) == "NOTIONAL")
                    {
                        symbolFilterArr[symbol_iter.second].SetMinNotional(filter["minNotional"].GetString());
                        symbolFilterArr[symbol_iter.second].SetMaxNotional(filter["maxNotional"].GetString());
                    }
                }
            }
            else
            {
                sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} rapidjson Parse() Error !", symbol_iter.first);
                return;
            }

            // Print
            //symbolFilterArr[symbol_iter.second].PrintExchInfo();
        }
        else
        {
            sptrAsyncLogger->error("BiHelper::InitSymbolFilter() {} open failed !", symbol_iter.first);
            continue;
        }
    }
}

//##################################################//
//   curl回调函数
//##################################################//
size_t BiHelper::HelpWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
