/*
 * File:        StringUtils.hpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-27
 * LastEdit:    2024-03-27
 * Description: String Utils for Linux & StockTrader
 */

#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <string.h>


////////////////////////////////////////////////////////
///                                                  ///
///                   String工具集合                  ///
///                                                  ///
////////////////////////////////////////////////////////
namespace StringUtils
{
    /************************************************************************/
    // 函数名:    double2string
    // 函数功能:  转换指定精度double值
    // 函数说明:  ~
    /************************************************************************/
    std::string double2string(double value, int precision)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
}
