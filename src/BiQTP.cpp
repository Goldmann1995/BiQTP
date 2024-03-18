///////////////////////////////////////////////////////////////////////////
///                                                                     ///
///                          Binance QTP                                ///
///                                                                     ///
///////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
//#include <string>
#include <string.h>
#include <chrono>

// STL
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

// lib
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <rapidjson/document.h>
#include <log/log.h>

// include
//#include <ThreadBase.h>

#include "MDRing.h"
#include "MDReceiver.h"
#include "Calculator.h"
#include "BiIniter.h"
#include "Global.h"

using namespace std;


/********** Main Entry **********/
int main()
{
    // tips summer@20240318 - 是否应该移到main()之前
    // 初始化libcurl库全局环境
    curl_global_init(CURL_GLOBAL_ALL);

    // 初始化Log
    Log::get_instance()->init("/home/Binance/BiQTP/log/Log", 1024, 5000000);
    // 打印系统版本信息
    LOG_INFO("--------------%s--------------", "----------");
    //LOG_INFO("   %s   ",  "  ");
    LOG_INFO("   %s   ", " ____ ____ ____ ____ ____ ");
    LOG_INFO("   %s   ", "||B |||i |||Q |||T |||P ||");
    LOG_INFO("   %s   ", "||__|||__|||__|||__|||__||");
    LOG_INFO("   %s   ", "|/__\\|/__\\|/__\\|/__\\|/__\\|");
    LOG_INFO("   %s   ",  "  ");
    LOG_INFO("        Version: \033[0;34m%s\033[0m    ", "Alpha");
    LOG_INFO("--------------%s--------------", "----------");

    // Binance-QTP初始化
    BiIniter initer("https://api3.binance.com");
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();

    // MD
    MDReceiver receiver("https://api3.binance.com");
    receiver.Start();

    // Cal
    Calculator calculator;
    calculator.Start();

    // join
    receiver.Join();
    calculator.Join();

    // ~
    curl_global_cleanup();

    return 0;
}

/********** END **********/
