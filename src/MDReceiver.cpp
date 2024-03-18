/*
 * File:        MDReceiver.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-17
 * Description: Receive MarketData from Binance
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctime>

//#include "log.h"
#include "MDReceiver.h"

std::string MDReceiver::mRetBuffer;


//##################################################//
//   Init
//##################################################//
MDReceiver::MDReceiver(const std::string& url)
{
    mMdUrl = url;
    reqTime = std::chrono::steady_clock::now();
    nowTime = std::chrono::steady_clock::now();
    // Curl初始化
    mMdCurl = curl_easy_init();
    if(mMdCurl)
    {
        curl_easy_setopt(mMdCurl, CURLOPT_URL, "https://api3.binance.com/api/v3/ticker/price");
        curl_easy_setopt(mMdCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(mMdCurl, CURLOPT_WRITEDATA, &mRetBuffer);
        curl_easy_setopt(mMdCurl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    }
    else
    {
        std::cout << "MD CURL Init Fail!" << std::endl;
    }
}

//##################################################//
//   ~
//##################################################//
MDReceiver::~MDReceiver()
{
    curl_easy_cleanup(mMdCurl);
}

//##################################################//
//   线程运行实体
//##################################################//
void MDReceiver::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec = 1;
    time_to_sleep.tv_nsec = 1000;

	while( true )
	{
        nowTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(nowTime - reqTime);
        if( elapsed > std::chrono::seconds(10) )
        {
            reqTime = std::chrono::steady_clock::now();
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result == 0 )
        {
            //std::cout << "Slept for 1 second." << std::endl;
        }
        else
        {
            //std::cerr << "nanosleep failed." << std::endl;
        }
	}
}

//##################################################//
//   ~
//##################################################//
int MDReceiver::RequestAllPrice()
{
    // 执行GET请求
    mClCode = curl_easy_perform(mMdCurl);
    // 检查错误
    if( mClCode != CURLE_OK )
    {
        fprintf(stderr, "MDReceiver::curl_easy_perform() failed: %s\n", curl_easy_strerror(mClCode));
        return -1;
    }
    else
    {
        // json解析
    }

    return 0;
}

//##################################################//
//   curl回调函数
//##################################################//
size_t MDReceiver::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t length = size*nmemb;
    userp->append((char*)contents, length);
    return length;
}
