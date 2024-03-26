/*
 * File:        Strategy.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: Strategy of Quanter
 */

#pragma once


/*** 策略控制block ***/
struct StBlock
{
    int pointType;   // 0=未交易 // 1=买入 // 2=卖出
    int buyMdIndex;
    int sellMdIndex;
};


//############################################################//
//   Strategy Class
//############################################################//
class Strategy
{
public:
    Strategy();
    ~Strategy();

    // 策略逻辑
    virtual void Run() = 0;

    double GetPositiveRate();

protected:
    int mStrategyID;

    int mPositiveCnt;
    int mNegativeCnt;
    int mOrderCnt;

    double mTotalProfitRate;
    double mTotalCommissionRate;

    StBlock mStPoint[TOTAL_SYMBOL];
};

//############################################################//

class AdvancedSLR1 : public Strategy
{
public:
    AdvancedSLR1(int id);
    ~AdvancedSLR1();

    void Run();
};

//############################################################//

class AdvancedSLR2 : public Strategy
{
public:
    AdvancedSLR2(int id);
    ~AdvancedSLR2();

    void Run();
};

//############################################################//

#if 0
class MACross1 : public Strategy
{
public:
    MACross1(int id);
    ~MACross1();

    void Run();
};
#endif

//############################################################//

#if 0
class MACross2 : public Strategy
{
public:
    MACross2(int id);
    ~MACross2();

    void Run();
};
#endif

//############################################################//

#if 0
class GridTrader : public Strategy
{
public:
    GridTrader(int id);
    ~GridTrader();

    void Run();

private:
    double total_pos;
    double total_captical;
    double total_commission;

    double last_price;
    std::chrono::time_point<std::chrono::steady_clock> checkTime;
};
#endif

//############################################################//
