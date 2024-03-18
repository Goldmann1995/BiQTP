/*
 * File:        MDRing.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: MarketData & CompuRes Ring
 */

#include <iostream>
#include <string>
#include <log/log.h>

#include "MDRing.h"


//##################################################//
//   Constructor
//##################################################//
MDRing::MDRing()
{
    md_index = 0;
    cal_ma_index = 0;
    cal_adr_index = 0;
    buy_index = 0;
    sell_index = 0;
    cycle_cnt = 0;
}

//##################################################//
//   Destructor
//##################################################//
MDRing::~MDRing()
{
    //
}

void MDRing::SetSymbolName(const std::string& symbol)
{
    mSymbol = symbol;
}

const std::string& MDRing::GetSymbolName()
{
    return mSymbol;
}

void MDRing::PushMD(double last_price)
{
    int index = md_index+1%RING_SIZE;
    price[index] = last_price;
    if(md_index+1==RING_SIZE)
        cycle_cnt++;
    md_index=index;

    LOG_INFO("MDRing::PushMD() Symbol: %s Index: %d Price: %f",  mSymbol.c_str(), index, last_price);
}

void MDRing::CalMovingAverage()
{
    if(cal_ma_index==md_index)
        return;
    else   // cal_ma_index
    {
        int next_index = cal_ma_index+1%RING_SIZE;
        mt5m += price[next_index];
        mt25m += price[next_index];
        mt100m += price[next_index];

        if(cycle_cnt == 0)
        {
            if(next_index>=GAP5M)
            {
                mt5m -= price[next_index-GAP5M];
                ma5m[next_index] = mt5m/GAP5M;

                LOG_INFO("MDRing::CalMovingAverage() Symbol: %s MA5: %f",  mSymbol.c_str(), ma5m[next_index]);
            }

            if(next_index>=GAP25M)
            {
                mt25m -= price[next_index-GAP25M];
                ma25m[next_index] = mt25m/GAP25M;
            }

            if(next_index>=GAP100M)
            {
                mt100m -= price[next_index-GAP100M];
                ma100m[next_index] = mt100m/GAP100M;
            }
        }
        else
        {
            if(next_index>=GAP5M)
            {
                mt5m -= price[next_index-GAP5M];
                ma5m[next_index] = mt5m/GAP5M;
            }
            else
            {
                mt5m -= price[RING_SIZE-GAP5M-next_index];
                ma5m[next_index] = mt5m/GAP5M; 
            }

            if(next_index>=GAP25M)
            {
                mt25m -= price[next_index-GAP25M];
                ma25m[next_index] = mt25m/GAP25M;
            }
            else
            {
                mt25m -= price[RING_SIZE-GAP25M-next_index];
                ma25m[next_index] = mt25m/GAP25M; 
            }

            if(next_index>=GAP100M)
            {
                mt100m -= price[next_index-GAP100M];
                ma100m[next_index] = mt100m/GAP100M;
            }
            else
            {
                mt100m -= price[RING_SIZE-GAP100M-next_index];
                ma100m[next_index] = mt100m/GAP100M; 
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
        int next_index = cal_adr_index+1%RING_SIZE;

        if(cycle_cnt == 0)
        {
            if(next_index>=GAP30S)
            {
                adr30s[next_index] = (price[next_index]-price[next_index-GAP30S])/price[next_index-GAP30S];

                LOG_INFO("MDRing::CalMovingAverage() Symbol: %s ADR30S: %f",  mSymbol.c_str(), adr30s[next_index]);
            }

            if(next_index>=GAP1M)
            {
                adr1m[next_index] = (price[next_index]-price[next_index-GAP1M])/price[next_index-GAP1M];
            }

            if(next_index>=GAP2M)
            {
                adr2m[next_index] = (price[next_index]-price[next_index-GAP2M])/price[next_index-GAP2M];
            }

            if(next_index>=GAP3M)
            {
                adr3m[next_index] = (price[next_index]-price[next_index-GAP3M])/price[next_index-GAP3M];
            }

            if(next_index>=GAP5M)
            {
                adr5m[next_index] = (price[next_index]-price[next_index-GAP5M])/price[next_index-GAP5M];
            }

            if(next_index>=GAP10M)
            {
                adr10m[next_index] = (price[next_index]-price[next_index-GAP10M])/price[next_index-GAP10M];
            }

            if(next_index>=GAP20M)
            {
                adr20m[next_index] = (price[next_index]-price[next_index-GAP20M])/price[next_index-GAP20M];
            }

            if(next_index>=GAP30M)
            {
                adr30m[next_index] = (price[next_index]-price[next_index-GAP30M])/price[next_index-GAP30M];
            }

            if(next_index>=GAP60M)
            {
                adr60m[next_index] = (price[next_index]-price[next_index-GAP60M])/price[next_index-GAP60M];
            }
        }
        else
        {
            if(next_index>=GAP30S)
            {
                adr30s[next_index] = (price[next_index]-price[next_index-GAP30S])/price[next_index-GAP30S];
            }
            else
            {
                adr30s[next_index] = (price[next_index]-price[RING_SIZE-GAP30S+next_index])/price[RING_SIZE-GAP30S+next_index];
            }

            if(next_index>=GAP1M)
            {
                adr1m[next_index] = (price[next_index]-price[next_index-GAP1M])/price[next_index-GAP1M];
            }
            else
            {
                adr1m[next_index] = (price[next_index]-price[RING_SIZE-GAP1M+next_index])/price[RING_SIZE-GAP1M+next_index];
            }

            if(next_index>=GAP2M)
            {
                adr2m[next_index] = (price[next_index]-price[next_index-GAP2M])/price[next_index-GAP2M];
            }
            else
            {
                adr2m[next_index] = (price[next_index]-price[RING_SIZE-GAP2M+next_index])/price[RING_SIZE-GAP2M+next_index];
            }

            if(next_index>=GAP3M)
            {
                adr3m[next_index] = (price[next_index]-price[next_index-GAP3M])/price[next_index-GAP3M];
            }
            else
            {
                adr3m[next_index] = (price[next_index]-price[RING_SIZE-GAP3M+next_index])/price[RING_SIZE-GAP3M+next_index];
            }

            if(next_index>=GAP5M)
            {
                adr5m[next_index] = (price[next_index]-price[next_index-GAP5M])/price[next_index-GAP5M];
            }
            else
            {
                adr5m[next_index] = (price[next_index]-price[RING_SIZE-GAP5M+next_index])/price[RING_SIZE-GAP5M+next_index];
            }

            if(next_index>=GAP10M)
            {
                adr10m[next_index] = (price[next_index]-price[next_index-GAP10M])/price[next_index-GAP10M];
            }
            else
            {
                adr10m[next_index] = (price[next_index]-price[RING_SIZE-GAP10M+next_index])/price[RING_SIZE-GAP10M+next_index];
            }

            if(next_index>=GAP20M)
            {
                adr20m[next_index] = (price[next_index]-price[next_index-GAP20M])/price[next_index-GAP20M];
            }
            else
            {
                adr20m[next_index] = (price[next_index]-price[RING_SIZE-GAP20M+next_index])/price[RING_SIZE-GAP20M+next_index];
            }

            if(next_index>=GAP30M)
            {
                adr30m[next_index] = (price[next_index]-price[next_index-GAP30M])/price[next_index-GAP30M];
            }
            else
            {
                adr30m[next_index] = (price[next_index]-price[RING_SIZE-GAP30M+next_index])/price[RING_SIZE-GAP30M+next_index];
            }

            if(next_index>=GAP60M)
            {
                adr60m[next_index] = (price[next_index]-price[next_index-GAP60M])/price[next_index-GAP60M];
            }
            else
            {
                adr60m[next_index] = (price[next_index]-price[RING_SIZE-GAP60M+next_index])/price[RING_SIZE-GAP60M+next_index];
            }
        }

        cal_adr_index = next_index;
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
