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
#include <queue>
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
#include "OrderManager.h"
#include "Strategy.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern MDRing mdring[TOTAL_SYMBOL];
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;
extern std::unique_ptr<OrderManager> uptrOrderManager;
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

    mTotalProfitRate = 0.0;
    mTotalCommissionRate = 0.0;

    for(int i=0; i<TOTAL_SYMBOL; i++)
    {
        mStPoint[i].pointType = 0;
        mStPoint[i].buyMdIndex = -1;
        mStPoint[i].sellMdIndex = -1;
    }
}

//##################################################//
//   Destructor
//##################################################//
Strategy::~Strategy()
{
    // ~
}

int Strategy::GetStrategyId()
{
    return mStrategyID;
}

double Strategy::GetPositiveRate()
{
    return (double)mPositiveCnt/(double)mOrderCnt;
}

//##################################################//
//   AdvancedSLR1
//##################################################//
AdvancedSLR1::AdvancedSLR1(int id)
{
    mStrategyID = id;
}

AdvancedSLR1::~AdvancedSLR1()
{
    //
}

void AdvancedSLR1::Run()
{
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        int lastMdIndex = mdring[symbol_iter.second].GetMDIndex();
        int lastCycCnt = mdring[symbol_iter.second].GetCycleCnt();
        if(lastCycCnt==0 && lastMdIndex<670)
            return;
        
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr5m_p1 = mdring[symbol_iter.second].GetADRatio5m(60);
        double adr5m_p2 = mdring[symbol_iter.second].GetADRatio5m(120);
        double adr5m_p3 = mdring[symbol_iter.second].GetADRatio5m(180);
        double adr5m_p4 = mdring[symbol_iter.second].GetADRatio5m(240);
        double adr5m_p5 = mdring[symbol_iter.second].GetADRatio5m(300);

        if(mStPoint[symbol_iter.second].pointType == 0 || mStPoint[symbol_iter.second].pointType == 2)
        {
            if( -0.01<adr5m_p1 && adr5m_p1<0.01 && \
                -0.01<adr5m_p2 && adr5m_p2<0.01 && \
                -0.01<adr5m_p3 && adr5m_p3<0.01 && \
                -0.01<adr5m_p4 && adr5m_p4<0.01 && \
                -0.01<adr5m_p5 && adr5m_p5<0.01 && \
                 0.01<adr30s   && adr1m >0.018 )
            {
                mOrderCnt++;
                mStPoint[symbol_iter.second].buyMdIndex = lastMdIndex;
                mStPoint[symbol_iter.second].pointType = 1;
                sptrAsyncLogger->debug("AdvancedSLR1::Run() BuySignal # Symbol: {} ADR_30s: {:.4f} ADR_1m: {:.4f}", \
                                        symbol_iter.first, adr30s, adr1m);
            }
        }
        else if(mStPoint[symbol_iter.second].pointType == 1)
        {
            if(adr1m<0.0)
            {
                double buyPrice = mdring[symbol_iter.second].GetIndexPrice(mStPoint[symbol_iter.second].buyMdIndex);
                double sellPrice = mdring[symbol_iter.second].GetLastPrice();
                double profitRate = (sellPrice-buyPrice)/buyPrice;
                mStPoint[symbol_iter.second].sellMdIndex = lastMdIndex;
                mStPoint[symbol_iter.second].pointType = 2;

                if(profitRate>0.0)
                    mPositiveCnt++;
                else
                    mNegativeCnt++;

                mTotalProfitRate += profitRate;
                mTotalCommissionRate += 2.0/1000.0;
                sptrAsyncLogger->debug("AdvancedSLR1::Run() SellSignal # Symbol: {} WinRate: {:.2f} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, GetPositiveRate(), profitRate, mTotalProfitRate, mTotalCommissionRate);
                // PushDeer
                //std::string notifystr = symbol_iter.first;
                //notifystr += " Profit=";
                //notifystr += std::to_string(profitRate);
                //notifystr += " Total=";
                //notifystr += std::to_string(mTotalProfitRate);
                //uptrPushDeer->Notify(notifystr);
            }
        }
    }
}

//##################################################//
//   AdvancedSLR2
//##################################################//
AdvancedSLR2::AdvancedSLR2(int id)
{
    mStrategyID = id;
}

AdvancedSLR2::~AdvancedSLR2()
{
    //
}

void AdvancedSLR2::Run()
{    
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        int lastMdIndex = mdring[symbol_iter.second].GetMDIndex();
        int lastCycCnt = mdring[symbol_iter.second].GetCycleCnt();
        if(lastCycCnt==0 && lastMdIndex < 920)
            return;
        
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr2m = mdring[symbol_iter.second].GetADRatio2m(0);
        double adr3m = mdring[symbol_iter.second].GetADRatio3m(0);
        double adr5m = mdring[symbol_iter.second].GetADRatio5m(0);
        double adr5m_p1 = mdring[symbol_iter.second].GetADRatio5m(300);
        double adr5m_p2 = mdring[symbol_iter.second].GetADRatio5m(360);
        double adr5m_p3 = mdring[symbol_iter.second].GetADRatio5m(420);
        double adr5m_p4 = mdring[symbol_iter.second].GetADRatio5m(480);
        double adr5m_p5 = mdring[symbol_iter.second].GetADRatio5m(540);

        if(mStPoint[symbol_iter.second].pointType == 0 || mStPoint[symbol_iter.second].pointType == 2)
        {
            if( -0.01<adr5m_p1 && adr5m_p1<0.01 && \
                -0.01<adr5m_p2 && adr5m_p2<0.01 && \
                -0.01<adr5m_p3 && adr5m_p3<0.01 && \
                -0.01<adr5m_p4 && adr5m_p4<0.01 && \
                -0.01<adr5m_p5 && adr5m_p5<0.01 && \
                adr1m>0.003 && adr2m >0.006 && adr3m >0.012 && adr5m >0.02 )
            {
                mOrderCnt++;
                mStPoint[symbol_iter.second].buyMdIndex = lastMdIndex;
                mStPoint[symbol_iter.second].pointType = 1;
                sptrAsyncLogger->debug("AdvancedSLR2::Run() BuySignal # Symbol: {} ADR_1m: {:.4f} ADR_2m: {:.4f} ADR_3m: {:.4f} ADR_5m: {:.4f}", \
                                        symbol_iter.first, adr1m, adr2m, adr3m, adr5m);
                
            #if !_BACK_TEST_
                double buyPrice = mdring[symbol_iter.second].GetLastPrice();
                uptrOrderManager->PushSignal(ASLR2, symbol_iter.first, 1, buyPrice);
            #endif
            }
        }
        else if(mStPoint[symbol_iter.second].pointType == 1)
        {
            if(adr5m<0.0)
            {
                double buyPrice = mdring[symbol_iter.second].GetIndexPrice(mStPoint[symbol_iter.second].buyMdIndex);
                double sellPrice = mdring[symbol_iter.second].GetLastPrice();
                double profitRate = (sellPrice-buyPrice)/buyPrice;
                mStPoint[symbol_iter.second].sellMdIndex = lastMdIndex;
                mStPoint[symbol_iter.second].pointType = 2;

                if(profitRate>0.0)
                    mPositiveCnt++;
                else
                    mNegativeCnt++;

                mTotalProfitRate += profitRate;
                mTotalCommissionRate += 2.0/1000.0;
                sptrAsyncLogger->debug("AdvancedSLR2::Run() SellSignal # Symbol: {} WinRate: {:.2f} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, GetPositiveRate(), profitRate, mTotalProfitRate, mTotalCommissionRate);
            
            #if !_BACK_TEST_
                uptrOrderManager->PushSignal(ASLR2, symbol_iter.first, 2, sellPrice);
            #endif
            }
        }
    }
}

#if 0
//##################################################//
//   MACross1
//##################################################//
MACross1::MACross1(int id)
{
    mStrategyID = id;
}

MACross1::~MACross1()
{
    //
}

void MACross1::Run()
{    
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        int lastMdIndex = mdring[symbol_iter.second].GetMDIndex();
        int lastCycCnt = mdring[symbol_iter.second].GetCycleCnt();
        if(lastCycCnt==0 && lastMdIndex < 1800)
            return;
        
        double adr30s = mdring[symbol_iter.second].GetADRatio30s(0);
        double adr1m = mdring[symbol_iter.second].GetADRatio1m(0);
        double adr2m = mdring[symbol_iter.second].GetADRatio2m(0);
        double adr3m = mdring[symbol_iter.second].GetADRatio3m(0);

        double ma5m = mdring[symbol_iter.second].GetMA5m(0);
        double ma25m = mdring[symbol_iter.second].GetMA25m(0);
        double ma5mp = mdring[symbol_iter.second].GetMA5m(20);
        double ma25mp = mdring[symbol_iter.second].GetMA25m(20);

        if(mStPoint[symbol_iter.second].pointType == 0 || mStPoint[symbol_iter.second].pointType == 2)
        {
            if( ((ma5m-ma25m)<0.000001 && (ma5m-ma5mp)<0) || \
                adr30s<-0.002 || adr1m<-0.003 || adr2m <-0.006 || adr3m <-0.01 )
            {
                mOrderCnt++;
                mStPoint[symbol_iter.second].buyMdIndex = lastMdIndex;
                mStPoint[symbol_iter.second].pointType == 1;
                sptrAsyncLogger->debug("AdvancedSLR2::Run() BuySignal # Symbol: {} ADR_1m: {:.4f} ADR_2m: {:.4f} ADR_3m: {:.4f} ADR_5m: {:.4f}", \
                                        symbol_iter.first, adr1m, adr2m, adr3m, adr5m);
            }
        }
        else if(mStPoint[symbol_iter.second].pointType == 1)
        {
            if(adr1m<0.0)
            {
                double buyPrice = mdring[symbol_iter.second].GetIndexPrice(mStPoint[symbol_iter.second].buyMdIndex);
                double sellPrice = mdring[symbol_iter.second].GetLastPrice();
                double profitRate = (sellPrice-buyPrice)/buyPrice;
                mStPoint[symbol_iter.second].sellMdIndex = lastMdIndex;
                mStPoint[symbol_iter.second].pointType == 2;

                if(profitRate>0.0)
                    mPositiveCnt++;
                else
                    mNegativeCnt++;

                mTotalProfitRate += profitRate;
                mTotalCommissionRate += 2.0/1000.0;
                sptrAsyncLogger->debug("AdvancedSLR2::Run() SellSignal # Symbol: {} WinRate: {:.2f} Profit: {:.4f} Total: {:.4f} Commission: {:.4f}", \
                                        symbol_iter.first, GetPositiveRate(), profitRate, mTotalProfitRate, mTotalCommissionRate);
            }
        }


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
#endif

#if 0
//##################################################//
//   MACross2
//##################################################//

MACross2::MACross2(int id)
{
    mStrategyID = id;
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
#endif

#if 0
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
        last_price = mdring[375].GetLastPrice();
        return;
    }
    if( check_time < std::chrono::seconds(5) )
    {
        return;
    }

    checkTime = std::chrono::steady_clock::now();
    double now_price = mdring[375].GetLastPrice();
    //sptrAsyncLogger->debug("GridTrader::Run() now_price: {:.5f} last_price: {:.5f}", now_price, last_price );
    //std::string notifystr = "BOMEUSDT now price: " + std::to_string(now_price);
    //uptrPushDeer->Notify(notifystr);
    if( now_price-last_price >= 0.0003 )
    {
        total_pos -= 20000.0;
        total_captical += 20000.0*now_price*0.999;
        sptrAsyncLogger->debug("GridTrader::Run() Sell # price: {:.5f} pos {:.1f} cap {:.1f} total {:.1f}", \
                                now_price, total_pos, total_captical, total_pos*now_price+total_captical );
        last_price = now_price;
    }
    else if( now_price-last_price <= -0.0003 )
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
#endif
