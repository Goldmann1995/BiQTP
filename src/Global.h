/*
 * File:        Global.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-24
 * Description: Global Data Structure
 */

#pragma once

// STL
#include <string>
#include <unordered_map>
// 
#include "RingBuffer.hpp"
//#include "UniStruct.h"
#include "BiFilter.h"
#include "MDRing.h"
#include "Macro.h"


//////////////////////////////////////////////
///                Global                  ///
//////////////////////////////////////////////

// QTP初始化标识
//bool bQTPIniting = true;

////////////////////////////////////
///              MD              ///
////////////////////////////////////

std::unordered_map<std::string, int> symbol2idxUMap;
Binance::SymbolFilter symbolFilterArr[TOTAL_SYMBOL];
Binance::miniTicker miniTickerArr[TOTAL_SYMBOL];
MDRing mdring[TOTAL_SYMBOL];


////////////////////////////////////
///           ZLogger            ///
////////////////////////////////////

// ZLogger
//RingBuffer<UniStruct::LogMessage> rb_log_msg;      // 主运行日志
//RingBuffer<UniStruct::LogMessage> rb_log_wdog;     // WatchDog日志


//////////////////////////////////////////////
