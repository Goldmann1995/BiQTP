/*
 * File:        Strategy.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: Strategy of Quanter
 */

#pragma once


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

private:
    int mStrategyID;

    double mCapital;
    double mTotalProfit;
    double mTotalCommission;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;

    // 注册友元派生策略
    friend class AdvancedSLR1;
    friend class AdvancedSLR2;
    friend class MACross1;
    friend class MACross2;
    friend class GridTrader;
};

//############################################################//

class AdvancedSLR1 : public Strategy
{
public:
    AdvancedSLR1(int id, double captical);
    ~AdvancedSLR1();

    void Run();
};

//############################################################//

class AdvancedSLR2 : public Strategy
{
public:
    AdvancedSLR2(int id, double captical);
    ~AdvancedSLR2();

    void Run();
};

//############################################################//

class MACross1 : public Strategy
{
public:
    MACross1(int id, double captical);
    ~MACross1();

    void Run();
};

//############################################################//

class MACross2 : public Strategy
{
public:
    MACross2(int id, double captical);
    ~MACross2();

    void Run();
};

//############################################################//

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

//############################################################//
