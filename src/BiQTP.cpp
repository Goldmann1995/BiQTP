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
#include <inih/INIReader.h>
#include <curl/curl.h>
//#include <openssl/hmac.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>
// internal
#include <Utils.h>
// RingMD
#include "MDRing.h"
// Module
#include "BiIniter.h"
#include "MDReceiver.h"
#include "MDSocket.h"
#include "Calculator.h"
#include "Strategy.h"
#include "StrategyBOX.h"
#include "WatchDog.h"
// Global
#include "Macro.h"
#include "Global.h"


/********** Module-Ptr **********/
// 延迟对象创建到main()
std::shared_ptr<spdlog::logger> sptrAsyncLogger = nullptr;
INIReader *ptrINIReader = nullptr;
MDReceiver *ptrMDReceiver = nullptr;
MDSocket *ptrMDSocket = nullptr;
Calculator *ptrCalculator = nullptr;
StrategyBOX *ptrStrategyBOX = nullptr;
WatchDog *ptrWatchDog = nullptr;


/********** Main Entry **********/
int main(int argc, char *argv[])
{
    /********** arg参数校验 **********/
    if( argc<2 )
    {
		fprintf(stderr, "Error Usage: %s ConfigFile \n", argv[0]);
		return -1;
	}
    if( !Utils::IsFileExists(argv[1]) )
	{
        fprintf(stderr, "ConfigFile Not Found: %s \n", argv[1]);
		return -2;
	}

    /********** 全局初始化 **********/
    // 全局初始化libcurl环境
    curl_global_init(CURL_GLOBAL_ALL);

    /********** 初始化inih **********/
    ptrINIReader = new INIReader(argv[1]);
    if( ptrINIReader->ParseError()<0 )
    {
        fprintf(stderr, "Error: %s \n", "INI ParseError() !");
        return -3;
    }

    /********** 初始化spdlog **********/
    // 默认队列大小8192 后台线程1
    spdlog::init_thread_pool(1024*16, 1);
    std::string log_path = ptrINIReader->Get("log", "LogPath", "UNKNOWN");
    log_path += Utils::Timer::GetDate("%Y_%m_%d") + "_log";
    sptrAsyncLogger = spdlog::basic_logger_mt<spdlog::async_factory>("sptrAsyncLogger", log_path);
    sptrAsyncLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    // 默认是info级别
    sptrAsyncLogger->set_level(spdlog::level::trace);
    // 默认是off 即关闭自动刷新
    sptrAsyncLogger->flush_on(spdlog::level::trace);

    /********** QTP版本信息 **********/
    sptrAsyncLogger->info("--------------------------------------");
    sptrAsyncLogger->info("       ____ ____ ____ ____ ____    ");
    sptrAsyncLogger->info("      ||B |||i |||Q |||T |||P ||   ");
    sptrAsyncLogger->info("      ||__|||__|||__|||__|||__||   ");
    sptrAsyncLogger->info("      |/__\\|/__\\|/__\\|/__\\|/__\\|   ");
    sptrAsyncLogger->info("                                   ");
    sptrAsyncLogger->info("           Version: \033[0;34m{}\033[0m    ", BI_QTP_VERSION);
    sptrAsyncLogger->info("           Date: \033[0;34m{}\033[0m    ",    BI_QTP_VERDATE);
    sptrAsyncLogger->info("--------------------------------------");
    sptrAsyncLogger->info("LogPath: {}", ptrINIReader->Get("log", "LogPath", "UNKNOWN"));
    sptrAsyncLogger->info("ApiUrl: {}", ptrINIReader->Get("binance", "ApiUrl", "UNKNOWN"));

    /********** BiIniter **********/
    std::string url = ptrINIReader->Get("binance", "ApiUrl", "UNKNOWN");
    BiIniter initer(url);
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();

    /********** MDSocket **********/
    ptrMDSocket = new MDSocket("wss://stream.binance.com:443/stream?streams=!miniTicker@arr");
    ptrMDSocket->Start();
    ptrMDSocket->SetSelfTName((char *)"MDSocket");

    /********** MDReceiver **********/
    //ptrMDReceiver = new MDReceiver(url);
    //ptrMDReceiver->Start();
    //ptrMDReceiver->SetSelfTName((char *)"MDReceiver");

    /********** Calculator **********/
    ptrCalculator = new Calculator();
    ptrCalculator->Start();
    ptrCalculator->SetSelfTName((char *)"Calculator");

    /********** StrategyBOX **********/
    ptrStrategyBOX = new StrategyBOX();
    AdvancedSLR1 *aslr1 = new AdvancedSLR1(ASLR1, 1000.0);
    AdvancedSLR2 *aslr2 = new AdvancedSLR2(ASLR2, 1000.0);
    MACross1 *macs1 = new MACross1(MACROSS1, 1000.0);
    MACross2 *macs2 = new MACross2(MACROSS2, 1000.0);
    ptrStrategyBOX->EntrustStrategy(aslr1);
    ptrStrategyBOX->EntrustStrategy(aslr2);
    ptrStrategyBOX->EntrustStrategy(macs1);
    ptrStrategyBOX->EntrustStrategy(macs2);
    ptrStrategyBOX->Start();
    ptrStrategyBOX->SetSelfTName((char *)"StrategyBOX");

    /********** WatchDog **********/
    ptrWatchDog = new WatchDog();
    ptrWatchDog->Start();
    ptrWatchDog->SetSelfTName((char *)"WatchDog");

    /********** Hold for Join **********/
    ptrMDSocket->Join();
    //ptrMDReceiver->Join();
    ptrCalculator->Join();
    ptrStrategyBOX->Join();
    ptrWatchDog->Join();

    /********** 资源清理 **********/
    // 全局清理libcurl环境
    curl_global_cleanup();
    // 关闭并等待所有日志记录完毕
    spdlog::shutdown();

    return 0;
}

/********** END **********/
