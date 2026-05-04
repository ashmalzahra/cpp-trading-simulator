#pragma once
#include <vector>
#include "OrderBookEntry.h"
#include "Candlestick.h"

class CandlestickCalculator
{
public:
    static std::vector<Candlestick> computeDailyCandles(const std::vector<OrderBookEntry> &entries);
    static std::vector<Candlestick> computeMonthlyCandles(const std::vector<OrderBookEntry> &entries);
    static std::vector<Candlestick> computeYearlyCandles(const std::vector<OrderBookEntry> &entries);
};
