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
// 3rd-lib
#include <curl/curl.h>
//#include <openssl/hmac.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
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
// 延迟对象的创建到main()中
MDReceiver *ptrReceiver = nullptr;
Calculator *ptrCalculator = nullptr;
StrategyBOX *ptrStrategyBOX = nullptr;
std::shared_ptr<spdlog::logger> sptrAsyncLogger = nullptr;


/********** Main Entry **********/
int main()
{
    // tips summer@20240318 - 是否应该移到main()之前
    // 全局初始化libcurl环境
    curl_global_init(CURL_GLOBAL_ALL);

    // 初始化Log
    Log::get_instance()->init("/home/Binance/BiQTP/log/Log", 1024, 5000000);

    /********** 初始化spdlog **********/
    // 默认队列大小8192 后台线程1
    spdlog::init_thread_pool(1024*16, 1);
    sptrAsyncLogger = spdlog::basic_logger_mt<spdlog::async_factory>("sptrAsyncLogger", "/home/Binance/BiQTP/log/2024_03_20_log");
    sptrAsyncLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    // 默认是info级别
    sptrAsyncLogger->set_level(spdlog::level::trace);
    // 默认是off 即关闭自动刷新
    sptrAsyncLogger->flush_on(spdlog::level::trace);

    // 打印QTP版本信息
    sptrAsyncLogger->info("--------------------------------------");
    sptrAsyncLogger->info("       ____ ____ ____ ____ ____    ");
    sptrAsyncLogger->info("      ||B |||i |||Q |||T |||P ||   ");
    sptrAsyncLogger->info("      ||__|||__|||__|||__|||__||   ");
    sptrAsyncLogger->info("      |/__\\|/__\\|/__\\|/__\\|/__\\|   ");
    sptrAsyncLogger->info("                                   ");
    sptrAsyncLogger->info("           Version: \033[0;34m{}\033[0m    ", BI_QTP_VERSION);
    sptrAsyncLogger->info("           Date: \033[0;34m{}\033[0m    ",    BI_QTP_VERDATE);
    sptrAsyncLogger->info("--------------------------------------");

    // Binance-QTP初始化
    BiIniter initer("https://api.binance.com");
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();

    // MDReceiver
    ptrReceiver = new MDReceiver("https://api.binance.com");
    ptrReceiver->Start();

    // Calculator
    ptrCalculator = new Calculator();
    ptrCalculator->Start();

    // Strategy
    ptrStrategyBOX = new StrategyBOX();
    //AdvancedSLR1 *aslr1 = new AdvancedSLR1(ASLR1, 1000.0);
    //AdvancedSLR2 *aslr2 = new AdvancedSLR2(ASLR2, 1000.0);
    MACross1 *macs1 = new MACross1(MACROSS1, 1000.0);
    MACross2 *macs2 = new MACross2(MACROSS2, 1000.0);
    //ptrStrategyBOX->EntrustStrategy(aslr1);
    ptrStrategyBOX->EntrustStrategy(macs1);
    ptrStrategyBOX->EntrustStrategy(macs2);
    ptrStrategyBOX->Start();

    /********** Hold for Join **********/
    ptrReceiver->Join();
    ptrCalculator->Join();
    ptrStrategyBOX->Join();

    /********** 资源清理 **********/
    // 全局清理libcurl环境
    curl_global_cleanup();
    // 关闭并等待所有日志记录完毕
    spdlog::shutdown();

    return 0;
}

/********** END **********/
