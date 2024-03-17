
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
#include <ThreadBase.h>

#include "Param.h"
#include "Order.h"

using namespace std;

// 秘钥
string api_key = "g6QbFD9UUOyupTr0IWoGnKBrqpaLkgU444DJNyNEVe9pbbNsM0u41UtaBVMpyfNY";
string secret_key = "4eTOYwLt0iZFh3WvcLs7AHvd80hfkbxaVwAMqtKtj7aGZmY8psDmzdKp1Sh2ZXMW";

// 策略全局变量
int hb_cnt = 0;
int signal_cnt = 0;

unordered_map<string, int> index_umap;
vector<class SymbolTrend> trend_vec;

queue<double> price_hfm_queue[SYMBOL_CNT];
queue<double> price_1m_queue[SYMBOL_CNT];
queue<double> price_2m_queue[SYMBOL_CNT];
queue<double> price_3m_queue[SYMBOL_CNT];
queue<double> price_5m_queue[SYMBOL_CNT];
queue<double> price_10m_queue[SYMBOL_CNT];
queue<double> price_30m_queue[SYMBOL_CNT];
queue<double> price_60m_queue[SYMBOL_CNT];

double total_profit;
double total_commission;
vector<class Order> orders;

string doubleToStringWithPrecision(double value, int precision = 2)
{
    ostringstream oss;
    if(value>200.0)
        return to_string((int)value);
    else if(100.0<value && value<200.0)
        oss << std::fixed << std::setprecision(1) << value;
    else if(10.0<value && value<100.0)
        oss << std::fixed << std::setprecision(precision) << value;
    else
        oss << std::fixed << std::setprecision(3) << value;
    return oss.str();
}

std::string hmac_sha256(const std::string &secretKey, const std::string &data);
// libcurl回调函数
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);
// Libcurl write callback function
size_t PostWriteCallback(void *contents, size_t size, size_t nmemb, std::string *s);

void UpdatePriceQueue(int symbol_idx, double price);
void UpdateTrendFlag(int symbol_idx);
void ResetTrendFlag(int symbol_idx);

bool SLRFinder(int symbol_idx);
void SLRDeleter(int symbol_idx, double price, CURL *curl_trader);

void PrintSymbolSignal(string symbol);
void PrintSymbolInfo(string symbol);

void UpdateOrders();


/***** Main Entry *****/
int main()
{
    CURLcode res;
    string readBuffer;

    // 交易curl初始化
    CURL *curl_trader;
    curl_trader = curl_easy_init();
    if(!curl_trader)
    {
        cout << "curl_trader Init Fail !" << endl;
        return -1;
    }

    // 行情curl初始化
    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl)
    {
        // curl配置
        curl_easy_setopt(curl, CURLOPT_URL, "https://api3.binance.com/api/v3/ticker/price");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        /*** symbol映射初始化 ***/
        {
            // 打印HeartBeat
            cout << "# Init SLRFinder UMap #" << endl;

            // 执行GET请求
            res = curl_easy_perform(curl);

            // 检查错误
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                return -1;
            }
            else
            {
                int index = 0;
                rapidjson::Document jsondoc;
                rapidjson::ParseResult psret = jsondoc.Parse(readBuffer.c_str());
                if(psret)
                {
                    for(const auto& item : jsondoc.GetArray())
                    {
                        string symbol = item["symbol"].GetString();
                        //string price = item["price"].GetString();
                        if(symbol.find("USDT") != string::npos)
                        {
                            cout << "Init # Index: " << index << "  Symbol: " << symbol << endl;

                            index_umap.insert(make_pair(symbol,index));
                            SymbolTrend sym_trend;
                            trend_vec.push_back(sym_trend);

                            index++;
                        }
                    }

                    cout << "-----------------------------" << endl;
                }
                else
                {
                    cout << "JsonParse Error" << endl;
                    return -1;
                }

                readBuffer.clear();
                sleep(5);
            }            
        }

        /*** 启动策略 ***/
        while(true)
        {
            // 打印HeartBeat
            cout << "##### HB: " << ++hb_cnt << " #####" << endl;
            cout << "##### Sig: " << signal_cnt << " #####" << endl;
            cout << "##### Profit: " << setprecision(6) << total_profit << " #####" << endl;
            cout << "##### Commission: " << setprecision(6) << total_commission << " #####" << endl;
            cout << "-----------------------------" << endl;

            // 执行GET请求
            res = curl_easy_perform(curl);

            // 检查错误
            if(res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }
            else
            {
                // 打印JSON回报
                //cout << readBuffer << endl;

                rapidjson::Document jsondoc;
                rapidjson::ParseResult psret = jsondoc.Parse(readBuffer.c_str());
                if(psret)
                {
                    for(const auto& item : jsondoc.GetArray())
                    {
                        string str_symbol = item["symbol"].GetString();
                        string str_price = item["price"].GetString();
                        double db_price = stod(str_price);
                        int symbol_idx = index_umap[str_symbol];

                        if(str_symbol.find("USDT") != string::npos)
                        {
                            //cout << "symbol:" << str_symbol << " index:" << symbol_idx << "   " << "price:" << db_price << endl;

                            /***** 更新价格队列 *****/
                            UpdatePriceQueue(symbol_idx, db_price);

                            /***** 更新价格趋势Flag *****/
                            UpdateTrendFlag(symbol_idx);

                            /***** Seed Finder Strategy of summer *****/
                            if( trend_vec[symbol_idx].track_flag )
                            {
                                PrintSymbolInfo(str_symbol);
                                SLRDeleter(symbol_idx, db_price, curl_trader);
                            }
                            else if( SLRFinder(symbol_idx) )
                            {
                                signal_cnt++;
                                
                                trend_vec[symbol_idx].track_flag = true;
                                trend_vec[symbol_idx].track_price = db_price;
                                trend_vec[symbol_idx].track_cnt = hb_cnt;

                                PrintSymbolSignal(str_symbol);

                                // 模拟下单
                                Order new_order;
                                new_order.symbol = str_symbol;
                                new_order.in_price = db_price;
                                new_order.qty = 100.0/db_price;
                                orders.push_back(new_order);

                                cout << " \033[32m BUY " << str_symbol;
                                cout << " Buy: " << setprecision(6) << db_price << " Qty: " << 100.0/db_price << " \033[0m" << endl;
                                cout << "---------------------------------------------------------------------------------------" << endl;

                                // 真实下单
                                string buyBuffer;
                                auto now = chrono::system_clock::now();
                                auto duration = now.time_since_epoch();
                                auto millis = chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                                string queryString = "symbol="+str_symbol+"&side=BUY&type=MARKET&quantity="+doubleToStringWithPrecision(100.0/db_price)+"&timestamp="+to_string(millis);
                                string signature = hmac_sha256(secret_key, queryString);
                                string url = "https://api.binance.com/api/v3/order";
                                string data = queryString + "&signature=" + signature;

                                struct curl_slist *chunk = NULL;
                                chunk = curl_slist_append(chunk, ("X-MBX-APIKEY: " + api_key).c_str());
                                curl_easy_setopt(curl_trader, CURLOPT_HTTPHEADER, chunk);
                                curl_easy_setopt(curl_trader, CURLOPT_URL, url.c_str());
                                curl_easy_setopt(curl_trader, CURLOPT_POSTFIELDS, data.c_str());
                                curl_easy_setopt(curl_trader, CURLOPT_WRITEFUNCTION, PostWriteCallback);
                                curl_easy_setopt(curl_trader, CURLOPT_WRITEDATA, &buyBuffer);
                                
                                res = curl_easy_perform(curl_trader);
                                if(res != CURLE_OK )
                                {
                                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                                }
                                cout << data << endl;
                                cout << buyBuffer << endl;
                            }

                            // 重置价格趋势Flag
                            ResetTrendFlag(symbol_idx);
                        }
                    }
                }
                else
                {
                    cout << "JsonParse Error" << endl;
                }
            }

            readBuffer.clear();
            // 一次curl往返大概500ms
            sleep(5);
        }

        // 清理
        curl_easy_cleanup(curl_trader);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

// 更新价格队列
void UpdatePriceQueue(int symbol_idx, double db_price)
{
    // 更新30s队列
    price_hfm_queue[symbol_idx].push(db_price);
    if(price_hfm_queue[symbol_idx].size()>6)
        price_hfm_queue[symbol_idx].pop();

    // 更新1min队列
    price_1m_queue[symbol_idx].push(db_price);
    if(price_1m_queue[symbol_idx].size()>12)
        price_1m_queue[symbol_idx].pop();

    // 更新2min队列
    price_2m_queue[symbol_idx].push(db_price);
    if(price_2m_queue[symbol_idx].size()>24)
        price_2m_queue[symbol_idx].pop();

    // 更新3min队列
    price_3m_queue[symbol_idx].push(db_price);
    if(price_3m_queue[symbol_idx].size()>36)
        price_3m_queue[symbol_idx].pop();

    // 更新5min队列
    price_5m_queue[symbol_idx].push(db_price);
    if(price_5m_queue[symbol_idx].size()>60)
        price_5m_queue[symbol_idx].pop();

    // 更新10min队列
    price_10m_queue[symbol_idx].push(db_price);
    if(price_10m_queue[symbol_idx].size()>120)
        price_10m_queue[symbol_idx].pop();
                            
    // 更新30min队列
    price_30m_queue[symbol_idx].push(db_price);
    if(price_30m_queue[symbol_idx].size()>360)
        price_30m_queue[symbol_idx].pop();
                            
    // 更新60min队列
    price_60m_queue[symbol_idx].push(db_price);
    if(price_60m_queue[symbol_idx].size()>720)
        price_60m_queue[symbol_idx].pop();
}

// 更新价格趋势
void UpdateTrendFlag(int symbol_idx)
{
    // 更新30s标识
    if(price_hfm_queue[symbol_idx].size()==6)
    {
        double front_price = price_hfm_queue[symbol_idx].front();
        double back_price = price_hfm_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_hfm = front_price;
        trend_vec[symbol_idx].back_hfm = back_price;
        trend_vec[symbol_idx].profit_hfm = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_hfm > Mhf_UP_THH )
        {
            trend_vec[symbol_idx].sflag_hfm = true;
        }
    }

    // 更新1min标识
    if(price_1m_queue[symbol_idx].size()==12)
    {
        double front_price = price_1m_queue[symbol_idx].front();
        double back_price = price_1m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_1m = front_price;
        trend_vec[symbol_idx].back_1m = back_price;
        trend_vec[symbol_idx].profit_1m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_1m > M1_UP_THH )
        {
            trend_vec[symbol_idx].sflag_1m = true;
        }
    }

    // 更新2min标识
    if(price_2m_queue[symbol_idx].size()==24)
    {
        double front_price = price_2m_queue[symbol_idx].front();
        double back_price = price_2m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_2m = front_price;
        trend_vec[symbol_idx].back_2m = back_price;
        trend_vec[symbol_idx].profit_2m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_2m > M2_UP_THH )
        {
            trend_vec[symbol_idx].sflag_2m = true;
        }
    }

    // 更新3min标识
    if(price_3m_queue[symbol_idx].size()==36)
    {
        double front_price = price_3m_queue[symbol_idx].front();
        double back_price = price_3m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_3m = front_price;
        trend_vec[symbol_idx].back_3m = back_price;
        trend_vec[symbol_idx].profit_3m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_3m > M3_UP_THH )
        {
            trend_vec[symbol_idx].sflag_3m = true;
        }
    }

    // 更新5min标识
    if(price_5m_queue[symbol_idx].size()==60)
    {
        double front_price = price_5m_queue[symbol_idx].front();
        double back_price = price_5m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_5m = front_price;
        trend_vec[symbol_idx].back_5m = back_price;
        trend_vec[symbol_idx].profit_5m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_5m > M5_UP_THH )
        {
            trend_vec[symbol_idx].sflag_5m = true;
        }
    }

    // 更新10min标识
    if(price_10m_queue[symbol_idx].size()==120)
    {
        double front_price = price_10m_queue[symbol_idx].front();
        double back_price = price_10m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_10m = front_price;
        trend_vec[symbol_idx].back_10m = back_price;
        trend_vec[symbol_idx].profit_10m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_10m > M10_UP_THH )
        {
            trend_vec[symbol_idx].sflag_10m = true;
        }
    }

    // 更新30min标识
    if(price_30m_queue[symbol_idx].size()==360)
    {
        double front_price = price_30m_queue[symbol_idx].front();
        double back_price = price_30m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_30m = front_price;
        trend_vec[symbol_idx].back_30m = back_price;
        trend_vec[symbol_idx].profit_30m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_30m > M30_UP_THH )
        {
            trend_vec[symbol_idx].sflag_30m = true;
        }
    }

    // 更新60min标识
    if(price_30m_queue[symbol_idx].size()==720)
    {
        double front_price = price_60m_queue[symbol_idx].front();
        double back_price = price_60m_queue[symbol_idx].back();
        trend_vec[symbol_idx].front_60m = front_price;
        trend_vec[symbol_idx].back_60m = back_price;
        trend_vec[symbol_idx].profit_60m = 100.0*(back_price-front_price)/front_price;
        if( trend_vec[symbol_idx].profit_60m > M60_UP_THH )
        {
            trend_vec[symbol_idx].sflag_60m = true;
        }
    }
}

// 重置价格趋势
void ResetTrendFlag(int symbol_idx)
{
    trend_vec[symbol_idx].sflag_hfm = false;
    trend_vec[symbol_idx].sflag_1m = false;
    trend_vec[symbol_idx].sflag_2m = false;
    trend_vec[symbol_idx].sflag_3m = false;
    trend_vec[symbol_idx].sflag_5m = false;
    trend_vec[symbol_idx].sflag_10m = false;
    trend_vec[symbol_idx].sflag_30m = false;
    trend_vec[symbol_idx].sflag_60m = false;
}

bool SLRFinder(int symbol_idx)
{
    if( trend_vec[symbol_idx].sflag_hfm && \
        trend_vec[symbol_idx].sflag_1m && \
        trend_vec[symbol_idx].sflag_2m && \
        trend_vec[symbol_idx].sflag_3m && \
        !trend_vec[symbol_idx].sflag_5m )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SLRDeleter(int symbol_idx, double db_price, CURL *curl_trader)
{
    if( (hb_cnt-trend_vec[symbol_idx].track_cnt)>24 && !trend_vec[symbol_idx].sflag_2m )
        trend_vec[symbol_idx].track_flag = false;

    if( (hb_cnt-trend_vec[symbol_idx].track_cnt)>60 && !trend_vec[symbol_idx].sflag_5m )
        trend_vec[symbol_idx].track_flag = false;

    if( !trend_vec[symbol_idx].track_flag )
    {
        for(int i=0; i<orders.size(); i++)
        {
            if(index_umap[orders[i].symbol]== symbol_idx)
            {
                // 模拟下单
                double profit = (db_price-orders[i].in_price)*orders[i].qty;
                double commission = db_price*orders[i].qty/1000;
                total_profit += profit;
                total_commission += commission;
                cout << " \033[32m SELL " << orders[i].symbol;
                cout << " Sell: " << setprecision(6) << db_price<< " Buy: " << orders[i].in_price << " Qty: " << orders[i].qty;
                cout << " Profit: " <<  profit << " Commission: " <<  commission << " \033[0m" << endl;
                cout << "---------------------------------------------------------------------------------------" << endl;
                
                // 真实下单
                CURLcode res;
                string sellBuffer;

                auto now = chrono::system_clock::now();
                auto duration = now.time_since_epoch();
                auto millis = chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                string queryString = "symbol="+orders[i].symbol+"&side=SELL&type=MARKET&quantity="+doubleToStringWithPrecision(orders[i].qty*0.998)+"&timestamp="+to_string(millis);
                string signature = hmac_sha256(secret_key, queryString);
                string url = "https://api.binance.com/api/v3/order";
                string data = queryString + "&signature=" + signature;

                struct curl_slist *chunk = NULL;
                chunk = curl_slist_append(chunk, ("X-MBX-APIKEY: " + api_key).c_str());
                curl_easy_setopt(curl_trader, CURLOPT_HTTPHEADER, chunk);
                curl_easy_setopt(curl_trader, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl_trader, CURLOPT_POSTFIELDS, data.c_str());
                curl_easy_setopt(curl_trader, CURLOPT_WRITEFUNCTION, PostWriteCallback);
                curl_easy_setopt(curl_trader, CURLOPT_WRITEDATA, &sellBuffer);
                                
                res = curl_easy_perform(curl_trader);
                if(res != CURLE_OK )
                {
                    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                }
                cout << data << endl;
                cout << sellBuffer << endl;

                orders.erase(orders.begin()+i);
                break;
            }
        }
    }
}

void PrintSymbolSignal(string symbol)
{
    int symbol_idx = index_umap[symbol];
    cout << "\033[1;32m New Signal ! \033[0m" << endl;

    // 30s策略
    if(price_hfm_queue[symbol_idx].size()==6)
    {
        cout << " \033[1;32m" << symbol << "   30s up " << setprecision(4) << trend_vec[symbol_idx].profit_hfm << " %\033[0m" << endl;
        //cout << "       " << front_1m << " ~ " << back_1m << endl;
    }

    // 1min策略
    if(price_1m_queue[symbol_idx].size()==12)
    {
        cout << " \033[1;32m" << symbol << "  1min up " << trend_vec[symbol_idx].profit_1m << " %\033[0m" << endl;
        //cout << "       " << front_1m << " ~ " << back_1m << endl;
    }

    // 2min策略
    if(price_1m_queue[symbol_idx].size()==24)
    {
        cout << " \033[1;32m" << symbol << "  1min up " << trend_vec[symbol_idx].profit_2m << " %\033[0m" << endl;
        //cout << "       " << front_1m << " ~ " << back_1m << endl;
    }

    // 3min策略
    if(price_3m_queue[symbol_idx].size()==36)
    {
        cout << " \033[1;32m" << symbol << "  3min up " << trend_vec[symbol_idx].profit_3m << " %\033[0m" << endl;
        //cout << "       " << front_3m << " ~ " << back_3m << endl;
    }

    // 5min策略
    if(price_5m_queue[symbol_idx].size()==60)
    {
        cout << " \033[1;32m" << symbol << "  5min up " << trend_vec[symbol_idx].profit_5m << " %\033[0m" << endl;
        //cout << "       " << front_5m << " ~ " << back_5m << endl;
    }

    // 10min策略
    if(price_10m_queue[symbol_idx].size()==120)
    {
        cout << " \033[1;32m" << symbol << " 10min up " << trend_vec[symbol_idx].profit_10m << " %\033[0m" << endl;
        //cout << "       " << front_10m << " ~ " << back_10m << endl;
    }

    // 30min策略
    if(price_30m_queue[symbol_idx].size()==360)
    {
        cout << " \033[1;32m" << symbol << " 30min up " << trend_vec[symbol_idx].profit_30m << " %\033[0m" << endl;
        //cout << "       " << front_30m << " ~ " << back_30m << endl;
    }

    // 60min策略
    if(price_60m_queue[symbol_idx].size()==720)
    {
        cout << " \033[1;32m" << symbol << " 60min up " << trend_vec[symbol_idx].profit_60m << " %\033[0m" << endl;
        //cout << "       " << front_60m << " ~ " << back_60m << endl;
    }

    cout << "-----------------------------" << endl;
}

void PrintSymbolInfo(string symbol)
{
    int symbol_idx = index_umap[symbol];

    cout << " " << setw(10) <<  symbol << " ";
    // 30s策略
    if(price_hfm_queue[symbol_idx].size()==6)
        cout << " | 30s: " << setprecision(2) << trend_vec[symbol_idx].profit_hfm << "%";
    // 1min策略
    if(price_1m_queue[symbol_idx].size()==12)
        cout << " | 1m: " << setprecision(2) << trend_vec[symbol_idx].profit_1m << "%";
    // 3min策略
    if(price_3m_queue[symbol_idx].size()==36)
        cout << " | 3m: " << setprecision(2) << trend_vec[symbol_idx].profit_3m << "%";
    else
    {
        cout << endl << "---------------------------------------------------------------------------------------" << endl;
        return;
    }
    // 5min策略
    if(price_5m_queue[symbol_idx].size()==60)
        cout << " | 5m: " << trend_vec[symbol_idx].profit_5m << "%";
    else
    {
        cout << endl << "---------------------------------------------------------------------------------------" << endl;
        return;
    }
    // 10min策略
    if(price_10m_queue[symbol_idx].size()==120)
        cout << " | 10m: " << trend_vec[symbol_idx].profit_10m << "%";
    else
    {
        cout << endl << "---------------------------------------------------------------------------------------" << endl;
        return;
    }
    // 30min策略
    if(price_30m_queue[symbol_idx].size()==360)
        cout << " | 30m: " << trend_vec[symbol_idx].profit_30m << "%";
    else
    {
        cout << endl << "---------------------------------------------------------------------------------------" << endl;
        return;
    }
    // 60min策略
    if(price_60m_queue[symbol_idx].size()==720)
        cout << " | 60m: " << trend_vec[symbol_idx].profit_60m << "%" << endl;
    else
    {
        cout << endl << "---------------------------------------------------------------------------------------" << endl;
        return;
    }

    cout << "---------------------------------------------------------------------------------------" << endl;
}

void UpdateOrders()
{
    // ~
}

std::string hmac_sha256(const std::string &secretKey, const std::string &data)
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(), \
                    reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), NULL, NULL);
    char mdString[65];
    for(int i = 0; i < 32; i++)
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    return std::string(mdString);
}

// 此回调函数提供给libcurl调用
// 用于获取响应数据
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    userp->append((char*)contents, size *nmemb);
    return size *nmemb;
}

// Libcurl write callback function
size_t PostWriteCallback(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t newLength = size * nmemb;
    try
    {
        s->append((char*)contents, newLength);
    }
    catch(std::bad_alloc &e)
    {
        // handle memory problem
        return 0;
    }
    return newLength;
}

/********** END **********/
