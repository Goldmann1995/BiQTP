/*
 * File:        BiDef.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-23
 * LastEdit:    2024-03-23
 * Description: Binance Defination
 */

#include "BiDef.h"


//########################################//
//   namespace @ Binance
//########################################//
namespace Binance
{
    // 获取订单状态
    const std::string GetOrderSide(OrderSide side)
    {
        switch( side )
        {
            case OrderSide::BUY:
                return "BUY";
            case OrderSide::SELL:
                return "SELL";
            default:
                return "";
        }
    };

    const std::string GetOrderType(OrderType type)
    {
        switch( type )
        {
            case OrderType::LIMIT:
                return "LIMIT";
            case OrderType::MARKET:
                return "MARKET";
            case OrderType::STOP_LOSS:
                return "STOP_LOSS";
            case OrderType::STOP_LOSS_LIMIT:
                return "STOP_LOSS_LIMIT";
            case OrderType::TAKE_PROFIT:
                return "TAKE_PROFIT";
            case OrderType::TAKE_PROFIT_LIMIT:
                return "TAKE_PROFIT_LIMIT";
            case OrderType::LIMIT_MAKER:
                return "LIMIT_MAKER";
            default:
                return "";
        }
    };

    const std::string GetTimeInForce(TimeInForce tif)
    {
        switch( tif )
        {
            case TimeInForce::GTC:
                return "GTC";
            case TimeInForce::IOC:
                return "IOC";
            case TimeInForce::FOK:
                return "FOK";
            default:
                return "";
        }
    };
}
