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
#include <chrono>
// STL
#include <unordered_map>
// 3rd-lib
#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// ~
#include "Macro.h"
#include "MDRing.h"
#include "PushDeer.h"
#include "Strategy.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern MDRing mdring[TOTAL_SYMBOL];
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;
extern std::unique_ptr<PushDeer> uptrPushDeer;


//##################################################//
//   Constructor
//##################################################//
Strategy::Strategy()
{
    mStrategyID = -1;

    mPositiveCnt = 0;
    mNegativeCnt = 0;
    mOrderCnt = 0;

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

double Strategy::GetPositiveRate()
{
    return (double)mPositiveCnt/(double)mOrderCnt;
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
    //nowTime = std::chrono::steady_clock::now();
    //std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    //if( start_time < std::chrono::seconds(670) )
    //    return;

    for(const auto& symbol_iter:symbol2idxUMap)
    {
        if(mdring[symbol_iter.second].GetMDIndex() < 670)
            return;
        
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr5m_p1 = mdring[symbol_iter.second].GetADRatio5m(20);
        double adr5m_p2 = mdring[symbol_iter.second].GetADRatio5m(40);
        double adr5m_p3 = mdring[symbol_iter.second].GetADRatio5m(60);
        double adr5m_p4 = mdring[symbol_iter.second].GetADRatio5m(80);
        double adr5m_p5 = mdring[symbol_iter.second].GetADRatio5m(100);

        if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)>=0)
        {
            if(adr30s<0.0)
            {
                double profit = mdring[symbol_iter.second].GetProfit(mStrategyID, mCapital);
                mdring[symbol_iter.second].ClearBuyIndex(mStrategyID);
                mdring[symbol_iter.second].SetSellIndex(mStrategyID);

                if(profit>0.0)
                    mPositiveCnt++;
                else
                    mNegativeCnt++;

                mTotalProfit += profit;
                mTotalCommission += 2*mCapital/1000.0;
                sptrAsyncLogger->debug("AdvancedSLR1::Run() SellSignal # Symbol: {} WinRate: {:.2f} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, GetPositiveRate(), profit, mTotalProfit, mTotalCommission);
            }
        }

        if( -0.01<adr5m_p1 && adr5m_p1<0.01 && \
            -0.01<adr5m_p2 && adr5m_p2<0.01 && \
            -0.01<adr5m_p3 && adr5m_p3<0.01 && \
            -0.01<adr5m_p4 && adr5m_p4<0.01 && \
            -0.01<adr5m_p5 && adr5m_p5<0.01 && \
            adr30s>0.01 && adr1m >0.018 && \
            mdring[symbol_iter.second].GetSellDuration(mStrategyID) > 10 )
        {
            if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)<0)
            {
                mOrderCnt++;
                mdring[symbol_iter.second].SetBuyIndex(mStrategyID);
                sptrAsyncLogger->debug("AdvancedSLR1::Run() BuySignal # Symbol: {} ADR_30s: {:.4f} ADR_1m: {:.4f}", \
                                        symbol_iter.first, adr30s, adr1m);
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
    //nowTime = std::chrono::steady_clock::now();
    //std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    //if( start_time < std::chrono::seconds(920) )
    //    return;
    
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        if(mdring[symbol_iter.second].GetMDIndex() < 920)
            return;
        
        //double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr2m = mdring[symbol_iter.second].GetADRatio2m(0);
        double adr3m = mdring[symbol_iter.second].GetADRatio3m(0);
        double adr5m = mdring[symbol_iter.second].GetADRatio5m(0);
        double adr5m_p1 = mdring[symbol_iter.second].GetADRatio5m(100);
        double adr5m_p2 = mdring[symbol_iter.second].GetADRatio5m(120);
        double adr5m_p3 = mdring[symbol_iter.second].GetADRatio5m(140);
        double adr5m_p4 = mdring[symbol_iter.second].GetADRatio5m(160);
        double adr5m_p5 = mdring[symbol_iter.second].GetADRatio5m(180);

        if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)>=0)
        {
            //if(mdring[symbol_iter.second].EstimateBuyMax(mStrategyID))
            if(adr1m<0.0)
            {
                double profit = mdring[symbol_iter.second].GetProfit(mStrategyID, mCapital);
                mdring[symbol_iter.second].ClearBuyIndex(mStrategyID);
                mdring[symbol_iter.second].SetSellIndex(mStrategyID);

                if(profit>0.0)
                    mPositiveCnt++;
                else
                    mNegativeCnt++;

                mTotalProfit += profit;
                mTotalCommission += 2*mCapital/1000.0;
                sptrAsyncLogger->debug("AdvancedSLR2::Run() SellSignal # Symbol: {} WinRate: {:.2f} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, GetPositiveRate(), profit, mTotalProfit, mTotalCommission);
            }
        }

        if( -0.01<adr5m_p1 && adr5m_p1<0.01 && \
            -0.01<adr5m_p2 && adr5m_p2<0.01 && \
            -0.01<adr5m_p3 && adr5m_p3<0.01 && \
            -0.01<adr5m_p4 && adr5m_p4<0.01 && \
            -0.01<adr5m_p5 && adr5m_p5<0.01 && \
            adr1m>0.003 && adr2m >0.006 && adr3m >0.012 && adr5m >0.02 && \
            mdring[symbol_iter.second].GetSellDuration(mStrategyID) > 10 )
        {
            if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)<0)
            {
                mOrderCnt++;
                mdring[symbol_iter.second].SetBuyIndex(mStrategyID);
                sptrAsyncLogger->debug("AdvancedSLR2::Run() BuySignal # Symbol: {} ADR_1m: {:.4f} ADR_2m: {:.4f} ADR_3m: {:.4f} ADR_5m: {:.4f}", \
                                        symbol_iter.first, adr1m, adr2m, adr3m, adr5m);
            }
        }
    }
}

//##################################################//
//   MACross1
//##################################################//

MACross1::MACross1(int id, double captical)
{
    mStrategyID = id;
    mCapital = captical;
}

MACross1::~MACross1()
{
    //
}

void MACross1::Run()
{
    //nowTime = std::chrono::steady_clock::now();
    //std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    //if( start_time < std::chrono::seconds(1800) )
    //    return;
    
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        if(mdring[symbol_iter.second].GetMDIndex() < 1800)
            return;
        
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr2m = mdring[symbol_iter.second].GetADRatio2m(0);
        double adr3m = mdring[symbol_iter.second].GetADRatio3m(0);

        double ma5m = mdring[symbol_iter.second].GetMA5m(0);
        double ma25m = mdring[symbol_iter.second].GetMA25m(0);
        double ma5mp = mdring[symbol_iter.second].GetMA5m(20);
        double ma25mp = mdring[symbol_iter.second].GetMA25m(20);

        if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)>=0)
        {
            if( ((ma5m-ma25m)<0.000001 && (ma5m-ma5mp)<0) || \
                adr30s<-0.002 || adr1m<-0.003 || adr2m <-0.006 || adr3m <-0.01 )
            {
                double profit = mdring[symbol_iter.second].GetProfit(mStrategyID, mCapital);
                mdring[symbol_iter.second].ClearBuyIndex(mStrategyID);
                mdring[symbol_iter.second].SetSellIndex(mStrategyID);

                mTotalProfit += profit;
                mTotalCommission += (mCapital/1000.0)*2;
                sptrAsyncLogger->debug("MACross1::Run() SellSignal # Symbol: {} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, profit, mTotalProfit, mTotalCommission);
            }
        }

        if( (ma5m-ma25m)<0.000001 && (ma5m-ma5mp)>0 && (ma25m-ma25mp)>0 && \
            adr1m>0.003 && adr2m >0.006 && adr3m >0.01 && \
            mdring[symbol_iter.second].GetSellDuration(mStrategyID) > 10 )
        {
            if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)<0)
            {
                mdring[symbol_iter.second].SetBuyIndex(mStrategyID);
                sptrAsyncLogger->debug("MACross1::Run() BuySignal # Symbol: {}", symbol_iter.first);
            }
        }
    }
}

//##################################################//
//   MACross2
//##################################################//

MACross2::MACross2(int id, double captical)
{
    mStrategyID = id;
    mCapital = captical;
}

MACross2::~MACross2()
{
    //
}

void MACross2::Run()
{
    //nowTime = std::chrono::steady_clock::now();
    //std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    //if( start_time < std::chrono::seconds(6060) )
    //    return;
    
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        if(mdring[symbol_iter.second].GetMDIndex() < 6060)
            return;

        double ma25m = mdring[symbol_iter.second].GetMA25m(0);
        double ma100m = mdring[symbol_iter.second].GetMA100m(0);
        double ma25m_p = mdring[symbol_iter.second].GetMA25m(20);
        double ma100m_p = mdring[symbol_iter.second].GetMA100m(20);

        if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)>=0)
        {
            if( (ma25m-ma100m)<0.000001 && (ma25m-ma25m_p)<0 )
            {
                double profit = mdring[symbol_iter.second].GetProfit(mStrategyID, mCapital);
                mdring[symbol_iter.second].ClearBuyIndex(mStrategyID);
                mdring[symbol_iter.second].SetSellIndex(mStrategyID);

                mTotalProfit += profit;
                mTotalCommission += (mCapital/1000.0)*2;
                sptrAsyncLogger->debug("MACross2::Run() SellSignal # Symbol: {} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, profit, mTotalProfit, mTotalCommission);
            }
        }

        if( (ma25m-ma100m)<0.000001 && (ma25m-ma25m_p)>0 && (ma100m-ma100m_p)>0 && \
            (ma25m-ma25m_p) > (ma100m-ma100m_p) && \
            mdring[symbol_iter.second].GetSellDuration(mStrategyID) > 20 )
        {
            if(mdring[symbol_iter.second].GetBuyIndex(mStrategyID)<0)
            {
                mdring[symbol_iter.second].SetBuyIndex(mStrategyID);
                sptrAsyncLogger->debug("MACross2::Run() BuySignal # Symbol: {}", symbol_iter.first);
            }
        }
    }
}

//##################################################//
//   GridTrader
//##################################################//

GridTrader::GridTrader(int id)
{
    mStrategyID = id;

    total_pos = 200000.0;
    total_captical = 2000.0;
    total_commission = 0.0;

    last_price = 0.0;
    checkTime = std::chrono::steady_clock::now();

    sptrAsyncLogger->debug("GridTrader::Run() Buy # pos {:.4f} cap {:.4f} total {:.4f}", \
                            total_pos, total_captical, total_pos*0.0144+total_captical );
}

GridTrader::~GridTrader()
{
    //
}

void GridTrader::Run()
{
    nowTime = std::chrono::steady_clock::now();
    std::chrono::seconds start_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime);
    std::chrono::seconds check_time = std::chrono::duration_cast<std::chrono::seconds>(nowTime - checkTime);
    if( start_time < std::chrono::seconds(30) )
    {
        last_price = mdring[376].GetLastPrice();
        return;
    }
    if( check_time < std::chrono::seconds(10) )
    {
        return;
    }

    checkTime = std::chrono::steady_clock::now();
    double now_price = mdring[376].GetLastPrice();
    sptrAsyncLogger->debug("GridTrader::Run() now_price: {:.5f} last_price: {:.5f}", now_price, last_price );
    //std::string notifystr = "BOMEUSDT now price: " + std::to_string(now_price);
    //uptrPushDeer->Notify(notifystr);
    if( now_price-last_price >= 0.0002 )
    {
        total_pos -= 20000.0;
        total_captical += 20000.0*now_price*0.999;
        sptrAsyncLogger->debug("GridTrader::Run() Sell # price: {:.5f} pos {:.1f} cap {:.1f} total {:.1f}", \
                                now_price, total_pos, total_captical, total_pos*now_price+total_captical );
        last_price = now_price;
    }
    else if( now_price-last_price <= -0.0002 )
    {
        total_pos += 20000.0;
        total_captical -= 20000.0*now_price*1.001;
        sptrAsyncLogger->debug("GridTrader::Run() Buy # price: {:.5f} pos {:.1f} cap {:.1f} total {:.1f}", \
                                    now_price, total_pos, total_captical, total_pos*now_price+total_captical );
        last_price = now_price;
    }
    else
    {
        return;
    }
}

