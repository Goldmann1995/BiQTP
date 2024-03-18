/*
 * File:        MDRing.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-17
 * Description: MarketData & CompuRes Ring
 */

#pragma once

#define RING_SIZE   (24*60*12)
#define ST_SIZE     100

#define PRECISION0  "1.00000000"
#define PRECISION1  "0.10000000"
#define PRECISION2  "0.01000000"
#define PRECISION3  "0.00100000"
#define PRECISION4  "0.00010000"
#define PRECISION5  "0.00001000"
#define PRECISION6  "0.00000100"
#define PRECISION7  "0.00000010"
#define PRECISION8  "0.00000001"

#define GAP30S  (6)
#define GAP1M  (12)
#define GAP2M  (12*2)
#define GAP3M  (12*3)
#define GAP5M  (12*5)
#define GAP10M  (12*10)
#define GAP20M  (12*20)
#define GAP25M  (12*25)
#define GAP30M  (12*30)
#define GAP60M  (12*60)
#define GAP100M  (12*100)


//############################################################//
//   MDRing Class
//############################################################//
class MDRing
{
public:
    MDRing();
    //MDRing(const std::string& symbol);
    ~MDRing();

    void SetSymbolName(const std::string& symbol);
    const std::string& GetSymbolName();
    void SetBuyIndex(int stidx);
    void ClearBuyIndex(int stidx);
    int GetBuyIndex(int stidx);
    void SetSellIndex(int stidx);
    int GetSellIndex(int stidx);
    int GetSellGap(int stidx);
    void ClearSellIndex(int stidx);
    double GetProfit(int stidx, double base);

    // 更新行情
    void PushMD(double last_price);
    // 计算因子
    void CalMovingAverage();
    void CalADRatio();
    // 获取因子
    double GetADRatio30s(unsigned int lead);
    double GetADRatio1m(unsigned int lead);
    double GetADRatio2m(unsigned int lead);
    double GetADRatio3m(unsigned int lead);
    double GetADRatio5m(unsigned int lead);
    double GetADRatio10m(unsigned int lead);
    double GetADRatio20m(unsigned int lead);
    double GetADRatio30m(unsigned int lead);
    double GetADRatio60m(unsigned int lead);

    // 初始化Binance交易参数
    void SetMinPrice(const std::string& price);
    void SetMaxPrice(const std::string& price);
    void SetTickSize(const std::string& size);
    void SetMinQty(const std::string& qty);
    void SetMaxQty(const std::string& qty);
    void SetStepSize(const std::string& size);
    void SetMinNotional(const std::string& notional);
    void SetMaxNotional(const std::string& notional);
    void PrintExchangeInfo();

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

private:
    double minPrice;
    double maxPrice;
    double tickSize;
    unsigned int pricePrecision;

    double minQty;
    double maxQty;
    double stepSize;
    unsigned int qtyPrecision;

    double minNotional;
    double maxNotional;
};

//##################################################//
