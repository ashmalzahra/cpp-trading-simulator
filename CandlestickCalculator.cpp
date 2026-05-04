#include "CandlestickCalculator.h"
#include <map>
#include <algorithm>
#include <limits>

std::vector<Candlestick> CandlestickCalculator::computeDailyCandles(const std::vector<OrderBookEntry> &entries)
{
    std::map<std::string, std::vector<OrderBookEntry>> grouped;

    for (auto &e : entries)
    {
        std::string day = e.timestamp.substr(0, 10);
        // Normalize separators so grouping keys are consistent even if timestamps contain '/'
        std::replace(day.begin(), day.end(), '/', '-');
        grouped[day].push_back(e);
    }

    std::vector<Candlestick> candles;

    for (auto &pair : grouped)
    {
        // List order determines Open/Close, sort by timestamp before using front()/back() if needed
        auto &list = pair.second;
        std::sort(list.begin(), list.end(),
                  [](const OrderBookEntry &a, const OrderBookEntry &b)
                  {
                      return a.timestamp < b.timestamp;
                  });

        double open = list.front().price;
        double close = list.back().price;
        double high = list.front().price;
        double low = list.front().price;

        // Scan all entries in this timeframe to find the highest and lowest traded prices
        for (auto &e : list)
        {
            if (e.price > high)
                high = e.price;
            if (e.price < low)
                low = e.price;
        }

        candles.emplace_back(pair.first, open, high, low, close);
    }

    return candles;
}

std::vector<Candlestick> CandlestickCalculator::computeMonthlyCandles(const std::vector<OrderBookEntry> &entries)
{
    std::map<std::string, std::vector<OrderBookEntry>> grouped;

    for (auto &e : entries)
    {
        std::string month = e.timestamp.substr(0, 7);
        std::replace(month.begin(), month.end(), '/', '-');
        grouped[month].push_back(e);
    }

    std::vector<Candlestick> candles;

    for (auto &pair : grouped)
    {
        auto &list = pair.second;
        std::sort(list.begin(), list.end(),
                  [](const OrderBookEntry &a, const OrderBookEntry &b)
                  {
                      return a.timestamp < b.timestamp;
                  });

        double open = list.front().price;
        double close = list.back().price;
        double high = list.front().price;
        double low = list.front().price;

        // High/Low are extremes within the bucket (day/month/year)
        for (auto &e : list)
        {
            if (e.price > high)
                high = e.price;
            if (e.price < low)
                low = e.price;
        }

        candles.emplace_back(pair.first, open, high, low, close);
    }

    return candles;
}

std::vector<Candlestick> CandlestickCalculator::computeYearlyCandles(const std::vector<OrderBookEntry> &entries)
{
    std::map<std::string, std::vector<OrderBookEntry>> grouped;

    for (auto &e : entries)
    {
        std::string year = e.timestamp.substr(0, 4);
        grouped[year].push_back(e);
    }

    std::vector<Candlestick> candles;

    for (auto &pair : grouped)
    {
        auto &list = pair.second;
        std::sort(list.begin(), list.end(),
                  [](const OrderBookEntry &a, const OrderBookEntry &b)
                  {
                      return a.timestamp < b.timestamp;
                  });

        double open = list.front().price;
        double close = list.back().price;
        double high = list.front().price;
        double low = list.front().price;

        for (auto &e : list)
        {
            if (e.price > high)
                high = e.price;
            if (e.price < low)
                low = e.price;
        }

        candles.emplace_back(pair.first, open, high, low, close);
    }

    return candles;
}
