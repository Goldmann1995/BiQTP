/*
 * File:        Strategy.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: Strategy of Quanter
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <ctime>
#include <chrono>
#include <unordered_map>

#include <curl/curl.h>
#include <rapidjson/document.h>
#include <log/log.h>

#include "Strategy.h"
#include "MDRing.h"

#define TOTAL_SYMBOL   600

extern std::unordered_map<std::string, int> symbolUMap;
extern MDRing mdring[TOTAL_SYMBOL];


//##################################################//
//   Constructor
//##################################################//
Strategy::Strategy()
{
    startTime = std::chrono::steady_clock::now();
    runTime = std::chrono::steady_clock::now();
    nowTime = std::chrono::steady_clock::now();
}

//##################################################//
//   Destructor
//##################################################//
Strategy::~Strategy()
{
    // ~
}

//##################################################//
//   线程运行实体
//##################################################//
void Strategy::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 0;
    time_to_sleep.tv_nsec = 1000*10;   // 10us

	while( true )
	{
        nowTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - runTime);
        std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
        if( elapsed_time >= std::chrono::milliseconds(1) )
        {
            runTime = std::chrono::steady_clock::now();

            // ST1
            if( start_time >= std::chrono::seconds(70) )
                AdvancedSLR1();
            // ST2
            if( start_time >= std::chrono::seconds(320) )
                AdvancedSLR2();
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result == 0 )
        {
            //std::cout << "Slept for 1 second." << std::endl;
        }
        else
        {
            //std::cerr << "nanosleep failed." << std::endl;
        }
	}
}

//##################################################//
//   ~
//##################################################//
void Strategy::AdvancedSLR1()
{
    for(const auto& symbol_iter:symbolUMap)
    {
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);

        if(mdring[symbol_iter.second].GetBuyIndex(ASLR1)>=0)
        {
            if(adr30s<0.0)
            {
                double profit = mdring[symbol_iter.second].GetProfit(ASLR1, 1000.0);
                mdring[symbol_iter.second].ClearBuyIndex(ASLR1);
                profit_ASLR1 += profit - 1000.0*2/1000.0;
                LOG_DEBUG("Strategy::AdvancedSLR1() SellSignal # Symbol: %s Profit: %f Total: %f ", \
                           symbol_iter.first.c_str(), profit, profit_ASLR1);
            }
        }

        if(adr30s>0.01 && adr1m >0.015)
        {
            if(mdring[symbol_iter.second].GetBuyIndex(ASLR1)<0)
            {
                mdring[symbol_iter.second].SetBuyIndex(ASLR1);
                LOG_DEBUG("Strategy::AdvancedSLR1() BuySignal # Symbol: %s ADR_30s: %f ADR_1m: %f", \
                           symbol_iter.first.c_str(), adr30s, adr1m);
            }
        } 
    }
}

//##################################################//
//   ~
//##################################################//
void Strategy::AdvancedSLR2()
{
    for(const auto& symbol_iter:symbolUMap)
    {
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr2m = mdring[symbol_iter.second].GetADRatio2m(0);
        double adr3m = mdring[symbol_iter.second].GetADRatio3m(0);
        double adr5m = mdring[symbol_iter.second].GetADRatio5m(0);

        if(mdring[symbol_iter.second].GetBuyIndex(ASLR2)>=0)
        {
            if(adr30s<0.0)
            {
                double profit = mdring[symbol_iter.second].GetProfit(ASLR2, 1000.0);
                mdring[symbol_iter.second].ClearBuyIndex(ASLR2);
                mdring[symbol_iter.second].SetSellIndex(ASLR2);
                profit_ASLR2+= profit - 1000.0*2/1000.0;
                LOG_DEBUG("Strategy::AdvancedSLR2() SellSignal # Symbol: %s Profit: %f Total: %f ", \
                           symbol_iter.first.c_str(), profit, profit_ASLR2);
            }
        }

        if( adr1m>0.002 && adr2m >0.004 && adr3m >0.006 && adr5m >0.01 &&\
            mdring[symbol_iter.second].GetSellGap(ASLR2) > 6 )
        {
            if(mdring[symbol_iter.second].GetBuyIndex(ASLR2)<0)
            {
                mdring[symbol_iter.second].SetBuyIndex(ASLR2);
                LOG_DEBUG("Strategy::AdvancedSLR2() BuySignal # Symbol: %s ADR_1m: %f ADR_2m: %f ADR_3m: %f ADR_5m: %f", \
                           symbol_iter.first.c_str(), adr1m, adr2m, adr3m, adr5m);
            }
        }
    }
}
