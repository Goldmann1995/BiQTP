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
#include <inih/INIReader.h>

#include <Utils.h>

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
std::shared_ptr<spdlog::logger> sptrAsyncLogger = nullptr;
INIReader *ptrINIReader = nullptr;
MDReceiver *ptrMDReceiver = nullptr;
Calculator *ptrCalculator = nullptr;
StrategyBOX *ptrStrategyBOX = nullptr;


/********** Main Entry **********/
int main(int argc, char *argv[])
{
    // 全局初始化libcurl环境
    curl_global_init(CURL_GLOBAL_ALL);

    if( argc<2 )
    {
		fprintf(stderr, "Error Usage: %s ConfigFile \n", argv[0]);
		return -1;
	}
    if( !Utils::IsFileExists(argv[1]) )
	{
        fprintf(stderr, "ZConfig Not Found: %s \n", argv[1]);
		return -2;
	}

    // inih
    //ptrINIReader = new INIReader("/home/Binance/BiQTP/etc/QTP.ini");
    ptrINIReader = new INIReader(argv[1]);
    if(ptrINIReader->ParseError()<0)
    {
        std::cout << "INI Init Error !" << std::endl;
        return -1;
    }

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
    // TODO 打印配置

    // Binance-QTP初始化
    std::string url = ptrINIReader->Get("binance", "ApiUrl", "UNKNOWN");
    BiIniter initer(url);
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();

    /********** MDReceiver **********/
    ptrMDReceiver = new MDReceiver("https://api.binance.com");
    ptrMDReceiver->Start();
    ptrMDReceiver->SetSelfTName((char *)"MDReceiver");

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

    /********** Hold for Join **********/
    ptrMDReceiver->Join();
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
