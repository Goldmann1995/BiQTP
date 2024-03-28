/*
 * File:        StringUtils.hpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-27
 * LastEdit:    2024-03-27
 * Description: String Utils for Linux & StockTrader
 */

#pragma once

#include <string.h>
#include <string>
#include <sstream>
#include <cctype>   // 包含std::tolower


////////////////////////////////////////////////////////
///                                                  ///
///                   String工具集合                  ///
///                                                  ///
////////////////////////////////////////////////////////
namespace StringUtils
{
    /************************************************************************/
    // 函数名:    IsSpecStarting
    // 函数功能:  判断字符串是否有指定开头
    // 函数说明:  ~
    /************************************************************************/
    static inline bool IsSpecStarting(const std::string &symbol, const std::string &starting)
    {
        if( symbol.find(symbol) == std::string::npos )
            return false;
        if( symbol.compare(0, starting.length(), starting) )
            return false;
        else
            return true;
    }
    
    /************************************************************************/
    // 函数名:    IsSpecEnding
    // 函数功能:  判断字符串是否有指定结尾
    // 函数说明:  ~
    /************************************************************************/
    static inline bool IsSpecEnding(const std::string &symbol, const std::string &ending)
    {
        if( symbol.find(ending) == std::string::npos )
            return false;
        if( symbol.compare(symbol.length()-ending.length(), ending.length(), ending) )
            return false;
        else
            return true;
    }

    /************************************************************************/
    // 函数名:    ConvertUpper2Lower
    // 函数功能:  大写转换小写
    // 函数说明:  ~
    /************************************************************************/
    static inline std::string ConvertUpper2Lower(const std::string &name)
    {
        std::string lowername(name);
        for(char& c : lowername)
            c = std::tolower(static_cast<unsigned char>(c));
        return lowername;
    }

    /************************************************************************/
    // 函数名:    ConvertLower2Upper
    // 函数功能:  小写转换大写
    // 函数说明:  ~
    /************************************************************************/
    static inline std::string ConvertLower2Upper(const std::string &name)
    {
        std::string uppername(name);
        for(char& c : uppername)
            c = std::toupper(static_cast<unsigned char>(c));
        return uppername;
    }

    /************************************************************************/
    // 函数名:    ConvertDouble2String
    // 函数功能:  转换指定精度double值
    // 函数说明:  ~
    /************************************************************************/
    static inline std::string ConvertDouble2String(double value, int precision)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
}
