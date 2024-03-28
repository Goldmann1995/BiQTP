/*
 * File:        OrderManager.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-28
 * LastEdit:    2024-03-28
 * Description: Order Manager
 */

#pragma once

#include <ThreadBase.h>


/*** 信号控制Block ***/
struct SignalBlock
{
    int strategyId;
    std::string symbol;
    int side;            // 1=买入 // 2=卖出 //
    double price;

    SignalBlock()
    {
        strategyId = 0;
        symbol = "";
        side = 0;
        price = 0.0;
    }
};

/*** 订单控制Block ***/
struct OrderBlock
{
    std::string symbol;
    double oriPrice;
    double exePrice;
    double totalQty;
    double commissionQty;

    OrderBlock()
    {
        symbol = "";
        oriPrice = 0.0;
        exePrice = 0.0;
        totalQty = 0.0;
        commissionQty = 0.0;
    }
};

//############################################################//
//   OrderManager Class
//############################################################//
class OrderManager: public ThreadBase
{
public:
    OrderManager();
    ~OrderManager();

    // 线程运行实体
    void Run();
    // 策略推送信号
    void PushSignal(int strategy_id, std::string symbol, int side, double price);
    // 策略买入信号
    void StrategyBuySignal(int strategy_id, std::string symbol, double price);
    // 策略卖出信号
    void StrategySellSignal(int strategy_id, std::string symbol, double price);

private:
    double totalCaptical;
    double unitCaptical;
    int orderCnt;

    std::mutex signalMtx;
    std::queue<SignalBlock> signalQueue;
    std::vector<OrderBlock> strategyOrders[ST_SIZE];
};

//##################################################//
