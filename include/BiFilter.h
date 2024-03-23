/*
 * File:        BiFilter.h
 * Author:      summer@SummerLab
 * CreateDate:  2024-03-23
 * LastEdit:    2024-03-23
 * Description: Binance Exchange Info
 */

#ifndef _BI_FILTER_H_
#define _BI_FILTER_H_

#include "BiDef.h"


//########################################//
//   namespace @ Binance
//########################################//
namespace Binance
{
    class SymbolFilter
    {
    public:
        SymbolFilter();
        ~SymbolFilter();
        // 币种名字
        void SetSymbolName(const std::string& symbol);
        const std::string& GetSymbolName();
        // 订单类型
        void AddOrderType(std::string type);
        void DelOrderType(Binance::OrderType type);
        bool IsSupportOrderType(Binance::OrderType type);
        // 价格过滤器
        void SetMinPrice(std::string price);
        void SetMaxPrice(std::string price);
        void SetTickSize(std::string size);
        double GetMinPrice();
        double GetMaxPrice();
        unsigned int GetTickSize();
        // 尺寸过滤器
        void SetMinQty(std::string price);
        void SetMaxQty(std::string price);
        void SetStepSize(std::string size);
        double GetMinQty();
        double GetMaxQty();
        unsigned int GetStepSize();
        // 名义价值过滤器
        void SetMinNotional(std::string notional);
        void SetMaxNotional(std::string notional);
        double GetMinNotional();
        double GetMaxNotional();
        // 打印参数
        void PrintExchInfo();
    private:
        // 币种名字
        std::string symbolName;
        // 支持的订单类型
        std::vector<Binance::OrderType> orderTypeVec;
        // 价格过滤器
        std::string minPrice;
        std::string maxPrice;
        std::string tickSize;
        unsigned int pricePrecision;
        // 尺寸过滤器
        std::string minQty;
        std::string maxQty;
        std::string stepSize;
        unsigned int qtyPrecision;
        // 名义价值过滤器
        std::string minNotional;
        std::string maxNotional;
    };
}

#endif /* _BI_FILTER_H_ */
