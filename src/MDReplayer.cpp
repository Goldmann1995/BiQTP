/*
 * File:        MDReplayer.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-25
 * LastEdit:    2024-03-25
 * Description: Replay MarketData from Binance
 */

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>
// 3rd-lib
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
// QTP
#include "Macro.h"
#include "MDRing.h"
#include "MDReplayer.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern MDRing mdring[TOTAL_SYMBOL];
extern std::shared_ptr<spdlog::logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
MDReplayer::MDReplayer()
{
    runTime = std::chrono::steady_clock::now();
    nowTime = std::chrono::steady_clock::now();
}

//##################################################//
//   Destructor
//##################################################//
MDReplayer::~MDReplayer()
{
    // ~
}

//##################################################//
//   线程运行实体
//##################################################//
void MDReplayer::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 0;
    time_to_sleep.tv_nsec = 1000*100;   // 100us

	while( true )
	{
        std::ifstream file("/home/BiQTP/md/BOMEUSDT.csv");
        std::string line;
        if( !file )
        {
            sptrAsyncLogger->error("MDReplayer::Run() Open file failed !");
            return;
        }

        while( std::getline(file, line) )   // 逐行读取
        {
            std::istringstream s(line);
            std::string field;
            std::vector<std::string> fields;

            while( getline(s, field, ',') )   // 使用逗号作为分隔符读取每个字段
                fields.push_back(field);

            // 处理读取的每一行
            std::string symbol("BOMEUSDT");
            int symbol_idx = 0;
            if(symbol2idxUMap.find(symbol) == symbol2idxUMap.end())
                return;
            else
                symbol_idx = symbol2idxUMap[symbol];

            std::string price = fields[4];
            std::string volume = fields[5];
            std::string amount = fields[7];

            //sptrAsyncLogger->info("{},{},{},{}", symbol, price, volume, amount);

            // 推送RingMD
            mdring[symbol_idx].PushMD(std::stod(price), std::stod(volume), std::stod(amount));

            int result = nanosleep(&time_to_sleep, NULL);
            if( result != 0 )
            {
                sptrAsyncLogger->error("MDReplayer::Run() nanosleep() failed !");
            }
        }

        file.close(); // 关闭文件
	}
}
