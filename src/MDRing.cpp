/*
 * File:        MDRing.cpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: MarketData & CompuRes Ring
 */

#include <iostream>
#include <string>

#include "MDRing.h"


//##################################################//
//   Constructor
//##################################################//
MDRing::MDRing()
{
    //mSymbol = symbol;
}

MDRing::MDRing(const std::string& symbol)
{
    mSymbol = symbol;
}

//##################################################//
//   Destructor
//##################################################//
MDRing::~MDRing()
{
    //
}

void MDRing::SetSymbolName(const std::string& symbol)
{
    mSymbol = symbol;
}

const std::string& MDRing::GetSymbolName()
{
    return mSymbol;
}

void MDRing::PushMD(double last_price)
{
    int index = md_index+1/RING_SIZE;
    price[index] = last_price;
    md_index=index;
}

void MDRing::CalMovingAverage()
{
    if(cal_index==md_index)
        return;
}

void MDRing::CalADRatio()
{
    if(cal_index==md_index)
        return;
}

void MDRing::SetMinPrice(const std::string& price)
{
    minPrice = std::stod(price);
}

void MDRing::SetMaxPrice(const std::string& price)
{
    maxPrice = std::stod(price);
}

void MDRing::SetTickSize(const std::string& size)
{
    tickSize = std::stod(size);
    if(size == PRECISION0)
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
        std::cout << "error" << std::endl;
}

void MDRing::SetMinQty(const std::string& qty)
{
    minQty = std::stod(qty);
}

void MDRing::SetMaxQty(const std::string& qty)
{
    maxQty = std::stod(qty);
}

void MDRing::SetStepSize(const std::string& size)
{
    stepSize = std::stod(size);
    // tips summer@20240318 - 换一种方式比较浮点值
    if(size == PRECISION0 || stepSize == 1.0)
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
        std::cout << "error" << std::endl;
}

void MDRing::SetMinNotional(const std::string& notional)
{
    minNotional = std::stod(notional);
}

void MDRing::SetMaxNotional(const std::string& notional)
{
    maxNotional = std::stod(notional);
}

void MDRing::PrintExchangeInfo()
{
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
