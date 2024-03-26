/*
 * File:        TimeUtils.hpp
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-27
 * LastEdit:    2024-03-27
 * Description: Time Utils for Linux & StockTrader
 */

#pragma once

#include <stdint.h>
#include <string>
#include <string.h>

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
///                 Linux时间工具集合                  ///
///                                                  ///
////////////////////////////////////////////////////////
namespace TimeUtils
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
    ///                    时间戳转换器                    ///
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

        // QTP启动时必须首先执行初始化
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
