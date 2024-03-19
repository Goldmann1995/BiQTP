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

#include <log/log.h>

#include "Macro.h"
#include "MDRing.h"
#include "Strategy.h"

// Extern
extern std::unordered_map<std::string, int> symbolUMap;
extern MDRing mdring[TOTAL_SYMBOL];


//##################################################//
//   Constructor
//##################################################//
Strategy::Strategy()
{
    mStrategyID = -1;

    mCapital = 0.0;
    mTotalProfit = 0.0;
    mTotalCommission = 0.0;

    startTime = std::chrono::steady_clock::now();
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
//   AdvancedSLR1
//##################################################//
AdvancedSLR1::AdvancedSLR1(int id, double captical)
{
    mStrategyID = id;
    mCapital = captical;
}

AdvancedSLR1::~AdvancedSLR1()
{
    //
}

void AdvancedSLR1::Run()
{
    nowTime = std::chrono::steady_clock::now();
    std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    if( start_time < std::chrono::seconds(70) )
        return;

    for(const auto& symbol_iter:symbolUMap)
    {
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);

        if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)>=0)
        {
            if(adr30s<0.0)
            {
                double profit = mdring[symbol_iter.second].GetProfit(mStrategyID, mCapital);
                mdring[symbol_iter.second].ClearBuyIndex(mStrategyID);
                mTotalProfit += profit;
                mTotalCommission += 2*mCapital/1000.0;
                LOG_DEBUG("AdvancedSLR1::Run() SellSignal # Symbol: %s Profit: %f Total: %f ", \
                           symbol_iter.first.c_str(), profit, mTotalProfit-mTotalCommission);
            }
        }

        if(adr30s>0.01 && adr1m >0.018)
        {
            if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)<0)
            {
                mdring[symbol_iter.second].SetBuyIndex(mStrategyID);
                LOG_DEBUG("AdvancedSLR1::Run() BuySignal # Symbol: %s ADR_30s: %f ADR_1m: %f", \
                           symbol_iter.first.c_str(), adr30s, adr1m);
            }
        } 
    }
}

//##################################################//
//   AdvancedSLR2
//##################################################//
AdvancedSLR2::AdvancedSLR2(int id, double captical)
{
    mStrategyID = id;
    mCapital = captical;
}

AdvancedSLR2::~AdvancedSLR2()
{
    //
}

void AdvancedSLR2::Run()
{
    nowTime = std::chrono::steady_clock::now();
    std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    if( start_time < std::chrono::seconds(320) )
        return;
    
    for(const auto& symbol_iter:symbolUMap)
    {
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr2m = mdring[symbol_iter.second].GetADRatio2m(0);
        double adr3m = mdring[symbol_iter.second].GetADRatio3m(0);
        double adr5m = mdring[symbol_iter.second].GetADRatio5m(0);

        if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)>=0)
        {
            if(adr30s<0.0)
            {
                double profit = mdring[symbol_iter.second].GetProfit(mStrategyID, mCapital);
                mdring[symbol_iter.second].ClearBuyIndex(mStrategyID);
                mdring[symbol_iter.second].SetSellIndex(mStrategyID);
                mTotalProfit += profit;
                mTotalCommission += 2*mCapital/1000.0;
                LOG_DEBUG("Strategy::AdvancedSLR2() SellSignal # Symbol: %s Profit: %f Total: %f ", \
                           symbol_iter.first.c_str(), profit, mTotalProfit-mTotalCommission);
            }
        }

        if( adr1m>0.003 && adr2m >0.006 && adr3m >0.012 && adr5m >0.02 &&\
            mdring[symbol_iter.second].GetSellDuration(mStrategyID) > 6 )
        {
            if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)<0)
            {
                mdring[symbol_iter.second].SetBuyIndex(mStrategyID);
                LOG_DEBUG("Strategy::AdvancedSLR2() BuySignal # Symbol: %s ADR_1m: %f ADR_2m: %f ADR_3m: %f ADR_5m: %f", \
                           symbol_iter.first.c_str(), adr1m, adr2m, adr3m, adr5m);
            }
        }
    }
}
