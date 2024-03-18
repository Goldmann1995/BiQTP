/*
 * File:        BiIniter.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: Initialization from Binance
 */

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <unordered_map>

#include <curl/curl.h>
#include <rapidjson/document.h>
#include <log/log.h>

#include "BiIniter.h"
#include "RingBuffer.hpp"
#include "MDRing.h"

#define TOTAL_SYMBOL   600

extern std::unordered_map<std::string, int> symbolUMap;
extern MDRing mdring[TOTAL_SYMBOL];


// Static
std::string BiIniter::mCurlBuffer;

//##################################################//
//   Constructor
//##################################################//
BiIniter::BiIniter(const std::string& url)
{
    // curl初始化
    mInitUrl = url;
    mInitCurl = curl_easy_init();
    if(mInitCurl)
    {
        //curl_easy_setopt(mInitCurl, CURLOPT_URL, "https://api3.binance.com/api/v3/ticker/price");
        curl_easy_setopt(mInitCurl, CURLOPT_WRITEFUNCTION, InitWriteCallback);
        curl_easy_setopt(mInitCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
        curl_easy_setopt(mInitCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
    else
    {
        std::cout << "BiIniter CURL Init Fail!" << std::endl;
    }
}

//##################################################//
//   Destructor
//##################################################//
BiIniter::~BiIniter()
{
    curl_easy_cleanup(mInitCurl);
}

//##################################################//
//   ~
//##################################################//
void BiIniter::InitSymbolUMap()
{
    // curl配置
    std::string url = mInitUrl + "/api/v3/ticker/price";
    curl_easy_setopt(mInitCurl, CURLOPT_URL, url.c_str());

    // 执行GET请求
    mCurlCode = curl_easy_perform(mInitCurl);
    if(mCurlCode != CURLE_OK)
    {
        fprintf(stderr, "BiIniter::curl_easy_perform() failed: %s\n", curl_easy_strerror(mCurlCode));
        return;
    }
    else
    {
        int index = 0;
        rapidjson::Document jsondoc;
        rapidjson::ParseResult jsonret = jsondoc.Parse(mCurlBuffer.c_str());
        if(jsonret)
        {
            for(const auto& item:jsondoc.GetArray())
            {
                std::string symbol = item["symbol"].GetString();
                std::string ending = "USDT";
                if(symbol.find("USDT") != std::string::npos &&\
                   symbol.compare(symbol.length()-ending.length(), ending.length(), ending) == 0)
                {
                    symbolUMap.insert(make_pair(symbol, index));
                    mdring[index].SetSymbolName(symbol);
                    //std::cout << "Init # Index: " << index << "  Symbol: " << symbol << std::endl;
                    //std::cout << "Init # GetSymbolName: " << mdring[symbolUMap[symbol]].GetSymbolName() << std::endl;
                    LOG_INFO("Init # Index: %d  Symbol: %s", index, symbol.c_str());
                    //LOG_INFO("   %s   ", " ____ ____ ____ ____ ____ ");
                    index++;
                }
            }
        }
        else
        {
            std::cout << "BiIniter::rapidjson Parse() Error !" << std::endl;
            return;
        }

        mCurlBuffer.clear();
    }     
}

void BiIniter::InitSymbolFilter()
{
    int index = 0;
    for(const auto& symbol_iter:symbolUMap)
    {
        std::cout << "Index: " << index++ << " Symbol: " << symbol_iter.first << std::endl;

        // 打开文件流
        std::string filename = "./etc/exchangeInfo/" + symbol_iter.first + ".json";
        std::ofstream outputFile(filename, std::ios::out);

        // curl配置
        std::string url = mInitUrl + "/api/v3/exchangeInfo?symbol=" + symbol_iter.first;
        curl_easy_setopt(mInitCurl, CURLOPT_URL, url.c_str());
        // 执行GET请求
        mCurlCode = curl_easy_perform(mInitCurl);
        if(mCurlCode != CURLE_OK)
        {
            fprintf(stderr, "BiIniter::curl_easy_perform() failed: %s\n", curl_easy_strerror(mCurlCode));
            return;
        }
        else
        {
            if( outputFile.is_open() )
            {
                outputFile << mCurlBuffer;
                outputFile.close();
                mCurlBuffer.clear();

                sleep(1);
            }
            else
            {
                std::cout << "BiIniter::is_open() Error !" << std::endl;
                return;
            }
        }
    }
}

void BiIniter::UpdateSymbolFilter()
{
    //int index = 0;
    for(const auto& symbol_iter:symbolUMap)
    {
        //std::cout << "Index: " << index << " Symbol: " << symbol_iter.first << std::endl;

        // 打开文件流
        std::string filename = "./etc/exchangeInfo/" + symbol_iter.first + ".json";
        std::ifstream inputFile(filename, std::ios::in);

        if( !inputFile.is_open() )
        {
            std::cout << "BiIniter::is_open() Error !" << std::endl;
            return;
        }
        else
        {
            std::string JsonBuffer;
            while( getline(inputFile, JsonBuffer) )
                continue;
            inputFile.close();

            rapidjson::Document jsondoc;
            rapidjson::ParseResult jsonret = jsondoc.Parse(JsonBuffer.c_str());
            if(jsonret)
            {
                 // 确认解析没有出错且JSON是一个对象
                if( !jsondoc.IsObject() )
                {
                    std::cerr << "JSON is not an object." << std::endl;
                    return;
                }

                // 访问symbols数组
                const rapidjson::Value& symbols = jsondoc["symbols"];
                if( !symbols.IsArray() || symbols.Empty() )
                {
                    std::cerr << "'symbols' is not an array." << std::endl;
                    return;
                }

                const rapidjson::Value& symbol = symbols[0]["symbol"];
                if( !symbol.IsString() )
                {
                    std::cerr << "'symbol' is not an string." << std::endl;
                    return;
                }
                
                //std::cout << "Index: " << index++ << " Symbol: " << symbols[0]["symbol"].GetString() << std::endl;

                // 访问第一个符号的filters数组
                const rapidjson::Value& filters = symbols[0]["filters"];
                if( !filters.IsArray() )
                {
                    std::cerr << "'filters' is not an array." << std::endl;
                    return;
                }

                // 遍历 filters 数组，查找 LOT_SIZE 类型的过滤器，然后打印 minQty
                for(const auto& filter : filters.GetArray())
                {
                    if (std::string(filter["filterType"].GetString()) == "PRICE_FILTER")
                    {
                        //std::cout << "minPrice: " << filter["minPrice"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetMinPrice(filter["minPrice"].GetString());
                        //std::cout << "maxPrice: " << filter["maxPrice"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetMaxPrice(filter["maxPrice"].GetString());
                        //std::cout << "tickSize: " << filter["tickSize"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetTickSize(filter["tickSize"].GetString());
                    }
                    else if (std::string(filter["filterType"].GetString()) == "LOT_SIZE")
                    {
                        //std::cout << "minQty: " << filter["minQty"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetMinQty(filter["minQty"].GetString());
                        //std::cout << "maxQty: " << filter["maxQty"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetMaxQty(filter["maxQty"].GetString());
                        //std::cout << "stepSize: " << filter["stepSize"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetStepSize(filter["stepSize"].GetString());
                    }
                    else if (std::string(filter["filterType"].GetString()) == "NOTIONAL")
                    {
                        //std::cout << "minNotional: " << filter["minNotional"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetMinNotional(filter["minNotional"].GetString());
                        //std::cout << "maxNotional: " << filter["maxNotional"].GetString() << std::endl;
                        mdring[symbol_iter.second].SetMaxNotional(filter["maxNotional"].GetString());
                    }
                }

                //mdring[symbol_iter.second].PrintExchangeInfo();
            }
            else
            {
                std::cout << "BiIniter::rapidjson Parse() Error !" << std::endl;
                return;
            }
        }
    }
}

//##################################################//
//   curl回调函数
//##################################################//
size_t BiIniter::InitWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
