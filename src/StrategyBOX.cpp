/*
 * File:        StrategyBOX.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: BOX of Strategy
 */

#include <unistd.h>
#include <string>
#include <chrono>
// STL
#include <vector>
#include <unordered_map>
// 3rd-liv
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// QTP
#include "Macro.h"
#include "MDRing.h"
#include "Strategy.h"
#include "StrategyBOX.h"

// Extern
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
StrategyBOX::StrategyBOX()
{
    totalProfit = 0.0;
    totalCommission = 0.0;

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
#if !_BACK_TEST_
    time_to_sleep.tv_nsec = 1000*100;   // 100us
#else
    time_to_sleep.tv_nsec = 1000;   // 1us
#endif

	while( true )
	{
    #if !_BACK_TEST_
        nowTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - runTime);
        // 每1ms运行一次
        if( elapsed_time >= std::chrono::milliseconds(1) )
        {
            runTime = std::chrono::steady_clock::now();

            for(auto &strategy:strategyVec)
                strategy->Run();
        }
    #else
        for(auto& strategy : strategyVec)
            strategy->Run();
    #endif

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

//##################################################//
//   注销策略
//##################################################//
void StrategyBOX::DetrustStrategy(int strategy_id)
{
    for( auto it=strategyVec.begin(); it!=strategyVec.end(); it++ )
    {
        if( (*it)->GetStrategyId()==strategy_id )
        {
            strategyVec.erase(it);
            break;
        }
    }
}
