/*
 * File:        StrategyBOX.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: BOX of Strategy
 */

#pragma once

#include <ThreadBase.h>
#include "Strategy.h"


//############################################################//
//   StrategyBOX Class
//############################################################//
class StrategyBOX: public ThreadBase
{
public:
    StrategyBOX();
    ~StrategyBOX();

    // 线程运行实体
    void Run();

    // Interface
    void EntrustStrategy(Strategy *strategy);

private:
    double mTotalProfit;
    double mTotalCommission;

    std::chrono::time_point<std::chrono::steady_clock> runTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;

    // 策略集合
    vector<Strategy*> strategyVec;
};

//############################################################//
