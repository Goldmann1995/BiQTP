/*
 * File:        WatchDog.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-23
 * Description: Watch Dog for QTP
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
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
extern MDRing mdring[TOTAL_SYMBOL];
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
WatchDog::WatchDog()
{
    runTime = std::chrono::steady_clock::now();
    nowTime = std::chrono::steady_clock::now();
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
    time_to_sleep.tv_sec  = 3;   // 3s
    time_to_sleep.tv_nsec = 0;

	while( true )
	{
        // 每3s运行一次
        //nowTime = std::chrono::steady_clock::now();
        //std::chrono::seconds elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - runTime);
        //if( elapsed_time >= std::chrono::seconds(3) )
        {
            //runTime = std::chrono::steady_clock::now();

            //int md_index      = mdring[0].GetMDIndex();
            //int cal_ma_index  = mdring[0].GetCalMAIndex();
            //int cal_adr_index = mdring[0].GetCalADRIndex();
            /*sptrAsyncLogger->info("WatchDog::Run() MDIndex={} CalMAIndex={} CalADRIndex={}", \
                                   md_index, cal_ma_index, cal_adr_index);*/
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
        {
            sptrAsyncLogger->error("WatchDog::Run() nanosleep() failed !");
        }
	}
}
