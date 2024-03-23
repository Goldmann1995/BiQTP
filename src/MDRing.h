/*
 * File:        MDRing.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-17
 * Description: MarketData & CompuRes Ring
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

    // Interface
    void SetSymbolName(const std::string& symbol);
    const std::string& GetSymbolName();
    int GetMDIndex();
    int GetCalMAIndex();
    int GetCalADRIndex();
    double GetLastPrice();

    // ~
    void SetBuyIndex(int stidx);
    void ClearBuyIndex(int stidx);
    int GetBuyIndex(int stidx);
    bool EstimateBuyMax(int stidx);
    // ~
    void SetSellIndex(int stidx);
    void ClearSellIndex(int stidx);
    int GetSellIndex(int stidx);
    int GetSellDuration(int stidx);
    // ~
    double GetProfit(int stidx, double base);

    // 更新行情
    void PushMD(double last_price);
    // 计算因子
    void CalMovingAverage();
    void CalADRatio();
    // 获取因子
    double GetADRatio30s(int lead);
    double GetADRatio1m(int lead);
    double GetADRatio2m(int lead);
    double GetADRatio3m(int lead);
    double GetADRatio5m(int lead);
    double GetADRatio10m(int lead);
    double GetADRatio20m(int lead);
    double GetADRatio30m(int lead);
    double GetADRatio60m(int lead);
    // 获取因子
    double GetMA5m(int lead);
    double GetMA25m(int lead);
    double GetMA100m(int lead);

private:
    std::string mSymbol;

    int md_index;
    int cal_ma_index;
    int cal_adr_index;
    int buy_index[ST_SIZE];
    int sell_index[ST_SIZE];
    int cycle_cnt;

    // 5s最新价
    double price[RING_SIZE];
    // ~
    double mt5m;
    double ma5m[RING_SIZE];
    double mt25m;
    double ma25m[RING_SIZE];
    double mt100m;
    double ma100m[RING_SIZE];
    // ~
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
