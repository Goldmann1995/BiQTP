/*
 * File:        Utils.hpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-27
 * LastEdit:    2024-03-27
 * Description: Utils for Linux
 */

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <string>


////////////////////////////////////////////////////////
///                                                  ///
///                   Linux工具集合                   ///
///                                                  ///
////////////////////////////////////////////////////////
namespace Utils
{
    /************************************************************************/
    // 函数名:    IsFileExists
    // 函数功能:  判断文件是否存在
    // 函数说明:  
    /************************************************************************/
	static inline bool IsFileExists(const std::string& filename)
	{
		struct stat buffer;
		return (::stat(filename.c_str(), &buffer) == 0);
	};

    /************************************************************************/
    // 函数名:    IsFileExists
    // 函数功能:  判断文件是否存在
    // 函数说明:  
    /************************************************************************/
	static inline bool IsFileExists(const char* filename)
	{
		return IsFileExists(std::string(filename));
	};
}
