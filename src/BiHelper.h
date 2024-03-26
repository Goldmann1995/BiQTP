/*
 * File:        BiHelper.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-25
 * Description: Get ExchInfo from Binance
 */

#pragma once


//############################################################//
//   BiHelper Class
//############################################################//
class BiHelper
{
public:
    BiHelper(const std::string& url, const std::string& path);
    ~BiHelper();

    // Interface
    void InitSymbolIdxMap();
    void GenerateSymbolList();
    void RequestSymbolFilter();
    void InitSymbolFilter();

    // curl回调函数
    static size_t HelpWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mInfoPath;
    std::string mHelpUrl;
    CURL *mHelpCurl = nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
