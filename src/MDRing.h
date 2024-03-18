/*
 * File:        MDRing.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-17
 * LastEdit:    2024-03-17
 * Description: MarketData & CompuRes Ring
 */

#pragma once

#define RING_SIZE   (24*60*12)

#define PRECISION0  "1.00000000"
#define PRECISION1  "0.10000000"
#define PRECISION2  "0.01000000"
#define PRECISION3  "0.00100000"
#define PRECISION4  "0.00010000"
#define PRECISION5  "0.00001000"
#define PRECISION6  "0.00000100"
#define PRECISION7  "0.00000010"
#define PRECISION8  "0.00000001"


//############################################################//
//   MDRing Class
//############################################################//
class MDRing
{
public:
    MDRing();
    MDRing(const std::string& symbol);
    ~MDRing();

    void SetSymbolName(const std::string& symbol);
    const std::string& GetSymbolName();
    void PushMD(double last_price);
    void CalMovingAverage();
    void CalADRatio();

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
    int cal_index;
    int buy_index;
    int sell_index;
    bool flag[RING_SIZE];
    double price[RING_SIZE];
    double ma5m[RING_SIZE];
    double ma25m[RING_SIZE];
    double ma100m[RING_SIZE];
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
