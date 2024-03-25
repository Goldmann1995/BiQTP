/*
 * File:        MDReplayer.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-25
 * LastEdit:    2024-03-25
 * Description: Replay MarketData from Binance
 */

#pragma once

#include <ThreadBase.h>


//############################################################//
//   MDReplayer Class
//############################################################//
class MDReplayer: public ThreadBase
{
public:
    MDReplayer();
    ~MDReplayer();

    // 线程运行实体
    void Run();

private:
    std::chrono::time_point<std::chrono::steady_clock> runTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
};

//############################################################//
