/*
 * File:   Utils.h
 * Author: summer@ZMLAB
 * CreateDate: 2022-10-06
 * LastEdit:   2023-02-06
 * Description: Common Utils of Linux
 */


#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>
/// for NS <Timer> ///
#include <chrono>
#include <time.h>
#include <sys/time.h>
/// for NS <SSL> ///
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/hmac.h>


////////////////////////////////////////////////////////
///                                                  ///
///                    Linux工具类                    ///
///                                                  ///
////////////////////////////////////////////////////////
namespace Utils
{
    /************************************************************************/
    // 函数名:    IsFileExists
    // 函数功能:  判断文件是否存在
    // 函数参数:  filename (std::string传参)
    // 函数返回值: 
    /************************************************************************/
	inline static bool IsFileExists(const std::string& filename)
	{
		struct stat buffer;
		return (::stat(filename.c_str(), &buffer) == 0);
	};

    /************************************************************************/
    // 函数名:    IsFileExists
    // 函数功能:  判断文件是否存在
    // 函数参数:  filename (char*传参)
    // 函数返回值: 
    /************************************************************************/
	inline static bool IsFileExists(const char* filename)
	{
		return IsFileExists(std::string(filename));
	};

    /************************************************************************/
    // 函数名:    GetOutFileDir
    // 函数功能:  获取当日Out文件目录
    // 函数参数:  
    // 函数返回值: 
    /************************************************************************/
	inline static std::string GetOutFileDir(const std::string& f)
	{
		time_t t = time(0);
		struct tm * now = localtime(& t);
		char buffer[100];
		strftime(buffer, 100, "%Y-%m-%d", now);
		std::stringstream ss;
		ss << f << std::string(buffer) << "/";
		return ss.str();
	};

    /************************************************************************/
    // 函数名:    GetOutFilename
    // 函数功能:  获取当日Out文件名称
    // 函数参数:  
    // 函数返回值: 
    /************************************************************************/
	inline static std::string GetOutFilename(const std::string& f)
	{
		time_t t = time(0);
		struct tm * now = localtime(& t);
		char buffer[100];
		strftime(buffer, 100, "%Y%m%d", now);
		std::stringstream ss;
		ss << f << "." << std::string(buffer);
		return ss.str();
	};
}

#endif /* _UTILS_H_ */
