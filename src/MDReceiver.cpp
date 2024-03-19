/*
 * File:        MDReceiver.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-17
 * Description: Receive MarketData from Binance
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctime>
#include <chrono>
#include <unordered_map>

#include <curl/curl.h>
#include <rapidjson/document.h>
#include <log/log.h>

#include "Macro.h"
#include "MDRing.h"
#include "MDReceiver.h"

// Extern
extern std::unordered_map<std::string, int> symbolUMap;
extern MDRing mdring[TOTAL_SYMBOL];

// Static
std::string MDReceiver::mCurlBuffer;


//##################################################//
//   Constructor
//##################################################//
MDReceiver::MDReceiver(const std::string& url)
{
    mMdUrl = url;
    reqTime = std::chrono::steady_clock::now();
    nowTime = std::chrono::steady_clock::now();

    // Curl初始化
    mMdCurl = curl_easy_init();
    if(mMdCurl)
    {
        //curl_easy_setopt(mMdCurl, CURLOPT_URL, "https://api3.binance.com/api/v3/ticker/price");
        curl_easy_setopt(mMdCurl, CURLOPT_WRITEFUNCTION, MDWriteCallback);
        curl_easy_setopt(mMdCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
        curl_easy_setopt(mMdCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
    else
        LOG_ERROR("MDReceiver::MDReceiver() %s ", "curl_easy_init() failed !");
}

//##################################################//
//   Destructor
//##################################################//
MDReceiver::~MDReceiver()
{
    curl_easy_cleanup(mMdCurl);
}

//##################################################//
//   线程运行实体
//##################################################//
void MDReceiver::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 0;
    time_to_sleep.tv_nsec = 1000*100;   // 100us

	while( true )
	{
        nowTime = std::chrono::steady_clock::now();
        std::chrono::seconds elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - reqTime);
        if( elapsed_time >= std::chrono::seconds(REQ_TIME_INTERVAL) )
        {
            reqTime = std::chrono::steady_clock::now();
            RequestAllPrice();
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            LOG_ERROR("MDReceiver::Run() %s ", "nanosleep() failed !");
	}
}

//##################################################//
//   请求所有币种最新价格
//##################################################//
int MDReceiver::RequestAllPrice()
{
    // curl配置
    std::string url = mMdUrl + "/api/v3/ticker/price";
    curl_easy_setopt(mMdCurl, CURLOPT_URL, url.c_str());

    // 执行GET请求
    mCurlCode = curl_easy_perform(mMdCurl);
    if(mCurlCode != CURLE_OK)
    {
        LOG_ERROR("MDReceiver::RequestAllPrice() %s ", "curl_easy_perform() failed !");
        return -1;
    }
    else
    {
        rapidjson::Document jsondoc;
        rapidjson::ParseResult jsonret = jsondoc.Parse(mCurlBuffer.c_str());
        if(jsonret)
        {
            if( !jsondoc.IsArray() || jsondoc.Empty() )
            {
                LOG_ERROR("MDReceiver::RequestAllPrice() %s ", "jsondoc is not an array !");
                return -2;
            }

            for(const auto& item : jsondoc.GetArray())
            {
                std::string str_symbol = item["symbol"].GetString();
                std::string str_price = item["price"].GetString();
                double db_price = stod(str_price);
                if(symbolUMap.find(str_symbol) != symbolUMap.end())
                {
                    int symbol_idx = symbolUMap[str_symbol];
                    mdring[symbol_idx].PushMD(db_price);
                }
            }
        }
        mCurlBuffer.clear();
    }

    return 0;
}

//##################################################//
//   curl回调函数
//##################################################//
size_t MDReceiver::MDWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
