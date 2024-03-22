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
    ////////////////////////////////////////////////////////
    ///                                                  ///
    ///                     时间工具类                    ///
    ///                                                  ///
    ////////////////////////////////////////////////////////
	namespace Timer
	{
		inline static int GetTradePhase()
		{
			time_t t = time(0);   // get time now
			struct tm *now = localtime(& t);

            if( now->tm_hour<9 )
                return 0;   // 未开盘
            else if( now->tm_hour==9 && now->tm_min<14 )
                return 0;   // 未开盘
            else if( now->tm_hour==15 && now->tm_min>=30 )
                return -1;   // 已收盘
            else if( now->tm_hour>15 )
                return -1;   // 已收盘
            else
                return 1;   // 盘中
		};

		// GetDate("%Y%m%d") == 20221006
		// GetDate("%w") == 4
		inline static std::string GetDate(const char* date_format)
		{
			time_t t = time(0);   // get time now
			struct tm * now = localtime(& t);
			char buffer[100];
			strftime(buffer, 100, date_format, now);
			return std::string(buffer);
		};

		// Default Date Format
		inline static std::string GetDate()
		{
			return GetDate("%Y-%m-%d");
		};

        //--------------------------------------------------
        //@C++// 
        //@C++// 
        //--------------------------------------------------

        /************************************************************************/
        //   平年每月天数
        //   时区
        /************************************************************************/
        /////////////////////////////  { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12}
        static const int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        static const int TIME_ZONE = 8;   // 东八区

        /************************************************************************/
        // 函数名:     RapidMakeTime
        // 函数功能:   计算当前时间到格林威治时间总共经过的秒钟数 (以东八区时间为准)
        // 函数参数:   
        // 函数返回值: 
        /************************************************************************/
        static inline unsigned long RapidMakeTime(unsigned int iYear, unsigned int iMonth, unsigned int iDay, \
                                                  unsigned int iHour, unsigned int iMin, unsigned int iSec)
        {
            unsigned long lDays = 0;
            unsigned long lSeconds = 0;
            unsigned long retTS = 0;

            /***** 计算今年以前的天数 *****/
            int preYear = iYear - 1;
            int leapYears = 0;                     // 闰年统计
            leapYears = preYear/4 - preYear/100;   // 计算普通闰年
            leapYears += preYear/400;              // 加上世纪闰年
            lDays = preYear * 365 + leapYears;     // 闰年补1天
            
            /***** 计算今年天数 *****/
            // 判断今年是否闰年
            int bLeapYear = 0;
            if( (iYear%4 == 0 && iYear % 100!=0) || iYear%400==0 )
                bLeapYear = 1;
            
            // 到上个月为止总共度过的天数
            for( unsigned int i=0; i<(iMonth-1); i++ )
                lDays += monthDays[i];
            if( iMonth > 2 )
                lDays += bLeapYear;   // 闰年2月补1天
            lDays = lDays + iDay - 1;
            
            // 减去1970年以前的天数 (1970/1/1 00:00:00)
            // preYear = 1969
            // leapYears = 1969/4-1969/100 + 1969/400 = 492 - 19 + 4 = 477
            // bLeapYear = 0
            // lDays = 1969 * 365 + 477 = 719162
            retTS = (lDays - 719162) * 24 * 60 * 60;

            // 加上今天经过的秒数
            lSeconds = (iHour * 60 * 60) + (iMin * 60) + iSec;
            retTS += lSeconds;

            /***** 处理时区问题 *****/
            // 北京东八区比本初子午线时间快8个小时
            // 返回秒钟总数应该扣减时区时间
            // 即 seconds -= 60 * 60 * 8
            retTS -= ((unsigned long)TIME_ZONE) * 60 * 60;

            // 返回秒钟总数
            return retTS;
        }

		// 东吴FPGA行情 SZB
		// 时间格式 (int64_t)YYYYMMDDHHMMSSsss
        static inline uint64_t SZBTime2Stamp(const int64_t& szb_tmf)
        {
            struct tm stm;
            memset(&stm, 0, sizeof(stm));
            int iY, iM, iD, iH, iMin, iS;

            int64_t sec = szb_tmf/1000;
            iS = sec%100;
            sec = sec/100;
            iMin = sec%100;
            sec = sec/100;
            iH = sec%100;
            sec = sec/100;
            iD = sec%100;
            sec = sec/100;
            iM = sec%100;
            sec = sec/100;
            iY = sec;
            
            stm.tm_year=iY-1900;
            stm.tm_mon=iM-1;
            stm.tm_mday=iD;
            stm.tm_hour=iH;
            stm.tm_min=iMin;
            stm.tm_sec=iS;

            /*printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);*/   //标准时间格式例如：2016:08:02 12:12:30
            uint64_t retstamp = (uint64_t)mktime(&stm)*1000 + (szb_tmf%1000);
            return retstamp;   // 精确到ms
        }

		// 东吴FPGA行情 SZB
		// 时间格式 (int64_t)YYYYMMDDHHMMSSsss
        static inline uint64_t RapidDwSZBTime2Stamp(const int64_t& szb_tmf)
        {
            int iY, iM, iD, iH, iMin, iS;

            int64_t sec = szb_tmf/1000;
            iS = sec%100;
            sec = sec/100;
            iMin = sec%100;
            sec = sec/100;
            iH = sec%100;
            sec = sec/100;
            iD = sec%100;
            sec = sec/100;
            iM = sec%100;
            sec = sec/100;
            iY = sec;

            uint64_t retstamp = (uint64_t)RapidMakeTime(iY, iM, iD, iH, iMin, iS)*1000 + (szb_tmf%1000);
            return retstamp;   // 精确到ms
        }

        // 东吴FPGA行情 SSE
		// Deep时间格式 (int32_t)HHMMSS
		// Tick时间格式 (int32_t)HHMMSSss  精确到百分之一秒
		// sse_tmf统一到百分之一秒处理
        static inline uint64_t SSETime2Stamp(const int32_t& sse_tmf)
        {
            struct tm stm;
            memset(&stm, 0, sizeof(stm));
            int iY, iM, iD, iH, iMin, iS;

            int64_t sec = sse_tmf/100;
            iS = sec%100;
            sec = sec/100;
            iMin = sec%100;
            sec = sec/100;
            iH = sec;
            
        #if 1   // 测试全量行情 2022.09.21
            iY = 2022;
            iM = 9;
            iD = 21;
            stm.tm_year=iY-1900;
            stm.tm_mon=iM-1;
            stm.tm_mday=iD;
            stm.tm_hour=iH;
            stm.tm_min=iMin;
            stm.tm_sec=iS;
		#endif

		#if 0   // 生产全量行情
			time_t timel;
			time(&timel);
			tm *today = gmtime(&timel);
            stm.tm_year=today.tm_year;
            stm.tm_mon=today.tm_mon;
            stm.tm_mday=today.tm_mday;
            stm.tm_hour=iH;
            stm.tm_min=iMin;
            stm.tm_sec=iS;
		#endif

            /*printf("%d-%0d-%0d %0d:%0d:%0d %d\n", iY, iM, iD, iH, iMin, iS, (sse_tmf%100)*10);*/   //标准时间格式例如：2016:08:02 12:12:30 500
            uint64_t retstamp = (uint64_t)mktime(&stm)*1000 + (sse_tmf%100)*10;
            return retstamp;   // 精确到ms
        }

        // 东吴FPGA行情 SSE
		// Deep时间格式 (int32_t)HHMMSS
		// Tick时间格式 (int32_t)HHMMSSss  精确到百分之一秒
		// sse_tmf统一到百分之一秒处理
        static inline uint64_t RapidDwSSETime2Stamp(const int32_t &dateY, const int32_t &dateM, const int32_t &dateD, \
                                                    const int32_t &sse_tmf)
        {
            int iY, iM, iD, iH, iMin, iS;

            int64_t sec = sse_tmf/100;
            iS = sec%100;
            sec = sec/100;
            iMin = sec%100;
            sec = sec/100;
            iH = sec;
            
            iY = dateY;
            iM = dateM;
            iD = dateD;

            uint64_t retstamp = (uint64_t)RapidMakeTime(iY, iM, iD, iH, iMin, iS)*1000 + (sse_tmf%100)*10;
            return retstamp;   // 精确到ms
        }

        // 启能达时间格式转换
        // YYYYMMDD + HHMMSSsss
        static inline uint64_t QNDTime2Stamp(const int &date, const int &msec)
        {
            struct tm stm;
            memset(&stm,0,sizeof(stm));
            int iY, iM, iD, iH, iMin, iS;

            int sec = msec/1000;
            iS = sec%100;
            sec = sec/100;
            iMin = sec%100;
            sec = sec/100;
            iH = sec;

            int day = date;
            iD = day%100;
            day = day/100;
            iM = day%100;
            day = day/100;
            iY = day;
            
            stm.tm_year=iY-1900;
            stm.tm_mon=iM-1;
            stm.tm_mday=iD;
            stm.tm_hour=iH;
            stm.tm_min=iMin;
            stm.tm_sec=iS;

            /*printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);*/   //标准时间格式例如：2016:08:02 12:12:30
            uint64_t retstamp = (uint64_t)mktime(&stm)*1000 + (msec%1000);
            return retstamp;   // 精确到ms
        }

        // 启能达时间格式转换
        // YYYYMMDD + HHMMSSsss
        static inline uint64_t RapidQNDTime2Stamp(const int &date, const int &msec)
        {
            int iY, iM, iD, iH, iMin, iS;

            int sec = msec/1000;
            iS = sec%100;
            sec = sec/100;
            iMin = sec%100;
            sec = sec/100;
            iH = sec;

            int day = date;
            iD = day%100;
            day = day/100;
            iM = day%100;
            day = day/100;
            iY = day;

            uint64_t retstamp = (uint64_t)RapidMakeTime(iY, iM, iD, iH, iMin, iS)*1000 + (msec%1000);
            return retstamp;   // 精确到ms
        }
	}

    ////////////////////////////////////////////////////////
    ///                                                  ///
    ///                    加解密工具类                   ///
    ///                   依赖OpenSSL库                   ///
    ///                                                  ///
    ////////////////////////////////////////////////////////
    namespace SSL
	{
        /************************************************************************/
        // 函数名:     GetHMAC_SHA256
        // 函数功能:   生成基于秘钥的SHA256摘要哈希 (哈希指纹)
        // 函数参数:   待处理字符串
        // 函数返回值:  64位摘要哈希
        /************************************************************************/ 
        static inline std::string GetHMAC_SHA256(const std::string &secretKey, const std::string &data)
        {
            unsigned char* digest;
            digest = HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(), \
                          reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), NULL, NULL);
            char mdString[65];
            for(int i = 0; i < 32; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
            std::string strHMACSHA256 = std::string(mdString);
            return strHMACSHA256;
        }
        
#if 0
        /************************************************************************/
        // 函数名:     GetSHA256
        // 函数功能:   生成SHA256摘要哈希 (哈希指纹)
        // 函数参数:   待处理字符串
        // 函数返回值: 256位摘要哈希
        /************************************************************************/ 
        static inline std::string GetSHA256(const std::string &strData)
        {
            // 声明返回哈希值 (256/sizeof(byte))+1
            unsigned char chHV[33] = {0};
            // 调用SHA256接口
            SHA256((const unsigned char *)strData.c_str(), strData.length(), chHV);
            // 返回256位摘要哈希
            std::string strSHA256 = std::string((const char *)chHV);
            return strSHA256;
        }

        /************************************************************************/
        // 函数名:     GetSHA256Hex
        // 函数功能:   生成十六进制SHA256摘要哈希 (哈希指纹)
        // 函数参数:   待处理字符串
        // 函数返回值: 十六进制256位摘要哈希
        /************************************************************************/ 
        static inline std::string GetSHA256Hex(const std::string &strData)
        {
            // 声明返回哈希值
            unsigned char chHV[33] = {0};
            // 调用SHA256接口
            SHA256((const unsigned char *)strData.c_str(), strData.length(), chHV);

            // 转换十六进制
            char buf[65] = {0};
            char tmp[3] = {0};
            for( int i=0; i<32; i++ )
            {
                sprintf(tmp, "%02x", chHV[i]);
                strcat(buf, tmp);
            }
            // 从32字节截断
            buf[32] = '\0';

            // 返回十六进制256位摘要哈希
            std::string strSHA256Hex = std::string(buf);
            return strSHA256Hex;
        }

        /************************************************************************/
        // 函数名:     DecodeAES256CBC
        // 函数功能:   AES解密 (AES256标准，CBC模式)
        // 函数参数:   密文+秘钥
        // 函数返回值: 原文
        /************************************************************************/
        static inline std::string DecodeAES256CBC(const std::string& strData, const std::string& strKey)
        {
            // tips summer@20230112 - 考虑是否将IV传参
            // 默认将iv全置为'0'
            unsigned char iv[AES_BLOCK_SIZE] = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};

            // 设置AES解密秘钥
            AES_KEY aes_key;
            if (AES_set_decrypt_key((const unsigned char*)strKey.c_str(), strKey.length()*8, &aes_key) < 0)
            {
                //assert(false);
                return "";
            }

            // AES解密 (CBC模式)
            std::string strRet;
            for( unsigned int i = 0; i < strData.length()/AES_BLOCK_SIZE; i++ )
            {
                std::string str16 = strData.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
                unsigned char out[AES_BLOCK_SIZE];
                ::memset(out, 0, AES_BLOCK_SIZE);
                AES_cbc_encrypt((const unsigned char*)str16.c_str(), out, AES_BLOCK_SIZE, &aes_key, iv, AES_DECRYPT);
                strRet += std::string((const char*)out, AES_BLOCK_SIZE);
            }
            return strRet;
        }

        /************************************************************************/
        //   Base64字符集
        /************************************************************************/
        static const std::string Base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        /************************************************************************/
        // 函数名:     IsBase64
        // 函数功能:   判断字节是否包含于Base64字符集
        // 函数参数:   
        // 函数返回值: 
        /************************************************************************/
        static inline bool IsBase64(unsigned char c)
        {
            return ( isalnum(c) || (c == '+') || (c == '/') );
        }

        /************************************************************************/
        // 函数名:     DecodeBase64
        // 函数功能:   Base64解码
        // 函数参数:   Base64字符串
        // 函数返回值: 原始字符串
        /************************************************************************/
        static inline std::string DecodeBase64(const std::string& strData)
        {
            int in_len = strData.size();
            int i = 0;
            int j = 0;
            int in_ = 0;
            unsigned char char_array_4[4], char_array_3[3];
            std::string strRet;

            while( in_len-- && (strData[in_] != '=') && IsBase64(strData[in_]) )
            {
                char_array_4[i++] = strData[in_];
                in_++;
                if( i == 4 )
                {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = Base64Chars.find(char_array_4[i]);

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        strRet += char_array_3[i];
                    
                    i = 0;
                }
            }

            if( i )
            {
                for (j = i; j < 4; j++)
                    char_array_4[j] = 0;
                for (j = 0; j < 4; j++)
                    char_array_4[j] = Base64Chars.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++)
                    strRet += char_array_3[j];
            }

            return strRet;
        }

        /************************************************************************/
        // 函数名:     Decrypt
        // 函数功能:   解密(Python)Encrypt加密后的密文
        // 函数参数:   
        // 函数返回值: 
        /************************************************************************/
        static inline std::string Decrypt(const std::string& strData, const std::string& strKey)
        {
            std::string strBase = DecodeBase64(strData); 
            std::string strSHAKey = GetSHA256(strKey);
            std::string strRet = DecodeAES256CBC(strBase, strSHAKey);
            int retlen = strRet.length() - AES_BLOCK_SIZE - (int)strRet[strRet.length()-1];
            return strRet.substr(AES_BLOCK_SIZE, retlen);
        }
#endif
    }

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

	// inline static std::string get_log_shm_segname()
	// {
	// 	std::stringstream ss;
	// 	//ss << "fastcount-log-" << utils::get_date();
	// 	ss << "fastcount-log-" << Timer::GetDate();
	// 	return ss.str();
	// };

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

    /************************************************************************/
    // 函数名:    PriceRound
    // 函数功能:  转换浮点精度
    // 函数参数:  double src 待转换浮点数
    //            int bits 精度位数
    //            (0表示保留小数点后0位小数，1表示保留1位小数)
    // 函数返回值: 精度转换结果
    /************************************************************************/
    // tips summer@20221024 - 以5结尾的小数存在四舍五入误差
    // Utils::PriceRound(11.24999999, 2) ===> 11.25
    // Utils::PriceRound(11.2451, 2) ===> 11.25
    // Utils::PriceRound(11.245, 2) ===> 11.24
    // Utils::PriceRound(11.244, 2) ===> 11.24
	inline static double PriceRound(double src, int bits)
	{
		std::stringstream ss; 
        ss << std::fixed << std::setprecision(bits) << src;
        ss >> src;
        return src;
	};
}

#endif /* _UTILS_H_ */
