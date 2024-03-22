/*
 * File:        AccTruster.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Binance Account Truster
 */

#pragma once

#include <ThreadBase.h>


//############################################################//
//   AccTruster Class
//############################################################//
class AccTruster: public ThreadBase
{
public:
    AccTruster(const std::string& url, const std::string& api_key, const std::string& secret_key);
    ~AccTruster();

    // 线程运行实体
    void Run();

    // Interface
    void QueryAccount();

    // curl回调函数
    static size_t TrustWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mTrustUrl;
    std::string mTrustApiKey;
    std::string mTrustSecretKey;
    
    CURL *mTrustCurl = nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
