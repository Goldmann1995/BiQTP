/*
 * File:        BiTrader.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Order Manager
 */

#pragma once

#include <ThreadBase.h>
#include <BiDef.h>


//############################################################//
//   BiTrader Class
//############################################################//
class BiTrader: public ThreadBase
{
public:
    BiTrader(const std::string& url, const std::string& api_key, const std::string& secret_key);
    ~BiTrader();

    // 线程运行实体
    void Run();

    // Interface
    void InsertOrder(std::string symbol, \
                     Binance::OrderSide side, \
                     double price, \
                     double qty, \
                     Binance::OrderType type, \
                     Binance::TimeInForce tif);
    // ~
    static std::string double2string(double value, int precision);
    // curl回调函数
    static size_t TDWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mTdUrl;
    std::string mTdApiKey;
    std::string mTdSecretKey;
    // TdCurl
    CURL *mTdCurl = nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
