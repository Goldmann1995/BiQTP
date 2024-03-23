/*
 * File:        BiFilter.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-23
 * LastEdit:    2024-03-23
 * Description: Binance Exchange Info
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "BiFilter.h"


//########################################//
//   namespace @ Binance
//########################################//
namespace Binance
{
    SymbolFilter::SymbolFilter()
    {
        // 币种名字
        symbolName = "";
        // 支持的订单类型
        orderTypeVec = {};
        // 价格过滤器
        minPrice = "";
        maxPrice = "";
        tickSize = "";
        pricePrecision = 0;
        // 尺寸过滤器
        minQty = "";
        maxQty = "";
        stepSize = "";
        qtyPrecision = 0;
        // 名义价值过滤器
        minNotional = "";
        maxNotional = "";
    }

    SymbolFilter::~SymbolFilter()
    {
        // 支持的订单类型
        //orderTypeVec.clear();
        orderTypeVec = {};
    }

    void SymbolFilter::SetSymbolName(const std::string& symbol)
    {
        symbolName = symbol;
    }

    const std::string& SymbolFilter::GetSymbolName()
    {
        return symbolName;
    }

    void SymbolFilter::AddOrderType(std::string type)
    {
        if(type == STR_LIMIT)
            orderTypeVec.push_back(OrderType::LIMIT);
        else if(type == STR_MARKET)
            orderTypeVec.push_back(OrderType::MARKET);
        else if(type == STR_STOP_LOSS)
            orderTypeVec.push_back(OrderType::STOP_LOSS);
        else if(type == STR_STOP_LOSS_LIMIT)
            orderTypeVec.push_back(OrderType::STOP_LOSS_LIMIT);
        else if(type == STR_TAKE_PROFIT)
            orderTypeVec.push_back(OrderType::TAKE_PROFIT);
        else if(type == STR_TAKE_PROFIT_LIMIT)
            orderTypeVec.push_back(OrderType::TAKE_PROFIT_LIMIT);
        else if(type == STR_LIMIT_MAKER)
            orderTypeVec.push_back(OrderType::LIMIT_MAKER);
        else
            return;
    }

    void SymbolFilter::DelOrderType(Binance::OrderType type)
    {
        auto it = std::find(orderTypeVec.begin(), orderTypeVec.end(), type);
        if(it != orderTypeVec.end())
            orderTypeVec.erase(it);
    }

    bool SymbolFilter::IsSupportOrderType(Binance::OrderType type)
    {
        auto it = std::find(orderTypeVec.begin(), orderTypeVec.end(), type);
        if(it != orderTypeVec.end())
            return true;
        else
            return false;
    }

    void SymbolFilter::SetMinPrice(std::string price)
    {
        minPrice = price;
    }

    void SymbolFilter::SetMaxPrice(std::string price)
    {
        maxPrice = price;
    }

    void SymbolFilter::SetTickSize(std::string size)
    {
        tickSize = size;
        // ~
        //if(size == PRECISION0)
        if(size.substr(0,3) == std::string(PRECISION0).substr(0,3))
            pricePrecision=0;
        else if(size == PRECISION1)
            pricePrecision=1;
        else if(size == PRECISION2)
            pricePrecision=2;
        else if(size == PRECISION3)
            pricePrecision=3;
        else if(size == PRECISION4)
            pricePrecision=4;
        else if(size == PRECISION5)
            pricePrecision=5;
        else if(size == PRECISION6)
            pricePrecision=6;
        else if(size == PRECISION7)
            pricePrecision=7;
        else if(size == PRECISION8)
            pricePrecision=8;
        else
            fprintf(stderr, "SymbolFilter::SetTickSize() Error: %s\n", size.c_str());
    }

    double SymbolFilter::GetMinPrice()
    {
        return std::stod(minPrice);
    }

    double SymbolFilter::GetMaxPrice()
    {
        return std::stod(maxPrice);
    }

    unsigned int SymbolFilter::GetTickSize()
    {
        return pricePrecision;
    }

    void SymbolFilter::SetMinQty(std::string price)
    {
        minQty = price;
    }

    void SymbolFilter::SetMaxQty(std::string price)
    {
        maxQty = price;
    }

    void SymbolFilter::SetStepSize(std::string size)
    {
        stepSize = size;
        // ~
        //if(size == PRECISION0)
        if(size.substr(0,3) == std::string(PRECISION0).substr(0,3))
            qtyPrecision=0;
        else if(size == PRECISION1)
            qtyPrecision=1;
        else if(size == PRECISION2)
            qtyPrecision=2;
        else if(size == PRECISION3)
            qtyPrecision=3;
        else if(size == PRECISION4)
            qtyPrecision=4;
        else if(size == PRECISION5)
            qtyPrecision=5;
        else if(size == PRECISION6)
            qtyPrecision=6;
        else if(size == PRECISION7)
            qtyPrecision=7;
        else if(size == PRECISION8)
            qtyPrecision=8;
        else
            fprintf(stderr, "SymbolFilter::SetStepSize() Error: %s\n", size.c_str());
    }

    double SymbolFilter::GetMinQty()
    {
        return std::stod(minQty);
    }

    double SymbolFilter::GetMaxQty()
    {
        return std::stod(maxQty);
    }

    unsigned int SymbolFilter::GetStepSize()
    {
        return qtyPrecision;
    }

    void SymbolFilter::SetMinNotional(std::string notional)
    {
        minNotional = notional;
    }

    void SymbolFilter::SetMaxNotional(std::string notional)
    {
        maxNotional = notional;
    }

    double SymbolFilter::GetMinNotional()
    {
        return std::stod(minNotional);
    }

    double SymbolFilter::GetMaxNotional()
    {
        return std::stod(maxNotional);
    }

    void SymbolFilter::PrintExchInfo()
    {
        std::cout << symbolName << std::endl;

        for(auto& orderType: orderTypeVec)
            std::cout << GetOrderType(orderType) << std::endl;

        std::cout << minPrice << std::endl;
        std::cout << maxPrice << std::endl;
        std::cout << tickSize << std::endl;
        std::cout << pricePrecision << std::endl;

        std::cout << minQty << std::endl;
        std::cout << maxQty << std::endl;
        std::cout << stepSize << std::endl;
        std::cout << qtyPrecision << std::endl;

        std::cout << minNotional << std::endl;
        std::cout << maxNotional << std::endl;
    }
}
