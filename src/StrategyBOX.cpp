/*
 * File:        StrategyBOX.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: BOX of Strategy
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctime>
#include <chrono>

#include <vector>
#include <unordered_map>

#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Macro.h"
#include "MDRing.h"
#include "StrategyBOX.h"

// Extern
//extern std::unordered_map<std::string, int> symbolUMap;
//extern MDRing mdring[TOTAL_SYMBOL];
extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
StrategyBOX::StrategyBOX()
{
    runTime = std::chrono::steady_clock::now();
    nowTime = std::chrono::steady_clock::now();

    strategyVec.clear();
}

//##################################################//
//   Destructor
//##################################################//
StrategyBOX::~StrategyBOX()
{
    // ~
}

//##################################################//
//   线程运行实体
//##################################################//
void StrategyBOX::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 0;
    time_to_sleep.tv_nsec = 1000*10;   // 10us

	while( true )
	{
        nowTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - runTime);
        // 每1ms运行一次
        if( elapsed_time >= std::chrono::milliseconds(1) )
        {
            runTime = std::chrono::steady_clock::now();

            for(auto &strategy:strategyVec)
                strategy->Run();
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("StrategyBOX::Run() nanosleep() failed !");
	}
}

//##################################################//
//   注册策略
//##################################################//
void StrategyBOX::EntrustStrategy(Strategy *strategy)
{
    strategyVec.push_back(strategy);
}
