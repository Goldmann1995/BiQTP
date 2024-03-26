/*****************************************/
/*****       BiQTP Global MACRO      *****/
/*****************************************/

#pragma once


// BiQTP版本号
#define BI_QTP_VERSION   "Alpha"
#define BI_QTP_VERDATE   "20240325"

// 回测模式
#define _BACK_TEST_   0


/********** 配置 **********/

// Binance币种容量上限
#define TOTAL_SYMBOL   500

// 策略容量
#define ST_SIZE     50


/********** RingMD **********/

#define RING_SIZE   (24*60*60)

#define INTERVAL30S   30
#define INTERVAL1M    60
#define INTERVAL2M    (60*2)
#define INTERVAL3M    (60*3)
#define INTERVAL5M    (60*5)
#define INTERVAL10M   (60*10)
#define INTERVAL20M   (60*20)
#define INTERVAL25M   (60*25)
#define INTERVAL30M   (60*30)
#define INTERVAL60M   (60*60)
#define INTERVAL100M  (60*100)
#define INTERVAL120M  (60*60*2)
#define INTERVAL180M  (60*60*3)
#define INTERVAL240M  (60*60*4)


/********** MDReceiver **********/

#define REQ_TIME_INTERVAL   3


/********** StrategyID **********/

#define ASLR1      0
#define ASLR2      1
#define MACROSS1   2
#define MACROSS2   3
#define GRID       4


/********** Strategy AdvancedSLR1 **********/


/************************************/
/*****        FF of MACRO       *****/
/************************************/
