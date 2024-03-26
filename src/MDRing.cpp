/*
 * File:        MDRing.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-24
 * Description: Ring of MarketData & CompuRes 
 */

#include <iostream>
#include <string>
#include <algorithm>

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>

#include "MDRing.h"

extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
MDRing::MDRing()
{
    mSymbol = "";

    cycle_cnt = 0;
    md_index = -1;
    cal_ma_index = -1;
    cal_adr_index = -1;

    mt5m = 0.0;
    mt25m = 0.0;
    mt100m = 0.0;
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

//##################################################//
//   更新行情
//##################################################//
void MDRing::PushMD(double last_price, double last_volume, double last_amount)
{
    int index = (md_index+1)%RING_SIZE;
    price[index] = last_price;
    volume[index] = last_volume;
    amount[index] = last_amount;
    if((md_index+1)==RING_SIZE)
        cycle_cnt++;
    md_index=index;
}

//##################################################//
//   获取行情Index
//##################################################//
int MDRing::GetMDIndex()
{
    return md_index;
}

//##################################################//
//   获取轮数Cnt
//##################################################//
int MDRing::GetCycleCnt()
{
    return cycle_cnt;
}

//##################################################//
//   获取最新行情
//##################################################//
double MDRing::GetLastPrice()
{
    if( md_index>=0 )
        return price[md_index];
    else
        return 0.0;
}

//##################################################//
//   获取指定Index行情
//##################################################//
double MDRing::GetIndexPrice(int index)
{
    if( 0<=index && index<RING_SIZE )
        return price[index];
    else
        return 0.0;
}

//##################################################//
//   计算涨跌幅度
//##################################################//
void MDRing::CalADRatio()
{
    if(cal_adr_index == md_index)
        return;
    else   // cal_adr_index
    {
        int next_index = (cal_adr_index+1)%RING_SIZE;

        if(cycle_cnt == 0)
        {
            if(next_index >= INTERVAL30S)
            {
                adr30s[next_index] = (price[next_index]-price[next_index-INTERVAL30S])/price[next_index-INTERVAL30S];
            }

            if(next_index >= INTERVAL1M)
            {
                adr1m[next_index] = (price[next_index]-price[next_index-INTERVAL1M])/price[next_index-INTERVAL1M];
            }

            if(next_index >= INTERVAL2M)
            {
                adr2m[next_index] = (price[next_index]-price[next_index-INTERVAL2M])/price[next_index-INTERVAL2M];
            }

            if(next_index >= INTERVAL3M)
            {
                adr3m[next_index] = (price[next_index]-price[next_index-INTERVAL3M])/price[next_index-INTERVAL3M];
            }

            if(next_index >= INTERVAL5M)
            {
                adr5m[next_index] = (price[next_index]-price[next_index-INTERVAL5M])/price[next_index-INTERVAL5M];
            }

            if(next_index >= INTERVAL10M)
            {
                adr10m[next_index] = (price[next_index]-price[next_index-INTERVAL10M])/price[next_index-INTERVAL10M];
            }

            if(next_index >= INTERVAL20M)
            {
                adr20m[next_index] = (price[next_index]-price[next_index-INTERVAL20M])/price[next_index-INTERVAL20M];
            }

            if(next_index >= INTERVAL30M)
            {
                adr30m[next_index] = (price[next_index]-price[next_index-INTERVAL30M])/price[next_index-INTERVAL30M];
            }

            if(next_index >= INTERVAL60M)
            {
                adr60m[next_index] = (price[next_index]-price[next_index-INTERVAL60M])/price[next_index-INTERVAL60M];
            }
        }
        else
        {
            if(next_index >= INTERVAL30S)
            {
                adr30s[next_index] = (price[next_index]-price[next_index-INTERVAL30S])/price[next_index-INTERVAL30S];
            }
            else
            {
                adr30s[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL30S+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL30S+next_index];
            }

            if(next_index >= INTERVAL1M)
            {
                adr1m[next_index] = (price[next_index]-price[next_index-INTERVAL1M])/price[next_index-INTERVAL1M];
            }
            else
            {
                adr1m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL1M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL1M+next_index];
            }

            if(next_index >= INTERVAL2M)
            {
                adr2m[next_index] = (price[next_index]-price[next_index-INTERVAL2M])/price[next_index-INTERVAL2M];
            }
            else
            {
                adr2m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL2M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL2M+next_index];
            }

            if(next_index >= INTERVAL3M)
            {
                adr3m[next_index] = (price[next_index]-price[next_index-INTERVAL3M])/price[next_index-INTERVAL3M];
            }
            else
            {
                adr3m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL3M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL3M+next_index];
            }

            if(next_index >= INTERVAL5M)
            {
                adr5m[next_index] = (price[next_index]-price[next_index-INTERVAL5M])/price[next_index-INTERVAL5M];
            }
            else
            {
                adr5m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL5M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL5M+next_index];
            }

            if(next_index >= INTERVAL10M)
            {
                adr10m[next_index] = (price[next_index]-price[next_index-INTERVAL10M])/price[next_index-INTERVAL10M];
            }
            else
            {
                adr10m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL10M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL10M+next_index];
            }

            if(next_index >= INTERVAL20M)
            {
                adr20m[next_index] = (price[next_index]-price[next_index-INTERVAL20M])/price[next_index-INTERVAL20M];
            }
            else
            {
                adr20m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL20M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL20M+next_index];
            }

            if(next_index >= INTERVAL30M)
            {
                adr30m[next_index] = (price[next_index]-price[next_index-INTERVAL30M])/price[next_index-INTERVAL30M];
            }
            else
            {
                adr30m[next_index] = (price[next_index]-price[(RING_SIZE-INTERVAL30M+next_index)%RING_SIZE])/price[RING_SIZE-INTERVAL30M+next_index];
            }

            if(next_index >= INTERVAL60M)
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

int MDRing::GetADRIndex()
{
    return cal_adr_index;
}

double MDRing::GetADRatio30s(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL30S)
            return adr30s[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr30s[cal_adr_index-lead];
        else
            return adr30s[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio1m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL1M)
            return adr1m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr1m[cal_adr_index-lead];
        else
            return adr1m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio2m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL2M)
            return adr2m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr2m[cal_adr_index-lead];
        else
            return adr2m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio3m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL3M)
            return adr3m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr3m[cal_adr_index-lead];
        else
            return adr3m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio5m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index>=INTERVAL5M)
            return adr5m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr5m[cal_adr_index-lead];
        else
            return adr5m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio10m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL10M)
            return adr10m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr10m[cal_adr_index-lead];
        else
            return adr10m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio20m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL20M)
            return adr20m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr20m[cal_adr_index-lead];
        else
            return adr20m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio30m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL30M)
            return adr30m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr30m[cal_adr_index-lead];
        else
            return adr30m[ST_SIZE+cal_adr_index-lead];
    }
}

double MDRing::GetADRatio60m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_adr_index-lead>=INTERVAL60M)
            return adr60m[cal_adr_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return adr60m[cal_adr_index-lead];
        else
            return adr60m[ST_SIZE+cal_adr_index-lead];
    }
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
        int next_index = (cal_ma_index+1)%RING_SIZE;
        mt5m += price[next_index];
        mt25m += price[next_index];
        mt100m += price[next_index];

        if(cycle_cnt == 0)
        {
            if(next_index >= INTERVAL5M)
            {
                mt5m -= price[next_index-INTERVAL5M];
                ma5m[next_index] = mt5m/INTERVAL5M;
            }

            if(next_index >= INTERVAL25M)
            {
                mt25m -= price[next_index-INTERVAL25M];
                ma25m[next_index] = mt25m/INTERVAL25M;
            }

            if(next_index >= INTERVAL100M)
            {
                mt100m -= price[next_index-INTERVAL100M];
                ma100m[next_index] = mt100m/INTERVAL100M;
            }
        }
        else
        {
            if(next_index >= INTERVAL5M)
            {
                mt5m -= price[next_index-INTERVAL5M];
                ma5m[next_index] = mt5m/INTERVAL5M;
            }
            else
            {
                mt5m -= price[RING_SIZE-INTERVAL5M+next_index];
                ma5m[next_index] = mt5m/INTERVAL5M; 
            }

            if(next_index >= INTERVAL25M)
            {
                mt25m -= price[next_index-INTERVAL25M];
                ma25m[next_index] = mt25m/INTERVAL25M;
            }
            else
            {
                mt25m -= price[RING_SIZE-INTERVAL25M+next_index];
                ma25m[next_index] = mt25m/INTERVAL25M; 
            }

            if(next_index >= INTERVAL100M)
            {
                mt100m -= price[next_index-INTERVAL100M];
                ma100m[next_index] = mt100m/INTERVAL100M;
            }
            else
            {
                mt100m -= price[RING_SIZE-INTERVAL100M+next_index];
                ma100m[next_index] = mt100m/INTERVAL100M;
            }
        }

        cal_ma_index = next_index;
    }
}

int MDRing::GetMAIndex()
{
    return cal_ma_index;
}

double MDRing::GetMA5m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_ma_index-lead >= INTERVAL5M)
            return ma5m[cal_ma_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return ma5m[cal_ma_index-lead];
        else
            return ma5m[ST_SIZE+cal_ma_index-lead];
    }
}

double MDRing::GetMA25m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_ma_index-lead > INTERVAL25M)
            return ma25m[cal_ma_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return ma25m[cal_ma_index-lead];
        else
            return ma25m[ST_SIZE+cal_ma_index-lead];
    }
}

double MDRing::GetMA100m(int lead)
{
    if( cycle_cnt==0 )
    {
        if(cal_ma_index-lead > INTERVAL100M)
            return ma100m[cal_ma_index-lead];
        else
            return 0.0;
    }
    else
    {
        if(cal_adr_index-lead>=0)
            return ma100m[cal_ma_index-lead];
        else
            return ma100m[ST_SIZE+cal_ma_index-lead];
    }
}
