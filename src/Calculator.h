/*
 * File:        Calculator.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: Calculator for MarketData
 */

#pragma once

#include <ThreadBase.h>


//############################################################//
//   Calculator Class
//############################################################//
class Calculator: public ThreadBase
{
public:
    Calculator();
    ~Calculator();

    // 线程运行实体
    void Run();

    // ~
    void CalculateLastPrice();

private:
    std::chrono::time_point<std::chrono::steady_clock> calTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
};

//##################################################//
