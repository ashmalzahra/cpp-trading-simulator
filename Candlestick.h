#pragma once
#include <string>
#include <iostream>
#include <iomanip>

class Candlestick
{
public:
    std::string date;
    double open;
    double high;
    double low;
    double close;

    Candlestick() : open(0), high(0), low(0), close(0) {}

    Candlestick(std::string date, double open, double high, double low, double close)
        : date(date), open(open), high(high), low(low), close(close) {}

    void displayRow() const
    {
        std::cout << std::left
                  << std::setw(14) << date
                  << std::setw(18) << std::fixed << std::setprecision(10) << open
                  << std::setw(18) << high
                  << std::setw(18) << low
                  << std::setw(18) << close
                  << std::endl;
    }
};
