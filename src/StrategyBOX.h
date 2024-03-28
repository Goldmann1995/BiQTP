/*
 * File:        StrategyBOX.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: BOX of Strategy
 */

#pragma once

#include <ThreadBase.h>


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
    // 注册策略
    void EntrustStrategy(Strategy *strategy);
    // 注销策略
    void DetrustStrategy(int strategy_id);

private:
    double totalProfit;
    double totalCommission;

    std::chrono::time_point<std::chrono::steady_clock> runTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;

    // 策略集合
    std::vector<Strategy*> strategyVec;
};

//############################################################//
