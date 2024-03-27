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
    MDReplayer(const std::string& path);
    ~MDReplayer();

    // 线程运行实体
    void Run();

    // 载入历史行情数据
    void LoadHistoryMD(const std::string& date);

private:
    std::string mdPath;
    std::vector<std::vector<std::string>> fields[TOTAL_SYMBOL];

    std::chrono::time_point<std::chrono::steady_clock> runTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
};

//############################################################//
