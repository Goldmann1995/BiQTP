/*
 * File:        MDRing.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-24
 * Description: Ring of MarketData & CompuRes 
 */

#pragma once

#include "Macro.h"


//############################################################//
//   MDRing Class
//############################################################//
class MDRing
{
public:
    MDRing();
    ~MDRing();

    // 设置币种
    void SetSymbolName(const std::string& symbol);
    const std::string& GetSymbolName();

    // 更新行情
    void PushMD(double last_price, double last_volume, double last_amount);
    int GetMDIndex();
    int GetCycleCnt();
    double GetLastPrice();
    double GetIndexPrice(int index);
    
    // ADR因子
    void CalADRatio();
    int GetADRIndex();
    double GetADRatio30s(int lead);
    double GetADRatio1m(int lead);
    double GetADRatio2m(int lead);
    double GetADRatio3m(int lead);
    double GetADRatio5m(int lead);
    double GetADRatio10m(int lead);
    double GetADRatio20m(int lead);
    double GetADRatio30m(int lead);
    double GetADRatio60m(int lead);

    // MA因子
    void CalMovingAverage();
    int GetMAIndex();
    double GetMA5m(int lead);
    double GetMA25m(int lead);
    double GetMA100m(int lead);

private:
    // 币种名称
    std::string mSymbol;
    
    // index
    int cycle_cnt;
    int md_index;
    int cal_ma_index;
    int cal_adr_index;
    
    double price[RING_SIZE];    // 最新价
    double volume[RING_SIZE];   // 成交量
    double amount[RING_SIZE];   // 成交额

    // MovingAverage
    double mt5m;
    double ma5m[RING_SIZE];
    double mt25m;
    double ma25m[RING_SIZE];
    double mt100m;
    double ma100m[RING_SIZE];
    
    // ADRatio
    double adr30s[RING_SIZE];
    double adr1m[RING_SIZE];
    double adr2m[RING_SIZE];
    double adr3m[RING_SIZE];
    double adr5m[RING_SIZE];
    double adr10m[RING_SIZE];
    double adr20m[RING_SIZE];
    double adr30m[RING_SIZE];
    double adr60m[RING_SIZE];
};

//############################################################//
