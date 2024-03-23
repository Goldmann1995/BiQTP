/*
 * File:        WatchDog.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-21
 * LastEdit:    2024-03-23
 * Description: Watch Dog for QTP
 */

#pragma once

#include <ThreadBase.h>


//############################################################//
//   WatchDog Class
//############################################################//
class WatchDog: public ThreadBase
{
public:
    WatchDog();
    ~WatchDog();

    // 线程运行实体
    void Run();

private:
    std::chrono::time_point<std::chrono::steady_clock> runTime;
    std::chrono::time_point<std::chrono::steady_clock> nowTime;
};

//############################################################//
