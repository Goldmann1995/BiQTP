/*
 * File:        StrategyBOX.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-19
 * LastEdit:    2024-03-19
 * Description: Strategy of Quanter
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

    // Strategy
    void AdvancedSLR1();
    void AdvancedSLR2();

private:
    double profit_ASLR1 = 0.0;
    double profit_ASLR2 = 0.0;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> runTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
};

//############################################################//
