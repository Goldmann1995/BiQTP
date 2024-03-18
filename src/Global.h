/*
 * File:        Global.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: Global
 */

#pragma once

#include <string>
#include <unordered_map>

#include "RingBuffer.hpp"
#include "UniStruct.h"
#include "MDRing.h"


#define TOTAL_SYMBOL   600


//////////////////////////////////////////////
///           RingBuffer Defs              ///
//////////////////////////////////////////////

// think summer@20221023 - 全局变量空间溢出问题

////////////////////////////////////
///              MD              ///
////////////////////////////////////

std::unordered_map<std::string, int> symbolUMap;
// MarketData
MDRing mdring[TOTAL_SYMBOL];


////////////////////////////////////
///           ZLogger            ///
////////////////////////////////////

// ZLogger
RingBuffer<UniStruct::LogMessage> rb_log_msg;      // 主运行日志
//RingBuffer<UniStruct::LogMessage> rb_log_wdog;   // WatchDog日志


////////////////////////////////////
///           Global             ///
////////////////////////////////////

// QTP初始化标识
//bool bQTPIniting = true;

