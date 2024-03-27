/*
 * File:        MDReplayer.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-25
 * LastEdit:    2024-03-25
 * Description: Replay MarketData from Binance
 */

#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
// STL
#include <vector>
#include <unordered_map>
// 3rd-lib
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// QTP
#include "Macro.h"
#include "MDRing.h"
#include "MDReplayer.h"

// Extern
extern std::unordered_map<std::string, int> symbol2idxUMap;
extern MDRing mdring[TOTAL_SYMBOL];
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;


//##################################################//
//   Constructor
//##################################################//
MDReplayer::MDReplayer(const std::string& path)
{
    mdPath = path;
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
    /*struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 0;
    time_to_sleep.tv_nsec = 1000;   // 1us*/

	for(int i=0; i<RING_SIZE; i++)
	{
        // 遍历SymbolMap
        for(const auto& symbol_iter:symbol2idxUMap)
        {
            if(fields[symbol_iter.second].empty())
                continue;
            
            std::string price = fields[symbol_iter.second][i][0];
            std::string volume = fields[symbol_iter.second][i][1];
            std::string amount = fields[symbol_iter.second][i][2];

            // 推送RingMD
            mdring[symbol_iter.second].PushMD(std::stod(price), std::stod(volume), std::stod(amount));

            /*int result = nanosleep(&time_to_sleep, NULL);
            if( result != 0 )
            {
                sptrAsyncLogger->error("MDReplayer::Run() nanosleep() failed !");
            }*/
        }
	}
}

//##################################################//
//   载入历史行情数据
//##################################################//
void MDReplayer::LoadHistoryMD(const std::string& date)
{
    std::ifstream md_file;
    std::string md_path;
    std::string md_line;

    // 遍历SymbolMap
    for(const auto& symbol_iter:symbol2idxUMap)
    {
        md_path = mdPath + date + "/" + symbol_iter.first + "-1s-" + date + ".csv";
        md_file.open(md_path);
        if( !md_file )
        {
            sptrAsyncLogger->error("MDReplayer::LoadHistoryMD() {} Open file failed !", symbol_iter.first);
            continue;
        }

        // 逐行读取
        while( std::getline(md_file, md_line) )
        {
            std::istringstream iss(md_line);
            std::vector<std::string> fieldvec = {};
            std::string field("");
            int column = 0;
            
            // 使用逗号作为分隔符读取每个字段
            while( getline(iss, field, ',') )
            {
                //@Binance// 0=open_time      // K线开盘时间
                //@Binance// 1=open           // 开盘价
                //@Binance// 2=high           // 最高价
                //@Binance// 3=low            // 最低价
                //@Binance// 4=close          // 收盘价
                //@Binance// 5=volume         // 成交量
                //@Binance// 6=close_time     // K线收盘时间
                //@Binance// 7=quote_volume   // 报价币成交量
                //@Binance// 8=count          // 成单数
                //@Binance// 9=taker_buy_volume         // 买入的基础币数量
                //@Binance// 10=taker_buy_quote_volume  // 买入的报价币数量
                //@Binance// 11=ignore                  // 忽略

                if( column==4 || column==5 || column==7 )
                    fieldvec.push_back(field);
                column++;
            }
            fields[symbol_iter.second].push_back(fieldvec);
        }

        sptrAsyncLogger->info("MDReplayer::LoadHistoryMD() {} {}", symbol_iter.first, fields[symbol_iter.second].size());
        // 关闭文件
        md_file.close();
    }   
}
