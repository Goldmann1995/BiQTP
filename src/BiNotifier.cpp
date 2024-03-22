/*
 * File:        BiNotifier.cpp
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
#include <spdlog/sinks/basic_file_sink.h>
// QTP
#include "BiNotifier.h"

// Extern
extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;


// Static
std::string BiNotifier::mCurlBuffer;

//##################################################//
//   Constructor
//##################################################//
BiNotifier::BiNotifier(const std::string& url, const std::string& key)
{
    mNotifyUrl = url;
    mNotifyKey = key;

    // curl初始化
    mNotifyCurl = curl_easy_init();
    if(mNotifyCurl)
    {
        curl_easy_setopt(mNotifyCurl, CURLOPT_WRITEFUNCTION, NotifyWriteCallback);
        curl_easy_setopt(mNotifyCurl, CURLOPT_WRITEDATA, &mCurlBuffer);
        curl_easy_setopt(mNotifyCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
    else
        sptrAsyncLogger->error("BiNotifier::BiNotifier() curl_easy_init() failed !");
}

//##################################################//
//   Destructor
//##################################################//
BiNotifier::~BiNotifier()
{
    curl_easy_cleanup(mNotifyCurl);
}

//##################################################//
//   ~
//##################################################//
void BiNotifier::PushDeer(const std::string& content)
{
    // curl配置
    std::string url = mNotifyUrl;
    std::string data = "pushkey=" + mNotifyKey + "&text=" + content;
    curl_easy_setopt(mNotifyCurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(mNotifyCurl, CURLOPT_POSTFIELDS, data.c_str());

    // 执行GET请求
    mCurlCode = curl_easy_perform(mNotifyCurl);
    if(mCurlCode != CURLE_OK)
    {
        sptrAsyncLogger->error("BiNotifier::PushDeer() curl_easy_perform() failed: {}", \
                                curl_easy_strerror(mCurlCode));
    }
    else
    {
        sptrAsyncLogger->info("BiNotifier::PushDeer() curl_easy_perform() success: {}", \
                                mCurlBuffer);
    }

    mCurlBuffer.clear();
}

//##################################################//
//   curl回调函数
//##################################################//
size_t BiNotifier::NotifyWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
