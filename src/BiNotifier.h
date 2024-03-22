/*
 * File:        BiNotifier.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-22
 * LastEdit:    2024-03-22
 * Description: Notifier by PushDeer
 */

#pragma once


//############################################################//
//   BiNotifier Class
//############################################################//
class BiNotifier
{
public:
    BiNotifier(const std::string& url, const std::string& key);
    ~BiNotifier();

    // Interface
    void PushDeer(const std::string& content);

    // curl回调函数
    static size_t NotifyWriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

private:
    std::string mNotifyUrl;
    std::string mNotifyKey;
    CURL *mNotifyCurl=nullptr;
    CURLcode mCurlCode;
    static std::string mCurlBuffer;
};

//##################################################//
