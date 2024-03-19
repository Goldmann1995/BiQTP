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
