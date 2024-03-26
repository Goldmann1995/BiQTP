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
    double GetLastPrice();
    
    // 计算因子
    void CalADRatio();
    void CalMovingAverage();
    // 获取因子Index
    int GetADRIndex();
    int GetMAIndex();
    // 获取ADR因子
    double GetADRatio30s(int lead);
    double GetADRatio1m(int lead);
    double GetADRatio2m(int lead);
    double GetADRatio3m(int lead);
    double GetADRatio5m(int lead);
    double GetADRatio10m(int lead);
    double GetADRatio20m(int lead);
    double GetADRatio30m(int lead);
    double GetADRatio60m(int lead);
    // 获取MA因子
    double GetMA5m(int lead);
    double GetMA25m(int lead);
    double GetMA100m(int lead);

    // Buy控制
    void SetBuyIndex(int stidx);
    void ClearBuyIndex(int stidx);
    int GetBuyIndex(int stidx);
    bool EstimateBuyMax(int stidx);
    // Sell控制
    void SetSellIndex(int stidx);
    void ClearSellIndex(int stidx);
    int GetSellIndex(int stidx);
    int GetSellDuration(int stidx);
    // 计算Profit
    double GetProfit(int stidx, double base);

private:
    // 币种名称
    std::string mSymbol;
    // index
    int md_index;
    int cal_ma_index;
    int cal_adr_index;
    int buy_index[ST_SIZE];
    int sell_index[ST_SIZE];
    int cycle_cnt;
    // 最新价
    double price[RING_SIZE];
    // 成交量
    double volume[RING_SIZE];
    // 成交额
    double amount[RING_SIZE];
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
