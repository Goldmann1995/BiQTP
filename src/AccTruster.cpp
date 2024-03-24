/*
 * File:        AccTruster.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Binance Account Truster
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
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// QTP
#include <Utils.h>
#include "AccTruster.h"


// Extern
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


// Static
std::string AccTruster::mCurlBuffer;

//##################################################//
//   Constructor
//##################################################//
AccTruster::AccTruster(const std::string& url, const std::string& api_key, const std::string& secret_key)
{
    mTrustUrl = url;
    mTrustApiKey = api_key;
    mTrustSecretKey = secret_key;

    // curl初始化
    mTrustCurl = curl_easy_init();
    if(mTrustCurl)
    {
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, ("X-MBX-APIKEY: " + mTrustApiKey).c_str());
        curl_easy_setopt(mTrustCurl, CURLOPT_HTTPHEADER, chunk);
        //curl_easy_setopt(mTrustUrl, CURLOPT_URL, (mTdUrl+"/api/v3/order").c_str());
        curl_easy_setopt(mTrustCurl, CURLOPT_WRITEFUNCTION, TrustWriteCallback);
        curl_easy_setopt(mTrustCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
    }
    else
        sptrAsyncLogger->error("BiTrader::BiTrader() curl_easy_init() failed !");
}

//##################################################//
//   Destructor
//##################################################//
AccTruster::~AccTruster()
{
    curl_easy_cleanup(mTrustCurl);
}

//##################################################//
//   线程运行实体
//##################################################//
void AccTruster::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 3;   // 3s
    time_to_sleep.tv_nsec = 0;

	while( true )
	{
        //QueryAccount();

        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("BiTruster::Run() nanosleep() failed !");
	}
}

//##################################################//
//   ~
//##################################################//
void AccTruster::QueryAccount()
{
    // 设置Order参数
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::string req = "timestamp="+std::to_string(millis);
    std::string signature = Utils::SSL::GetHMAC_SHA256(mTrustSecretKey, req);
    std::string data = req + "&signature=" + signature;
    // curl配置
    curl_easy_setopt(mTrustCurl, CURLOPT_URL, (mTrustUrl+"/api/v3/account?"+data).c_str());

    // 执行GET请求
    mCurlCode = curl_easy_perform(mTrustCurl);
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
size_t AccTruster::TrustWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
