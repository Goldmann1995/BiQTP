/*
 * File:        WatchDog.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-21
 * Description: Watch Dog
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctime>
#include <chrono>
// STL
#include <vector>
#include <unordered_map>
// 3rd-lib
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <websocketpp/client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_client.hpp>
// Module
#include "WatchDog.h"
#include "BiIniter.h"
#include "MDRing.h"
#include "MDReceiver.h"
#include "MDSocket.h"
#include "Calculator.h"
#include "Strategy.h"
#include "StrategyBOX.h"
// Global
#include "Macro.h"
//#include "Global.h"

// Extern
extern MDRing mdring[TOTAL_SYMBOL];
extern std::unordered_map<std::string, int> symbolUMap;
extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;


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
        nowTime = std::chrono::steady_clock::now();
        std::chrono::seconds elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - runTime);
        // 每3s运行一次
        if( elapsed_time >= std::chrono::seconds(3) )
        {
            runTime = std::chrono::steady_clock::now();

            int md_index = mdring[0].GetMDIndex();
            int cal_ma_index = mdring[0].GetCalMAIndex();
            int cal_adr_index = mdring[0].GetCalADRIndex();
            sptrAsyncLogger->info("WatchDog::Run() MDIndex={} CalMAIndex={} CalADRIndex={}", \
                                  md_index, cal_ma_index, cal_adr_index);
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("WatchDog::Run() nanosleep() failed !");
	}
}
