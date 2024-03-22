///////////////////////////////////////////////////////////////////////////
///                                                                     ///
///                          Binance QTP                                ///
///                                                                     ///
///////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string.h>
#include <chrono>
#include <memory>
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
#include "MDSocket.h"
#include "MDReceiver.h"
#include "Calculator.h"
#include "Strategy.h"
#include "StrategyBOX.h"
#include "AccTruster.h"
#include "BiTrader.h"
#include "WatchDog.h"
#include "PushDeer.h"
// Global
#include "Macro.h"
#include "Global.h"


/********** Module-Ptr **********/
std::shared_ptr<spdlog::logger> sptrAsyncLogger = nullptr;
std::unique_ptr<INIReader> uptrINIReader = nullptr;
// uptrModules
std::unique_ptr<MDSocket> uptrMDSocket = nullptr;
//MDReceiver *ptrMDReceiver = nullptr;
Calculator *ptrCalculator = nullptr;
StrategyBOX *ptrStrategyBOX = nullptr;
std::unique_ptr<AccTruster> uptrAccTruster = nullptr;
std::unique_ptr<BiTrader> uptrBiTrader = nullptr;
WatchDog *ptrWatchDog = nullptr;
std::unique_ptr<PushDeer> uptrPushDeer = nullptr;
// 声明nullptr 延迟对象创建到main()


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
    uptrINIReader = std::make_unique<INIReader>(argv[1]);
    if( uptrINIReader->ParseError()<0 )
    {
        fprintf(stderr, "Error: %s \n", "INIReader ParseError() failed !");
        return -3;
    }

    /********** 初始化spdlog **********/
    // 默认队列大小8192 后台线程1
    spdlog::init_thread_pool(1024*16, 1);
    std::string log_path = uptrINIReader->Get("log", "LogPath", "UNKNOWN");
    std::string out_path = uptrINIReader->Get("log", "OutPath", "UNKNOWN");
    log_path += Utils::Timer::GetDate("%Y_%m_%d") + "_log";
    sptrAsyncLogger = spdlog::basic_logger_mt<spdlog::async_factory>("sptrAsyncLogger", log_path);
    sptrAsyncLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    // 默认是info级别
    sptrAsyncLogger->set_level(spdlog::level::trace);
    // 默认是off关闭自动刷新
    sptrAsyncLogger->flush_on(spdlog::level::trace);


    /********** 打印QTP版本信息 **********/
    sptrAsyncLogger->info("--------------------------------------");
    sptrAsyncLogger->info("       ____ ____ ____ ____ ____    ");
    sptrAsyncLogger->info("      ||B |||i |||Q |||T |||P ||   ");
    sptrAsyncLogger->info("      ||__|||__|||__|||__|||__||   ");
    sptrAsyncLogger->info("      |/__\\|/__\\|/__\\|/__\\|/__\\|   ");
    sptrAsyncLogger->info("                                   ");
    sptrAsyncLogger->info("           Version: \033[0;34m{}\033[0m    ", BI_QTP_VERSION);
    sptrAsyncLogger->info("           Date: \033[0;34m{}\033[0m    ",    BI_QTP_VERDATE);
    sptrAsyncLogger->info("--------------------------------------");

    /********** 打印Config信息 **********/
    sptrAsyncLogger->info("Config # LogPath: {}", log_path);
    sptrAsyncLogger->info("Config # OutPath: {}", out_path);
    std::string bi_wss_url = uptrINIReader->Get("binance", "WssUrl", "UNKNOWN");
    std::string bi_api_url = uptrINIReader->Get("binance", "ApiUrl", "UNKNOWN");
    std::string bi_api_key = uptrINIReader->Get("binance", "Apikey", "UNKNOWN");
    std::string bi_secret_key = uptrINIReader->Get("binance", "SecretKey", "UNKNOWN");
    sptrAsyncLogger->info("Config # WssUrl: {}", bi_wss_url);
    sptrAsyncLogger->info("Config # ApiUrl: {}", bi_api_url);
    sptrAsyncLogger->info("Config # Apikey: {}", bi_api_key);
    std::string push_url = uptrINIReader->Get("notifier", "PushUrl", "UNKNOWN");
    std::string push_key = uptrINIReader->Get("notifier", "PushKey", "UNKNOWN");
    sptrAsyncLogger->info("Config # PushUrl: {}", push_url);
    sptrAsyncLogger->info("Config # PushKey: {}", push_key);


    /********** BiIniter **********/
    std::string url = uptrINIReader->Get("binance", "ApiUrl", "UNKNOWN");
    BiIniter initer(url);
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();


    /********** MDSocket **********/
    uptrMDSocket = std::make_unique<MDSocket>(bi_wss_url);
    uptrMDSocket->Start();
    uptrMDSocket->SetSelfTName((char *)"MDSocket");


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
    //AdvancedSLR1 *aslr1 = new AdvancedSLR1(ASLR1, 1000.0);
    //AdvancedSLR2 *aslr2 = new AdvancedSLR2(ASLR2, 1000.0);
    //MACross1 *macs1 = new MACross1(MACROSS1, 1000.0);
    //MACross2 *macs2 = new MACross2(MACROSS2, 1000.0);
    GridTrader *grid = new GridTrader(GRID);
    //ptrStrategyBOX->EntrustStrategy(aslr1);
    //ptrStrategyBOX->EntrustStrategy(aslr2);
    //ptrStrategyBOX->EntrustStrategy(macs1);
    //ptrStrategyBOX->EntrustStrategy(macs2);
    ptrStrategyBOX->EntrustStrategy(grid);
    ptrStrategyBOX->Start();
    ptrStrategyBOX->SetSelfTName((char *)"StrategyBOX");


    /********** AccTruster **********/
    uptrAccTruster = std::make_unique<AccTruster>(bi_api_url, bi_api_key, bi_secret_key);
    uptrAccTruster->Start();
    uptrAccTruster->SetSelfTName((char *)"AccTruster");


    /********** BiTrader **********/
    uptrBiTrader = std::make_unique<BiTrader>(bi_api_url, bi_api_key, bi_secret_key);
    //uptrBiTrader->InsertOrder();
    uptrBiTrader->Start();
    uptrBiTrader->SetSelfTName((char *)"BiTrader");


    /********** WatchDog **********/
    ptrWatchDog = new WatchDog();
    ptrWatchDog->Start();
    ptrWatchDog->SetSelfTName((char *)"WatchDog");


    /********** PushDeer **********/
    uptrPushDeer = std::make_unique<PushDeer>(push_url, push_key);
    //uptrBiNotifier->PushDeer("BiQTP Start");


    /********** Hold for Join **********/
    uptrMDSocket->Join();
    //ptrMDReceiver->Join();
    ptrCalculator->Join();
    ptrStrategyBOX->Join();
    uptrAccTruster->Join();
    uptrBiTrader->Join();
    ptrWatchDog->Join();


    /********** 资源清理 **********/
    // 全局清理libcurl环境
    curl_global_cleanup();
    // 关闭并等待所有日志记录完毕
    spdlog::shutdown();

    return 0;
}

/********** END **********/
