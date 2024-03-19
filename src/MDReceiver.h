/*
 * File:        MDReceiver.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-17
 * Description: Receive MarketData from Binance
 */

#pragma once

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
    static size_t MDWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mMdUrl;
    CURL *mMdCurl=nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;

    std::chrono::time_point<std::chrono::steady_clock> reqTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
};

//############################################################//
