/*
 * File:        StringUtils.hpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-27
 * LastEdit:    2024-03-27
 * Description: String Utils for Linux & StockTrader
 */

#pragma once

#include <string>
#include <sstream>
#include <string.h>
#include <cctype>   // 包含std::tolower


////////////////////////////////////////////////////////
///                                                  ///
///                   String工具集合                  ///
///                                                  ///
////////////////////////////////////////////////////////
namespace StringUtils
{
    /************************************************************************/
    // 函数名:    IsSpecEnding
    // 函数功能:  判断字符串是否有指定结尾
    // 函数说明:  ~
    /************************************************************************/
    static inline bool IsSpecEnding(const std::string &name, const std::string &ending)
    {
        if( name.find(ending) == std::string::npos )
            return false;
        if( name.compare(name.length()-ending.length(), ending.length(), ending) )
            return false;
        else
            return true;
    }

    /************************************************************************/
    // 函数名:    double2string
    // 函数功能:  转换指定精度double值
    // 函数说明:  ~
    /************************************************************************/
    static inline std::string double2string(double value, int precision)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
}
