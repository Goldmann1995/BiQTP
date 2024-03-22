/*
 * File:        PushDeer.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-23
 * LastEdit:    2024-03-23
 * Description: Notifier by PushDeer
 */

#pragma once


//############################################################//
//   PushDeer Class
//############################################################//
class PushDeer
{
public:
    PushDeer(const std::string& url, const std::string& key);
    ~PushDeer();

    // Interface
    void Notify(const std::string& content);

    // curl回调函数
    static size_t PushWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mPushUrl;
    std::string mPushKey;
    CURL *mPushCurl=nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
