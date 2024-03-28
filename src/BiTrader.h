/*
 * File:        BiTrader.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Binance Trader
 */

#pragma once


//############################################################//
//   BiTrader Class
//############################################################//
class BiTrader
{
public:
    BiTrader(const std::string& url, const std::string& api_key, const std::string& secret_key);
    ~BiTrader();

    // 币安报单
    bool InsertOrder(std::string symbol, \
                     Binance::OrderSide side, \
                     double price, \
                     double qty, \
                     Binance::OrderType type, \
                     Binance::TimeInForce tif, \
                     double& exe_price, \
                     double& exe_qty, \
                     double& commission_qty);
    // 解析报单回报
    void ParseInsertResp(std::string rsp, double& exe_price, double& exe_qty, double& commission_qty);

    // curl回调函数
    static size_t TDWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:    
    // Key
    std::string mTdUrl;
    std::string mTdApiKey;
    std::string mTdSecretKey;
    // TdCurl
    CURL *mTdCurl = nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
