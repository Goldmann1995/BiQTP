///////////////////////////////////////////////////////////////////////////
///                                                                     ///
///                          Binance QTP                                ///
///                           by summer                                 ///
///                                                                     ///
///////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <iostream>
#include <string>
#include <memory>
// STL
#include <unordered_map>
#include <vector>
// 3rd-lib
#include <inih/INIReader.h>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>
// QTP
#include "BiDef.h"
#include "BiHelper.h"
#include "BiTrader.h"
#include "MDRing.h"
#include "MDSocket.h"
#include "MDReceiver.h"
#include "MDReplayer.h"
#include "Calculator.h"
#include "Strategy.h"
#include "StrategyBOX.h"
#include "OrderManager.h"
#include "AccTruster.h"
#include "WatchDog.h"
#include "PushDeer.h"
// Global
#include "Macro.h"
#include "Global.h"
#include "Utils/Utils.hpp"


/********** Module-Ptr **********/
// 声明nullptr 延迟对象创建到main()
std::unique_ptr<INIReader> uptrINIReader = nullptr;
std::shared_ptr<spdlog::sinks::daily_file_sink_mt> sptrDailyLogSink = nullptr;
std::shared_ptr<spdlog::async_logger> sptrAsyncLogger = nullptr;
std::shared_ptr<spdlog::sinks::daily_file_sink_mt> sptrDailyOutSink = nullptr;
std::shared_ptr<spdlog::async_logger> sptrAsyncOuter = nullptr;
// uptrModules
std::unique_ptr<BiHelper> uptrBiHelper = nullptr;
std::unique_ptr<BiTrader> uptrBiTrader = nullptr;
std::unique_ptr<MDSocket> uptrMDSocket = nullptr;
//std::unique_ptr<MDReceiver> uptrMDReceiver = nullptr;
std::unique_ptr<MDReplayer> uptrMDReplayer = nullptr;
std::unique_ptr<Calculator> uptrCalculator = nullptr;
std::unique_ptr<StrategyBOX> uptrStrategyBOX = nullptr;
std::unique_ptr<OrderManager> uptrOrderManager = nullptr;
std::unique_ptr<AccTruster> uptrAccTruster = nullptr;
std::unique_ptr<WatchDog> uptrWatchDog = nullptr;
std::unique_ptr<PushDeer> uptrPushDeer = nullptr;


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
    //@spdlog// 后台线程数量超过1会引起乱序问题
    spdlog::init_thread_pool(1024*128, 1);
    std::string log_path = uptrINIReader->Get("log", "LogPath", "UNKNOWN");
    std::string out_path = uptrINIReader->Get("log", "OutPath", "UNKNOWN");
    // 创建每日滚动的文件sink
    sptrDailyLogSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_path, 0, 0);
    sptrDailyLogSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    sptrDailyOutSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(out_path, 0, 0);
    sptrDailyOutSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    // 创建异步Logger
    //@spdlog// block - 当消息队列满时 日志操作会阻塞直到队列中有足够的空间
    //@spdlog// overrun_oldest - 当消息队列满时 最旧的消息会被新的消息覆盖以避免阻塞
    sptrAsyncLogger = std::make_shared<spdlog::async_logger>(
                      "sptrAsyncLogger", sptrDailyLogSink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(sptrAsyncLogger);
    // 创建异步Outer
    sptrAsyncOuter = std::make_shared<spdlog::async_logger>(
                      "sptrAsyncOuter", sptrDailyOutSink, spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(sptrAsyncOuter);
    // 默认是info级别
    sptrAsyncLogger->set_level(spdlog::level::trace);
    sptrAsyncOuter->set_level(spdlog::level::trace);
    // 默认是off关闭自动刷新
    sptrAsyncLogger->flush_on(spdlog::level::trace);
    sptrAsyncOuter->flush_on(spdlog::level::trace);


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
    std::string exchange_path = uptrINIReader->Get("log", "ExchangePath", "UNKNOWN");
    sptrAsyncLogger->info("Config # ExchangePath: {}", exchange_path);
    sptrAsyncLogger->info("Config # LogPath: {}", log_path);
    sptrAsyncLogger->info("Config # OutPath: {}", out_path);
    std::string bi_wss_url = uptrINIReader->Get("binance", "WssUrl", "UNKNOWN");
    std::string bi_api_url = uptrINIReader->Get("binance", "ApiUrl", "UNKNOWN");
    std::string bi_api_key = uptrINIReader->Get("binance", "Apikey", "UNKNOWN");
    std::string bi_secret_key = uptrINIReader->Get("binance", "SecretKey", "UNKNOWN");
    sptrAsyncLogger->info("Config # WssUrl: {}", bi_wss_url);
    sptrAsyncLogger->info("Config # ApiUrl: {}", bi_api_url);
    sptrAsyncLogger->info("Config # Apikey: {}", bi_api_key);
    std::string md_path = uptrINIReader->Get("replayer", "MdPath", "UNKNOWN");
    std::string md_date = uptrINIReader->Get("replayer", "MdDate", "UNKNOWN");
    sptrAsyncLogger->info("Config # MdPath: {}", md_path);
    sptrAsyncLogger->info("Config # MdDate: {}", md_date);
    std::string push_url = uptrINIReader->Get("notifier", "PushUrl", "UNKNOWN");
    std::string push_key = uptrINIReader->Get("notifier", "PushKey", "UNKNOWN");
    sptrAsyncLogger->info("Config # PushUrl: {}", push_url);
    sptrAsyncLogger->info("Config # PushKey: {}", push_key);


    /********** BiHelper **********/
    uptrBiHelper = std::make_unique<BiHelper>(bi_api_url, exchange_path);
    uptrBiHelper->InitSymbolIdxMap();
    uptrBiHelper->InitSymbolFilter();
    uptrBiHelper->GenerateSymbolList();


    /********** BiTrader **********/
    uptrBiTrader = std::make_unique<BiTrader>(bi_api_url, bi_api_key, bi_secret_key);
    /*uptrBiTrader->InsertOrder(1, "BOMEUSDT", Binance::OrderSide::BUY, 0.008, 2000.0, \
                              Binance::OrderType::LIMIT, Binance::TimeInForce::GTC);*/


#if !_BACK_TEST_
    /********** MDSocket **********/
    uptrMDSocket = std::make_unique<MDSocket>(bi_wss_url);
    uptrMDSocket->Start();
    uptrMDSocket->SetSelfTName((char *)"MDSocket");
#endif

    /********** MDReceiver **********/
    //uptrMDReceiver = std::make_unique<MDReceiver>(bi_api_url);
    //uptrMDReceiver->Start();
    //uptrMDReceiver->SetSelfTName((char *)"MDReceiver");

#if _BACK_TEST_
    /********** MDReplayer **********/
    uptrMDReplayer = std::make_unique<MDReplayer>(md_path);
    uptrMDReplayer->LoadHistoryMD(md_date);
    uptrMDReplayer->Start();
    uptrMDReplayer->SetSelfTName((char *)"MDReplayer");
#endif

    /********** Calculator **********/
    uptrCalculator = std::make_unique<Calculator>();
    uptrCalculator->Start();
    uptrCalculator->SetSelfTName((char *)"Calculator");


    /********** StrategyBOX **********/
    uptrStrategyBOX = std::make_unique<StrategyBOX>();
    //AdvancedSLR1 *aslr1 = new AdvancedSLR1(ASLR1);
    AdvancedSLR2 *aslr2 = new AdvancedSLR2(ASLR2);
    //MACross1 *macs1 = new MACross1(MACROSS1, 1000.0);
    //MACross2 *macs2 = new MACross2(MACROSS2, 1000.0);
    //GridTrader *grid = new GridTrader(GRID);
    //uptrStrategyBOX->EntrustStrategy(aslr1);
    uptrStrategyBOX->EntrustStrategy(aslr2);
    //uptrStrategyBOX->EntrustStrategy(macs1);
    //uptrStrategyBOX->EntrustStrategy(macs2);
    //uptrStrategyBOX->EntrustStrategy(grid);
    uptrStrategyBOX->Start();
    uptrStrategyBOX->SetSelfTName((char *)"StrategyBOX");


    /********** OrderManager **********/
    uptrOrderManager = std::make_unique<OrderManager>();
    uptrOrderManager->Start();
    uptrOrderManager->SetSelfTName((char *)"OrderManager");


    /********** AccTruster **********/
    uptrAccTruster = std::make_unique<AccTruster>(bi_api_url, bi_api_key, bi_secret_key);
    uptrAccTruster->Start();
    uptrAccTruster->SetSelfTName((char *)"AccTruster");


    /********** WatchDog **********/
    uptrWatchDog = std::make_unique<WatchDog>();
    uptrWatchDog->Start();
    uptrWatchDog->SetSelfTName((char *)"WatchDog");


    /********** PushDeer **********/
    uptrPushDeer = std::make_unique<PushDeer>(push_url, push_key);
    //uptrBiNotifier->PushDeer("BiQTP Start");


    /********** Hold for Join **********/
#if !_BACK_TEST_
    uptrMDSocket->Join();
#endif
    //uptrMDReceiver->Join();
#if _BACK_TEST_
    uptrMDReplayer->Join();
#endif
    uptrCalculator->Join();
    uptrStrategyBOX->Join();
    uptrOrderManager->Join();
    uptrAccTruster->Join();
    uptrWatchDog->Join();


    /********** 资源清理 **********/
    // 全局清理libcurl环境
    curl_global_cleanup();
    // 关闭并等待所有日志记录完毕
    spdlog::shutdown();

    return 0;
}

/********** END **********/
