/*
 * File:   MDReceiver.h
 * Author: summer@SummerLab
 * Date:   2024-03-17
 * Upadte: 2024-03-17
 * Description: Receive MarketData from Binance
 */

#pragma once

#include <chrono>
#include <curl/curl.h>
#include <ThreadBase.h>


//############################################################//
//   MDReceiver Class
//############################################################//
class MDReceiver: public ThreadBase
{
public:
    MDReceiver(const std::string& url);
    ~MDReceiver();

    // 线程运行实体
    void Run();

    // 请求Binance行情服务器
    int RequestAllPrice();

    // curl回调函数
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mMdUrl;
    CURL *mMdCurl=nullptr;
    CURLcode mClCode;
    std::chrono::time_point<std::chrono::steady_clock> reqTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
    static std::string mRetBuffer;
};

//##################################################//
