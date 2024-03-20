/*
 * File:        MDRing.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: MarketData & CompuRes Ring
 */

#include <iostream>
#include <string>
#include <algorithm>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "MDRing.h"

extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
MDRing::MDRing()
{
    md_index = 0;
    cal_ma_index = 0;
    cal_adr_index = 0;
    for(int i=0; i<ST_SIZE; i++)
    {
        buy_index[i] = -1;
        sell_index[i] = -1;
    }
    cycle_cnt = 0;
}

//##################################################//
//   Destructor
//##################################################//
MDRing::~MDRing()
{
    //
}

//##################################################//
//   设置Symbol名称
//##################################################//
void MDRing::SetSymbolName(const std::string& symbol)
{
    mSymbol = symbol;
}

//##################################################//
//   获取Symbol名称
//##################################################//
const std::string& MDRing::GetSymbolName()
{
    return mSymbol;
}

void MDRing::SetBuyIndex(int stidx)
{
    buy_index[stidx] = cal_adr_index;
}

void MDRing::ClearBuyIndex(int stidx)
{
    buy_index[stidx] = -1;
}

int MDRing::GetBuyIndex(int stidx)
{
    return buy_index[stidx];
}

bool MDRing::EstimateBuyMax(int stidx)
{
    double max_price = 0.0;
    for(int i=buy_index[stidx]; ; i++)
    {
        int index = i%RING_SIZE;
        max_price = std::max(max_price, price[index]);
        if(index == md_index)
            break;
    }
    if( max_price > price[md_index] )
        return true;
    else
        return false;
}

void MDRing::SetSellIndex(int stidx)
{
    sell_index[stidx] = cal_adr_index;
}

void MDRing::ClearSellIndex(int stidx)
{
    sell_index[stidx] = -1;
}

int MDRing::GetSellIndex(int stidx)
{
    return sell_index[stidx];
}

int MDRing::GetSellDuration(int stidx)
{
    return cal_adr_index-sell_index[stidx];
}

double MDRing::GetProfit(int stidx, double base)
{
    return (price[cal_adr_index]-price[buy_index[stidx]])*base/price[buy_index[stidx]];
}

//##################################################//
//   更新行情
//##################################################//
void MDRing::PushMD(double last_price)
{
    int index = md_index+1%RING_SIZE;
    price[index] = last_price;
    if(md_index+1==RING_SIZE)
        cycle_cnt++;
    md_index=index;
}

//##################################################//
//   计算移动平均值
//##################################################//
void MDRing::CalMovingAverage()
{
    if(cal_ma_index==md_index)
        return;
    else   // cal_ma_index
    {
        // tips summer@20240319 - [0]位并没有值
        int next_index = cal_ma_index+1%RING_SIZE;
        mt5m += price[next_index];
        mt25m += price[next_index];
        mt100m += price[next_index];

        if(cycle_cnt == 0)
        {
            if(next_index > INTERVAL5M)
            {
                mt5m -= price[next_index-INTERVAL5M];
                ma5m[next_index] = mt5m/INTERVAL5M;
            }

            if(next_index > INTERVAL25M)
            {
                mt25m -= price[next_index-INTERVAL25M];
                ma25m[next_index] = mt25m/INTERVAL25M;
            }

            if(next_index > INTERVAL100M)
            {
                mt100m -= price[next_index-INTERVAL100M];
                ma100m[next_index] = mt100m/INTERVAL100M;
            }
        }
        else
        {
            if(next_index > INTERVAL5M)
            {
                mt5m -= price[next_index-INTERVAL5M];
                ma5m[next_index] = mt5m/INTERVAL5M;
            }
            else
            {
                mt5m -= price[RING_SIZE-INTERVAL5M-next_index];
                ma5m[next_index] = mt5m/INTERVAL5M; 
            }

            if(next_index > INTERVAL25M)
            {
                mt25m -= price[next_index-INTERVAL25M];
                ma25m[next_index] = mt25m/INTERVAL25M;
            }
            else
            {
                mt25m -= price[RING_SIZE-INTERVAL25M-next_index];
                ma25m[next_index] = mt25m/INTERVAL25M; 
            }

            if(next_index > INTERVAL100M)
            {
                mt100m -= price[next_index-INTERVAL100M];
                ma100m[next_index] = mt100m/INTERVAL100M;
            }
            else
            {
                mt100m -= price[RING_SIZE-INTERVAL100M-next_index];
                ma100m[next_index] = mt100m/INTERVAL100M; 
            }
        }

        cal_ma_index = next_index;
    }
}

void MDRing::CalADRatio()
{
    if(cal_adr_index == md_index)
        return;
    else   // cal_adr_index
    {
        // tips summer@20240319 - [0]位并没有值
        int next_index = cal_adr_index+1%RING_SIZE;

        if(cycle_cnt == 0)
        {
            if(next_index > INTERVAL30S)
            {
                adr30s[next_index] = (price[next_index]-price[next_index-INTERVAL30S])/price[next_index-INTERVAL30S];
            }

            if(next_index > INTERVAL1M)
            {
                adr1m[next_index] = (price[next_index]-price[next_index-INTERVAL1M])/price[next_index-INTERVAL1M];
            }

            if(next_index > INTERVAL2M)
            {
                adr2m[next_index] = (price[next_index]-price[next_index-INTERVAL2M])/price[next_index-INTERVAL2M];
            }

            if(next_index > INTERVAL3M)
            {
                adr3m[next_index] = (price[next_index]-price[next_index-INTERVAL3M])/price[next_index-INTERVAL3M];
            }

            if(next_index > INTERVAL5M)
            {
                adr5m[next_index] = (price[next_index]-price[next_index-INTERVAL5M])/price[next_index-INTERVAL5M];
            }

            if(next_index > INTERVAL10M)
            {
                adr10m[next_index] = (price[next_index]-price[next_index-INTERVAL10M])/price[next_index-INTERVAL10M];
            }

            if(next_index > INTERVAL20M)
            {
                adr20m[next_index] = (price[next_index]-price[next_index-INTERVAL20M])/price[next_index-INTERVAL20M];
            }

            if(next_index > INTERVAL30M)
            {
                adr30m[next_index] = (price[next_index]-price[next_index-INTERVAL30M])/price[next_index-INTERVAL30M];
            }

            if(next_index > INTERVAL60M)
            {
                adr60m[next_index] = (price[next_index]-price[next_index-INTERVAL60M])/price[next_index-INTERVAL60M];
            }
        }
        else
        {
            if(next_index > INTERVAL30S)
            {
                adr30s[next_index] = (price[next_index]-price[next_index-INTERVAL30S])/price[next_index-INTERVAL30S];
            }
            else
            {
                adr30s[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL30S+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL30S+next_index];
            }

            if(next_index > INTERVAL1M)
            {
                adr1m[next_index] = (price[next_index]-price[next_index-INTERVAL1M])/price[next_index-INTERVAL1M];
            }
            else
            {
                adr1m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL1M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL1M+next_index];
            }

            if(next_index > INTERVAL2M)
            {
                adr2m[next_index] = (price[next_index]-price[next_index-INTERVAL2M])/price[next_index-INTERVAL2M];
            }
            else
            {
                adr2m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL2M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL2M+next_index];
            }

            if(next_index > INTERVAL3M)
            {
                adr3m[next_index] = (price[next_index]-price[next_index-INTERVAL3M])/price[next_index-INTERVAL3M];
            }
            else
            {
                adr3m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL3M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL3M+next_index];
            }

            if(next_index > INTERVAL5M)
            {
                adr5m[next_index] = (price[next_index]-price[next_index-INTERVAL5M])/price[next_index-INTERVAL5M];
            }
            else
            {
                adr5m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL5M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL5M+next_index];
            }

            if(next_index > INTERVAL10M)
            {
                adr10m[next_index] = (price[next_index]-price[next_index-INTERVAL10M])/price[next_index-INTERVAL10M];
            }
            else
            {
                adr10m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL10M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL10M+next_index];
            }

            if(next_index > INTERVAL20M)
            {
                adr20m[next_index] = (price[next_index]-price[next_index-INTERVAL20M])/price[next_index-INTERVAL20M];
            }
            else
            {
                adr20m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL20M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL20M+next_index];
            }

            if(next_index > INTERVAL30M)
            {
                adr30m[next_index] = (price[next_index]-price[next_index-INTERVAL30M])/price[next_index-INTERVAL30M];
            }
            else
            {
                adr30m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL30M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL30M+next_index];
            }

            if(next_index > INTERVAL60M)
            {
                adr60m[next_index] = (price[next_index]-price[next_index-INTERVAL60M])/price[next_index-INTERVAL60M];
            }
            else
            {
                adr60m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL60M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL60M+next_index];
            }
        }

        cal_adr_index = next_index;
    }
}

// tips summer@20240319 - 需要重写 考虑cycle
double MDRing::GetADRatio30s(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL30S)
    {
        return adr30s[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio1m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL1M)
    {
        return adr1m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio2m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL2M)
    {
        return adr2m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio3m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL3M)
    {
        return adr3m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio5m(unsigned int lead)
{
    if(cal_adr_index>=INTERVAL5M)
    {
        return adr5m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio10m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL10M)
    {
        return adr10m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio20m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL20M)
    {
        return adr20m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio30m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL30M)
    {
        return adr30m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetADRatio60m(unsigned int lead)
{
    if(cal_adr_index-lead>=INTERVAL60M)
    {
        return adr60m[cal_adr_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetMA5m(unsigned int lead)
{
    if(cal_ma_index-lead > INTERVAL5M)
    {
        return ma5m[cal_ma_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetMA25m(unsigned int lead)
{
    if(cal_ma_index-lead > INTERVAL25M)
    {
        return ma25m[cal_ma_index-lead];
    }
    else
    {
        return 0.0;
    }
}

double MDRing::GetMA100m(unsigned int lead)
{
    if(cal_ma_index-lead > INTERVAL100M)
    {
        return ma100m[cal_ma_index-lead];
    }
    else
    {
        return 0.0;
    }
}

void MDRing::SetMinPrice(const std::string& price)
{
    minPrice = std::stod(price);
}

void MDRing::SetMaxPrice(const std::string& price)
{
    maxPrice = std::stod(price);
}

void MDRing::SetTickSize(const std::string& size)
{
    tickSize = std::stod(size);
    if(size == PRECISION0)
        pricePrecision=0;
    else if(size == PRECISION1)
        pricePrecision=1;
    else if(size == PRECISION2)
        pricePrecision=2;
    else if(size == PRECISION3)
        pricePrecision=3;
    else if(size == PRECISION4)
        pricePrecision=4;
    else if(size == PRECISION5)
        pricePrecision=5;
    else if(size == PRECISION6)
        pricePrecision=6;
    else if(size == PRECISION7)
        pricePrecision=7;
    else if(size == PRECISION8)
        pricePrecision=8;
    else
        std::cout << "error" << std::endl;
}

void MDRing::SetMinQty(const std::string& qty)
{
    minQty = std::stod(qty);
}

void MDRing::SetMaxQty(const std::string& qty)
{
    maxQty = std::stod(qty);
}

void MDRing::SetStepSize(const std::string& size)
{
    stepSize = std::stod(size);
    // tips summer@20240318 - 换一种方式比较浮点值
    if(size == PRECISION0 || stepSize == 1.0)
        qtyPrecision=0;
    else if(size == PRECISION1)
        qtyPrecision=1;
    else if(size == PRECISION2)
        qtyPrecision=2;
    else if(size == PRECISION3)
        qtyPrecision=3;
    else if(size == PRECISION4)
        qtyPrecision=4;
    else if(size == PRECISION5)
        qtyPrecision=5;
    else if(size == PRECISION6)
        qtyPrecision=6;
    else if(size == PRECISION7)
        qtyPrecision=7;
    else if(size == PRECISION8)
        qtyPrecision=8;
    else
        std::cout << "error" << std::endl;
}

void MDRing::SetMinNotional(const std::string& notional)
{
    minNotional = std::stod(notional);
}

void MDRing::SetMaxNotional(const std::string& notional)
{
    maxNotional = std::stod(notional);
}

void MDRing::PrintExchangeInfo()
{
    std::cout << minPrice << std::endl;
    std::cout << maxPrice << std::endl;
    std::cout << tickSize << std::endl;
    std::cout << pricePrecision << std::endl;

    std::cout << minQty << std::endl;
    std::cout << maxQty << std::endl;
    std::cout << stepSize << std::endl;
    std::cout << qtyPrecision << std::endl;

    std::cout << minNotional << std::endl;
    std::cout << maxNotional << std::endl;
}
