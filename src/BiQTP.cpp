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

// STL-DS
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

// lib
#include <curl/curl.h>
//#include <openssl/hmac.h>
#include <rapidjson/document.h>
#include <log/log.h>

// RingMD
#include "MDRing.h"

// Module
#include "BiIniter.h"
#include "MDReceiver.h"
#include "Calculator.h"
#include "Strategy.h"
#include "StrategyBOX.h"

// Global
#include "Macro.h"
#include "Global.h"

/********** ~ **********/
MDReceiver *ptrReceiver = nullptr;
Calculator *ptrCalculator = nullptr;
StrategyBOX *ptrStrategyBOX = nullptr;


/********** Main Entry **********/
int main()
{
    // tips summer@20240318 - 是否应该移到main()之前
    // 全局初始化libcurl环境
    curl_global_init(CURL_GLOBAL_ALL);

    // 初始化Log
    Log::get_instance()->init("/home/Binance/BiQTP/log/Log", 1024, 5000000);

    // 打印QTP版本信息
    LOG_INFO("--------------%s--------------", "----------");
    LOG_INFO("      %s   ", " ____ ____ ____ ____ ____ "     );
    LOG_INFO("      %s   ", "||B |||i |||Q |||T |||P ||"     );
    LOG_INFO("      %s   ", "||__|||__|||__|||__|||__||"     );
    LOG_INFO("      %s   ", "|/__\\|/__\\|/__\\|/__\\|/__\\|");
    LOG_INFO("      %s   ",  "  ");
    LOG_INFO("           Version: \033[0;34m%s\033[0m    ", BI_QTP_VERSION);
    LOG_INFO("           Date: \033[0;34m%s\033[0m    ",    BI_QTP_VERDATE);
    LOG_INFO("--------------%s--------------", "----------");

    // Binance-QTP初始化
    BiIniter initer("https://api3.binance.com");
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();

    // MDReceiver
    ptrReceiver = new MDReceiver("https://api3.binance.com");
    ptrReceiver->Start();

    // Calculator
    ptrCalculator = new Calculator();
    ptrCalculator->Start();

    // Strategy
    ptrStrategyBOX = new StrategyBOX();
    AdvancedSLR1 *aslr1 = new AdvancedSLR1(ASLR1, 1000.0);
    AdvancedSLR2 *aslr2 = new AdvancedSLR2(ASLR2, 1000.0);
    ptrStrategyBOX->EntrustStrategy(aslr1);
    ptrStrategyBOX->EntrustStrategy(aslr2);
    ptrStrategyBOX->Start();

    // Hold for Join
    ptrReceiver->Join();
    ptrCalculator->Join();
    ptrStrategyBOX->Join();

    // 全局清理libcurl环境
    curl_global_cleanup();

    return 0;
}

/********** END **********/
