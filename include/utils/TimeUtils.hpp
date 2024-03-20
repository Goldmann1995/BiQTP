/*
 * File:   TimeUtils.hpp
 * Author: summer @ZMLAB
 * CreateDate: 2023-03-02
 * LastEdit:   2023-03-02
 * Description: Time Utils for Linux & StockTrader
 */


#pragma once

#include <stdint.h>
#include <string>
#include <string.h>

//#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#include <chrono>


// tips summer@20230306
// 设计接口注意考虑多线程访问的安全和可重入问题

//@C++// struct tm
//@C++// {
//@C++//     int tm_sec;     // 秒    [范围0-59]
//@C++//     int tm_min;     // 分钟  [范围0-59]
//@C++//     int tm_hour;    // 小时  [范围0-23]
//@C++//     int tm_mday;    // 日    [范围1-31]
//@C++//     int tm_mon;     // 月    [范围0-11] (1+p->tm_mon)
//@C++//     int tm_year;    // 年    从1900至今的年数 (1900＋p->tm_year)
//@C++//     int tm_wday;    // 星期  从星期日算起       [范围0-6]
//@C++//     int tm_yday;    // 从今年1月1日到目前的天数 [范围0-365]
//@C++//     int tm_isdst;   // 夏令时标识
//@C++// };

//@C++// Unix时间 (POSIX时间)
//@C++// 时间值time_t为长整型的别名 typedef long time_t
//@C++// 表示从1970年1月1日0时0分0秒到某一个时间点的秒数
//@C++// 不晚于2038年1月18日19时14分07秒

////////////////////////////////////////////////////////
///                                                  ///
///                   Linux工具集合                   ///
///                                                  ///
////////////////////////////////////////////////////////
namespace Utils
{
    //@C++// constexpr表达式在编译过程就能确定计算结果
    constexpr int64_t MILLISECS_PER_SEC = 1000;        // 毫秒
    constexpr int64_t MICROSECS_PER_MILLISEC = 1000;   // 微秒
    constexpr int64_t NANOSECS_PER_MICROSEC = 1000;    // 纳秒
    constexpr int64_t NANOSECS_PER_MILLISEC = 1000 * 1000;
    constexpr int64_t NANOSECS_PER_SEC = 1000 * 1000 * 1000;

    constexpr int64_t SECS_PER_MINUTE = 60;
    constexpr int64_t MINUTES_PER_HOUR = 60;
    constexpr int64_t HOURS_PER_DAY = 24;

    constexpr int TIME_ZONE = 8;   // 东八区

    constexpr int STOCK_PRE   =  0;   // 盘前
    constexpr int STOCK_TRADE =  1;   // 盘中
    constexpr int STOCK_CLOSE = -1;   // 收盘


    ////////////////////////////////////////////////////////
    ///                                                  ///
    ///                     时间工具类                    ///
    ///                                                  ///
    ////////////////////////////////////////////////////////
    class TimeUtils
    {
    public:
        /************************************************************************/
        // 函数名:    GetStockStatus
        // 函数功能:  获取股市状态
        /************************************************************************/
        static inline int GetMarketStatus()
        {
            time_t t = time(0);   // get time now
            struct tm *now = localtime(&t);

            // 盘前
            if( now->tm_hour < 9 )
                return STOCK_PRE;
            // 盘前
            else if( now->tm_hour==9 && now->tm_min<14 )
                return STOCK_PRE;
            // 收盘
            else if( now->tm_hour==15 && now->tm_min>=30 )
                return STOCK_CLOSE;
            // 收盘
            else if( now->tm_hour > 15 )
                return STOCK_CLOSE;
            // 盘中
            else
                return STOCK_TRADE;
        };

        /************************************************************************/
        // 函数名:    GetDate
        // 函数功能:  获取系统当日日期
        // 函数说明:  GetDate("%Y%m%d") == 20221006
        //           GetDate("%w") == 4
        /************************************************************************/
        static inline std::string GetDate(const char *date_format)
        {
            time_t t = time(0);   // get time now
            struct tm *now = localtime(&t);
            char buffer[128];
            strftime(buffer, 128, date_format, now);
            return std::string(buffer);
        };

        /************************************************************************/
        // 函数名:    GetDate
        // 函数功能:  获取系统当日日期
        // 函数说明:  默认%Y-%m-%d格式
        /************************************************************************/
        static inline std::string GetDate()
        {
            return GetDate("%Y-%m-%d");
        };

        /************************************************************************/
        // 函数名:    GetLastTradeDate
        // 函数功能:  获取上一个交易日日期
        // 函数说明:  长假失效，需要手动指定
        /************************************************************************/
        static inline std::string GetLastTradeDate(const char *date_format)
        {
            time_t t = time(0);
            struct tm *now = localtime(&t);

            if( now->tm_wday == 6 )
            {
                t -= SECS_PER_MINUTE * MINUTES_PER_HOUR * HOURS_PER_DAY * 2;
                now = localtime(&t);
            }
            else if( now->tm_wday == 0 )
            {
                t -= SECS_PER_MINUTE * MINUTES_PER_HOUR * HOURS_PER_DAY * 3;
                now = localtime(&t);
            }
            else if( now->tm_wday == 1 )
            {
                t -= SECS_PER_MINUTE * MINUTES_PER_HOUR * HOURS_PER_DAY * 3;
                now = localtime(&t);
            }
            else
            {
                t -= SECS_PER_MINUTE * MINUTES_PER_HOUR * HOURS_PER_DAY * 1;
                now = localtime(&t);
            }

            char buffer[100];
            strftime(buffer, 100, date_format, now);
            return std::string(buffer);
        };

        /************************************************************************/
        // 函数名:    GetLastTradeDate
        // 函数功能:  获取上一个交易日日期
        // 函数说明:  默认%Y-%m-%d格式
        /************************************************************************/
        static inline std::string GetLastTradeDate()
        {
            return GetLastTradeDate("%Y-%m-%d");
        }
    };


    /**************************************************************/
    //   每月天数 (平年)
    /**************************************************************/
    //                      MONTH = { Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec };
    static const int MONTH_DAYS[] = { 31 , 28 , 31 , 30 , 31 , 30 , 31 , 31 , 30 , 31 , 30 , 31  };
    static const int  ACCU_DAYS[] = { 31 , 59 , 90 , 120, 151, 181, 212, 243, 273, 304, 334, 365 };

    // tips summer@20230307
    // HF接口设计考虑空间换时间的思路

    ////////////////////////////////////////////////////////
    ///                                                  ///
    ///                    时间戳转换器                   ///
    ///                                                  ///
    ////////////////////////////////////////////////////////
    class TimeConverter
    {
    public:
        /************************************************************************/
        // 函数名:    RapidMakeTime
        // 函数功能:  将指定时间转换为Unix时间 (以东八区时间为准)
        // 函数说明:  mktime()库函数可能会遇到较大时间开销
        //           在HF中用RapidMakeTime()替换mktime()
        /************************************************************************/
        static inline unsigned long RapidMakeTime(unsigned int iYear, unsigned int iMonth, unsigned int iDay, \
                                                  unsigned int iHour, unsigned int iMin, unsigned int iSec)
        {
            unsigned long lDays = 0;
            unsigned long lSeconds = 0;
            unsigned long retTS = 0;

            /***** 计算今年以前的天数 *****/
            int preYear = iYear - 1;   // 扣除今年
            // 闰年统计
            int leapYears = 0;
            leapYears = preYear/4 - preYear/100;   // 计算普通闰年
            leapYears += preYear/400;              // 加上世纪闰年
            lDays = preYear * 365 + leapYears;     // 闰年补1天
            
            /***** 计算今年天数 *****/
            // 判断今年是否闰年
            int bLeapYear = 0;
            if( ( iYear%4==0 && iYear%100!=0 ) || iYear%400==0 )
                bLeapYear = 1;
            // 到上个月为止总共度过的天数
            //lDays += ACCU_DAYS[iMonth-2];
            for( unsigned int i=0; i<(iMonth-1); i++ )
                lDays += MONTH_DAYS[i];
            if( iMonth > 2 )
                lDays += bLeapYear;   // 闰年2月补1天
            lDays = lDays + iDay - 1;   // 扣除今天
            
            // 减去1970年以前的天数 (1970/1/1 00:00:00)
            // preYear = 1969
            // leapYears = 1969/4-1969/100 + 1969/400 = 492 - 19 + 4 = 477
            // bLeapYear = 0
            // lDays = 1969 * 365 + 477 = 719162
            retTS = (lDays - 719162) * 24 * 60 * 60;

            /***** 计算今天经过的秒数 *****/
            lSeconds = (iHour * 60 * 60) + (iMin * 60) + iSec;
            retTS += lSeconds;

            /***** 处理时区问题 *****/
            // 东八区时间比格林尼治时间快8个小时
            // 返回秒钟总数应该扣减时区时间
            // 即 seconds -= 60 * 60 * 8
            retTS -= ((unsigned long)TIME_ZONE) * SECS_PER_MINUTE * MINUTES_PER_HOUR;

            /***** 返回秒钟总数 (Unix时间) *****/
            return retTS;
        }

        /************************************************************************/
        // 函数名:    SZBTime2Stamp
        // 函数功能:  将东吴FPGA深交所行情时间戳转换为Unix时间戳
        // 函数说明:  时间格式 (int64_t)YYYYMMDDHHMMSSsss
        /************************************************************************/
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
            
            stm.tm_year = iY-1900;
            stm.tm_mon = iM-1;
            stm.tm_mday = iD;
            stm.tm_hour = iH;
            stm.tm_min = iMin;
            stm.tm_sec = iS;

            uint64_t retstamp = (uint64_t)mktime(&stm)*1000 + (szb_tmf%1000);
            return retstamp;   // 精确到ms
        }

        /************************************************************************/
        // 函数名:    RapidDwSZBTime2Stamp
        // 函数功能:  将东吴FPGA深交所行情时间戳转换为Unix时间戳
        // 函数说明:  调用RapidMakeTime实现
        /************************************************************************/
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

        /************************************************************************/
        // 函数名:    SSETime2Stamp
        // 函数功能:  将东吴FPGA上交所行情时间戳转换为Unix时间戳
        // 函数说明:  Deep时间格式 (int32_t)HHMMSS
        //           Tick时间格式 (int32_t)HHMMSSss  精确到百分之一秒
        //           sse_tmf统一到百分之一秒处理
        /************************************************************************/
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
            stm.tm_year=today->tm_year;
            stm.tm_mon=today->tm_mon;
            stm.tm_mday=today->tm_mday;
            stm.tm_hour=iH;
            stm.tm_min=iMin;
            stm.tm_sec=iS;
        #endif

            uint64_t retstamp = (uint64_t)mktime(&stm)*1000 + (sse_tmf%100)*10;
            return retstamp;   // 精确到ms
        }

        /************************************************************************/
        // 函数名:    RapidDwSSETime2Stamp
        // 函数功能:  将东吴FPGA上交所行情时间戳转换为Unix时间戳
        // 函数说明:  调用RapidMakeTime实现
        /************************************************************************/
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

        /************************************************************************/
        // 函数名:    QNDTime2Stamp
        // 函数功能:  将QND行情时间戳转换为Unix时间戳
        // 函数说明:  QND时间格式 YYYYMMDD + HHMMSSsss
        /************************************************************************/
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
            
            stm.tm_year = iY-1900;
            stm.tm_mon = iM-1;
            stm.tm_mday = iD;
            stm.tm_hour = iH;
            stm.tm_min = iMin;
            stm.tm_sec = iS;

            /*printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);*/   //标准时间格式例如：2016:08:02 12:12:30
            uint64_t retstamp = (uint64_t)mktime(&stm)*1000 + (msec%1000);
            return retstamp;   // 精确到ms
        }

        /************************************************************************/
        // 函数名:    RapidQNDTime2Stamp
        // 函数功能:  将QND行情时间戳转换为Unix时间戳
        // 函数说明:  调用RapidMakeTime实现
        /************************************************************************/
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

        /************************************************************************/
        // 函数名:    RapidQNDTime2Stamp
        // 函数功能:  将QND行情时间戳转换为Unix时间戳
        // 函数说明:  调用RapidMakeTime实现
        /************************************************************************/
        static inline uint64_t RapidQNDTime2Stamp(const int &date, const int &hour, const int &min, const int &sec)
        {
            int iY, iM, iD, iH, iMin, iS;

            iS = sec;
            iMin = min;
            iH = hour;

            int day = date;
            iD = day%100;
            day = day/100;
            iM = day%100;
            day = day/100;
            iY = day;

            uint64_t retstamp = (uint64_t)RapidMakeTime(iY, iM, iD, iH, iMin, iS)*1000;
            return retstamp;   // 精确到ms
        }
    };

    ////////////////////////////////////////////////////////
    ///                                                  ///
    ///                    时间戳生成器                   ///
    ///                                                  ///
    ////////////////////////////////////////////////////////
    class TimeStamper
    {
    public:
        TimeStamper() = default;

        // QTS启动时必须首先执行初始化
        static void init()
        {
            // 系统时钟
            auto now = std::chrono::system_clock::now();
            system_start_ts = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
            // 单调时钟
            steady_start_ts = std::chrono::steady_clock::now().time_since_epoch().count();
        }

        // think summer@20230306
        // 考虑直接/截断还是duration_cast转换
        // unix-timestamp * 1e9 + nano-part

        /*** 获取时间戳 [纳秒] ***/
        static uint64_t nano_ts()
        {
            auto duration = std::chrono::steady_clock::now().time_since_epoch().count() - steady_start_ts;
            return (system_start_ts + duration);
        }

        /*** 获取时间戳 [微秒] ***/
        static uint64_t micro_ts()
        {
            auto duration = std::chrono::steady_clock::now().time_since_epoch().count() - steady_start_ts;
            return (system_start_ts + duration) / NANOSECS_PER_MICROSEC;
        }

        /*** 获取时间戳 [毫秒] ***/
        static uint64_t milli_ts()
        {
            auto duration = std::chrono::steady_clock::now().time_since_epoch().count() - steady_start_ts;
            return (system_start_ts + duration) / NANOSECS_PER_MILLISEC;
        }

        /*** 获取时间戳 [秒] ***/
        static uint64_t sec_ts()
        {
            auto duration = std::chrono::steady_clock::now().time_since_epoch().count() - steady_start_ts;
            return (system_start_ts + duration) / NANOSECS_PER_SEC;
        }

    private:
        static int64_t system_start_ts;   // 系统时钟
        static int64_t steady_start_ts;   // 单调时钟
    };

    ////////////////////////////////////////////////////////
    ///                                                  ///
    ///                    高精度计时器                   ///
    ///                                                  ///
    ////////////////////////////////////////////////////////
    class Ticker
    {
    public:
        /*** 初始化计时器 ***/
        Ticker()
        {
            start_tick = std::chrono::high_resolution_clock::now();
        }

        /*** 重置计时器 ***/
        void reset()
        {
            start_tick = std::chrono::high_resolution_clock::now();
        }

        /*** 返回时间间隔 [纳秒级] ***/
        inline int64_t nano_seconds() const
        {
            auto duration = std::chrono::high_resolution_clock::now() - start_tick;
            return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        }

        /*** 返回时间间隔 [微秒级] ***/
        inline int64_t micro_seconds() const
        {
            auto duration = std::chrono::high_resolution_clock::now() - start_tick;
            return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }

        /*** 返回时间间隔 [毫秒级] ***/
        inline int64_t milli_seconds() const
        {
            auto duration = std::chrono::high_resolution_clock::now() - start_tick;
            return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        }

        /*** 返回时间间隔 [秒级] ***/
        inline int64_t seconds() const
        {
            auto duration = std::chrono::high_resolution_clock::now() - start_tick;
            return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        }

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start_tick;
    };
}
