/*
 * File:        BiTrader.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Order Manager
 */

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
// 3rd-lib
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
// QTP
#include <Utils.h>
#include "BiTrader.h"


// Extern
extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;


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
        sptrAsyncLogger->error("BiTrader::BiTrader() curl_easy_init() failed !");
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
//   ~
//##################################################//
void BiTrader::InsertOrder()
{
    // 设置Order参数
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::string req = "symbol=BOMEUSDT&side=SELL&type=LIMIT&timeInForce=GTC&quantity=1000.0&price=0.02&timestamp="+std::to_string(millis);
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
//   curl回调函数
//##################################################//
size_t BiTrader::TDWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
