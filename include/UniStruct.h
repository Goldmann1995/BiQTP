/*
 * File:        UniStruct.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-18
 * LastEdit:    2024-03-18
 * Description: Universal Struct of QTP
 */

#ifndef _UNI_STRUCT_H_
#define _UNI_STRUCT_H_

#include <string>
#include <string.h>
#include <stdint.h>


//@C++// 通道push()操作必须赋值运算符重载

//########################################//
//   namespace @ UniStruct
//########################################//
namespace UniStruct
{
    constexpr int LOG_MSG_LEN = 1024;

    /***** 主运行日志 *****/
	struct LogMessage
	{
        enum LOG_TYPE
        {
            LT_DEBUG = 1,
            LT_INFO  = 2,
            LT_WARN  = 3,
            LT_ERROR = 4
        };

        int type;
		char content[LOG_MSG_LEN];

        LogMessage()
        {
            type = 0;
            memset(content, 0, LOG_MSG_LEN);
        }

        // 拷贝构造函数
        LogMessage(const LogMessage& other)
        {
            if(this != &other)
            {
                type = other.type;
                memcpy(content, other.content, LOG_MSG_LEN);
            }
        }

        // 赋值运算符重载
		LogMessage& operator=(const LogMessage& other)
		{
            if(this != &other)
            {
                type = other.type;
                memcpy(content, other.content, LOG_MSG_LEN);
            }
			return *this;
		}
	};
}

#endif /* _UNI_STRUCT_H_ */
