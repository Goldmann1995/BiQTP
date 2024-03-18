///////////////////////////////////////////////////////////////////////////
///                                                                     ///
///                          Binance QTP                                ///
///                                                                     ///
///////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
//#include <string>
#include <string.h>
#include <chrono>

// STL
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

// lib
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <rapidjson/document.h>

// include
//#include <ThreadBase.h>

//#include "Param.h"
#include "MDRing.h"
#include "BiIniter.h"
#include "Global.h"

using namespace std;


/********** Main Entry **********/
int main()
{
    // tips summer@20240318 - 是否应该移到main()之前
    // 初始化libcurl库全局环境
    curl_global_init(CURL_GLOBAL_ALL);

    // Binance-QTP初始化
    BiIniter initer("https://api3.binance.com");
    initer.InitSymbolUMap();
    //initer.InitSymbolFilter();
    initer.UpdateSymbolFilter();

    // ~
    curl_global_cleanup();

    return 0;
}

/********** END **********/
