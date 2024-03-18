/*
 * File:        BiIniter.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: Initialization from Binance
 */

#pragma once


//############################################################//
//   BiIniter Class
//############################################################//
class BiIniter
{
public:
    BiIniter(const std::string& url);
    ~BiIniter();

    // Init Logic
    void InitSymbolUMap();
    void InitSymbolFilter();
    void UpdateSymbolFilter();

    // curl回调函数
    static size_t InitWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mInitUrl;
    CURL *mInitCurl=nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
