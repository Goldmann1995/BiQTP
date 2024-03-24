/*
 * File:        PushDeer.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Notifier by PushDeer
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
#include "PushDeer.h"

// Extern
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


// Static
std::string PushDeer::mCurlBuffer;

//##################################################//
//   Constructor
//##################################################//
PushDeer::PushDeer(const std::string& url, const std::string& key)
{
    mPushUrl = url;
    mPushKey = key;

    // curl初始化
    mPushCurl = curl_easy_init();
    if(mPushCurl)
    {
        curl_easy_setopt(mPushCurl, CURLOPT_WRITEFUNCTION, PushWriteCallback);
        curl_easy_setopt(mPushCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
        curl_easy_setopt(mPushCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
    else
        sptrAsyncLogger->error("PushDeer::PushDeer() curl_easy_init() failed: {}", \
                                curl_easy_strerror(mCurlCode));
}

//##################################################//
//   Destructor
//##################################################//
PushDeer::~PushDeer()
{
    curl_easy_cleanup(mPushCurl);
}

//##################################################//
//   ~
//##################################################//
void PushDeer::Notify(const std::string& content)
{
    // curl配置
    std::string url = mPushUrl;
    std::string data = "pushkey=" + mPushKey + "&text=" + content;
    curl_easy_setopt(mPushCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(mPushCurl, CURLOPT_POSTFIELDS, data.c_str());

    // 执行GET请求
    mCurlCode = curl_easy_perform(mPushCurl);
    if(mCurlCode != CURLE_OK)
    {
        sptrAsyncLogger->error("PushDeer::Notify() curl_easy_perform() failed: {}", \
                                curl_easy_strerror(mCurlCode));
    }
    else
    {
        sptrAsyncLogger->info("PushDeer::Notify() curl_easy_perform() success: {}", \
                               mCurlBuffer);
    }

    mCurlBuffer.clear();
}

//##################################################//
//   curl回调函数
//##################################################//
size_t PushDeer::PushWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
