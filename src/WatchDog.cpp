/*
 * File:        WatchDog.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-26
 * Description: Watch Dog for QTP
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
// STL
#include <vector>
#include <unordered_map>
// 3rd-lib
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>
// Module
#include "WatchDog.h"
#include "BiHelper.h"
#include "MDRing.h"
#include "MDSocket.h"
#include "MDReceiver.h"
#include "Calculator.h"
#include "Strategy.h"
#include "StrategyBOX.h"
// Global
#include "Macro.h"

// Extern
extern int SymbolMaxIndex;
extern MDRing mdring[TOTAL_SYMBOL];
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
WatchDog::WatchDog()
{
    // ~
}

//##################################################//
//   Destructor
//##################################################//
WatchDog::~WatchDog()
{
    // ~
}

//##################################################//
//   线程运行实体
//##################################################//
void WatchDog::Run()
{
    struct timespec time_to_sleep;
#if !_BACK_TEST_
    time_to_sleep.tv_sec  = 5;   // 5s
#else
    time_to_sleep.tv_sec  = 1;   // 1s
#endif
    time_to_sleep.tv_nsec = 0;

	while( true )
	{
    #if !_BACK_TEST_
        // 每5s运行一次
        int md_index  = RING_SIZE;
        int adr_index = RING_SIZE;
        int ma_index  = RING_SIZE;
        for(int i=0; i<=SymbolMaxIndex; i++)
        {
            md_index  = std::min(md_index, mdring[i].GetMDIndex());
            adr_index = std::min(md_index, mdring[i].GetADRIndex());
            ma_index  = std::min(md_index, mdring[i].GetMAIndex());
        }
        sptrAsyncLogger->info("WatchDog::Run() MDIndex={} ADRIndex={} MAIndex={}", \
                               md_index, adr_index, ma_index);
    #else
        // 每1s运行一次
        int md_index  = mdring[0].GetMDIndex();
        int adr_index = mdring[0].GetADRIndex();
        int ma_index  = mdring[0].GetMAIndex();
        sptrAsyncLogger->info("WatchDog::Run() MDIndex={} ADRIndex={} MAIndex={}", \
                               md_index, adr_index, ma_index);
    #endif

        // Interval-Sleep
        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
        {
            sptrAsyncLogger->error("WatchDog::Run() nanosleep() failed !");
        }
	}
}
