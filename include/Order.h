
#include <string>
using namespace std;


/*** SymbolTrend ***/
class SymbolTrend
{
public:
    bool track_flag = false;
    double track_price = 0.0;
    int track_cnt = 0;
    
    bool sflag_hfm = false;
    bool sflag_1m = false;
    bool sflag_2m = false;
    bool sflag_3m = false;
    bool sflag_5m = false;
    bool sflag_10m = false;
    bool sflag_30m = false;
    bool sflag_60m = false;

    double front_hfm = 0.0;
    double back_hfm = 0.0;
    double profit_hfm = 0.0;

    double front_1m = 0.0;
    double back_1m = 0.0;
    double profit_1m = 0.0;

    double front_2m = 0.0;
    double back_2m = 0.0;
    double profit_2m = 0.0;

    double front_3m = 0.0;
    double back_3m = 0.0;
    double profit_3m = 0.0;

    double front_5m = 0.0;
    double back_5m = 0.0;
    double profit_5m = 0.0;

    double front_10m = 0.0;
    double back_10m = 0.0;
    double profit_10m = 0.0;

    double front_30m = 0.0;
    double back_30m = 0.0;
    double profit_30m = 0.0;

    double front_60m = 0.0;
    double back_60m = 0.0;
    double profit_60m = 0.0;
};


/*** Order ***/
class Order
{
public:
    void SetSymbol(string name);
    void SetInPrice(double price);
    void SetOutPrice(double price);
    void SetQuanty(double num);
public:
    string symbol;
    double in_price;
    double out_price;
    double qty;
};

void Order::SetSymbol(string name)
{
    symbol = name;
}

void Order::SetInPrice(double price)
{
    in_price = price;
}

void Order::SetOutPrice(double price)
{
    out_price = price;
}

void Order::SetQuanty(double num)
{
    qty = num;
}
