/*
 * File:        OrderManager.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-28
 * LastEdit:    2024-03-28
 * Description: Order Manager
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <chrono>
// STL
#include <queue>
#include <vector>
// 3rd-lib
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>
// QTP
#include "Macro.h"
#include "BiDef.h"
#include "BiFilter.h"
#include "BiTrader.h"
#include "OrderManager.h"

// Extern
extern std::shared_ptr<spdlog::async_logger> sptrAsyncLogger;
extern std::unique_ptr<BiTrader> uptrBiTrader;


//##################################################//
//   Constructor
//##################################################//
OrderManager::OrderManager()
{
    totalCaptical = 1000.0;
    unitCaptical = 100.0;
    orderCnt = 0;

    for(int i=0; i<ST_SIZE; i++)
        strategyOrders[i].clear();
}

//##################################################//
//   Destructor
//##################################################//
OrderManager::~OrderManager()
{
    //
}

//##################################################//
//   线程运行实体
//##################################################//
void OrderManager::Run()
{
    struct timespec time_to_sleep;
    time_to_sleep.tv_sec  = 0;
    time_to_sleep.tv_nsec = 1000*100;   // 100us

	while( true )
	{
        std::lock_guard<std::mutex> lock(signalMtx);
        if( !signalQueue.empty() )
        {
            SignalBlock sig_block = signalQueue.front();
            // 买入信号
            if(sig_block.side==1)
            {
                if(unitCaptical*orderCnt<totalCaptical)
                    StrategyBuySignal(sig_block.strategyId, sig_block.symbol, sig_block.price);
                else
                    sptrAsyncLogger->error("OrderManager::Run() no available Captical !");
            }
            // 卖出信号
            else if(sig_block.side==2)
            {
                StrategySellSignal(sig_block.strategyId, sig_block.symbol, sig_block.price);
                orderCnt--;
            }
            else
                sptrAsyncLogger->error("OrderManager::Run() signalBlock Error !");
            
            // Pop操作
            signalQueue.pop();
        }

        int result = nanosleep(&time_to_sleep, NULL);
        if( result != 0 )
            sptrAsyncLogger->error("OrderManager::Run() nanosleep() Failed !");
	}
}

//##################################################//
//   策略推送信号
//##################################################//
void OrderManager::PushSignal(int strategy_id, std::string symbol, int side, double price)
{
    SignalBlock sig_block;
    sig_block.strategyId = strategy_id;
    sig_block.symbol = symbol;
    sig_block.side = side;
    sig_block.price = price;

    // Push操作
    std::lock_guard<std::mutex> lock(signalMtx);
    signalQueue.push(sig_block);

    //@STL// Queue本身不是线程安全的
    //@STL// 如果只是一个线程进行Push操作
    //@STL// 另一个线程进行Pop操作则安全
}

//##################################################//
//   策略买入信号
//##################################################//
void OrderManager::StrategyBuySignal(int strategy_id, std::string symbol, double price)
{
    // 计算报单qty
    double exe_price = 0.0;
    double exe_qty = 0.0;
    double comm_qty = 0.0;
    double qty = unitCaptical/price;

    // 币安报单
    bool ret = uptrBiTrader->InsertOrder(symbol, Binance::OrderSide::BUY, 0.0, qty, \
                                        Binance::OrderType::MARKET, Binance::TimeInForce::GTC, \
                                        exe_price, exe_qty, comm_qty);
    
    // 维护策略OrderVector
    if( ret )
    {
        OrderBlock order_block;
        order_block.symbol = symbol;
        order_block.oriPrice = price;
        order_block.exePrice = exe_price;
        order_block.commissionQty = comm_qty;

        strategyOrders[strategy_id].push_back(order_block);
    }
    else
        sptrAsyncLogger->error("OrderManager::StrategyBuySignal() InsertOrder Error !");
}

//##################################################//
//   策略卖出信号
//##################################################//
void OrderManager::StrategySellSignal(int strategy_id, std::string symbol, double price)
{
    // 计算报单qty
    double exe_price = 0.0;
    double exe_qty = 0.0;
    double comm_qty = 0.0;
    double qty = 0.0;
    for(auto& order_block : strategyOrders[strategy_id])
    {
        if(order_block.symbol==symbol)
            qty = order_block.totalQty - order_block.commissionQty;
    }

    // 币安报单
    bool ret = uptrBiTrader->InsertOrder(symbol, Binance::OrderSide::SELL, 0.0, qty, \
                                        Binance::OrderType::MARKET, Binance::TimeInForce::GTC, \
                                        exe_price, exe_qty, comm_qty);
    
    // 维护策略OrderVector
    if( ret )
    {
        for(auto it = strategyOrders[strategy_id].begin(); it != strategyOrders[strategy_id].end(); it++)
        {
            if(it->symbol==symbol)
            {
                strategyOrders[strategy_id].erase(it);
                break;
            }
        }
    }
    else
        sptrAsyncLogger->error("OrderManager::StrategySellSignal() InsertOrder Error !");
}
