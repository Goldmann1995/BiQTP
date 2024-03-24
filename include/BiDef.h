/*
 * File:        BiDef.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-23
 * LastEdit:    2024-03-23
 * Description: Binance Defination
 */

#ifndef _BI_DEF_H_
#define _BI_DEF_H_

#include <string>
#include <string.h>
#include <stdint.h>


#define PRECISION0  "1.00000000"
#define PRECISION1  "0.10000000"
#define PRECISION2  "0.01000000"
#define PRECISION3  "0.00100000"
#define PRECISION4  "0.00010000"
#define PRECISION5  "0.00001000"
#define PRECISION6  "0.00000100"
#define PRECISION7  "0.00000010"
#define PRECISION8  "0.00000001"

#define STR_LIMIT              "LIMIT"
#define STR_MARKET             "MARKET"
#define STR_STOP_LOSS          "STOP_LOSS"
#define STR_STOP_LOSS_LIMIT    "STOP_LOSS_LIMIT"
#define STR_TAKE_PROFIT        "TAKE_PROFIT"
#define STR_TAKE_PROFIT_LIMIT  "TAKE_PROFIT_LIMIT"
#define STR_LIMIT_MAKER        "LIMIT_MAKER"


//########################################//
//   namespace @ Binance
//########################################//
namespace Binance
{
    //constexpr int LOG_MSG_LEN = 1024;

    // 订单状态
    enum OrderStatus
    {
        NEW               = 1,   // 订单被交易引擎接收
        PARTIALLY_FILLED  = 2,   // 订单部分成交
        FILLED            = 3,   // 订单完全成交
        CANCELED          = 4,   // 订单已撤销
        PENDING_CANCEL    = 5,   // 撤销中 (目前并未使用)
        REJECTED          = 6,   // 订单被交易引擎拒绝
        EXPIRED           = 7    // 订单被交易引擎取消
    };

    // 订单方向
    enum OrderSide
    {
        BUY  = 1,   // 买入
        SELL = 2    // 卖出
    };

    // 订单类型
    enum OrderType
    {
        LIMIT             = 1,   // 限价单
        MARKET            = 2,   // 市价单
        STOP_LOSS         = 3,   // 止损单
        STOP_LOSS_LIMIT   = 4,   // 限价止损单
        TAKE_PROFIT       = 5,   // 止盈单
        TAKE_PROFIT_LIMIT = 6,   // 限价止盈单
        LIMIT_MAKER       = 7    // 限价只挂单
    };

    // 订单有效方式
    enum TimeInForce
    {
        GTC = 1,   // 成交为止
        IOC = 2,   // 无法立即成交的部分就撤销
        FOK = 3    // 无法全部立即成交就撤销
    };


    // 获取订单状态
    const std::string GetOrderSide(OrderSide side);

    const std::string GetOrderType(OrderType type);

    const std::string GetTimeInForce(TimeInForce tif);

    // miniTicker
    struct miniTicker
    {
        int64_t timestamp;
        std::string symbol;
        std::string price;
        std::string volume;
        std::string amount;
    };
}

#endif /* _BI_DEF_H_ */
