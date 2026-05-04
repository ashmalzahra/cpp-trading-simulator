#pragma once
#include <string>
#include <map>
#include "OrderBookEntry.h"

class Wallet
{
public:
    Wallet();

    void insertCurrency(std::string type, double amount);
    bool removeCurrency(std::string type, double amount);
    bool containsCurrency(std::string type, double amount);
    double getBalance(const std::string &currency);

    std::string toString();

    bool canFulfillOrder(OrderBookEntry order);
    void processSale(OrderBookEntry &sale);

    // Centralised wallet persistence
    void loadFromCSV(const std::string &username);
    void saveToCSV();

    void clear();

private:
    std::map<std::string, double> currencies;
    std::string currentUsername; // active user context
};

std::ostream &operator<<(std::ostream &os, Wallet &wallet);
