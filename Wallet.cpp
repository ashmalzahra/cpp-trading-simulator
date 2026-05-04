#include "Wallet.h"
#include <iostream>
#include <fstream>
#include "CSVReader.h"

Wallet::Wallet() {}

void Wallet::insertCurrency(std::string type, double amount)
{
    // Ignore non-positive deposits so balances can't be corrupted by invalid values
    if (amount <= 0)
        return;
    currencies[type] += amount;
}

bool Wallet::removeCurrency(std::string type, double amount)
{
    if (amount <= 0)
        return false;
    if (!containsCurrency(type, amount))
        return false;

    currencies[type] -= amount;
    return true;
}

bool Wallet::containsCurrency(std::string type, double amount)
{
    if (currencies.count(type) == 0)
        return false;
    return currencies[type] >= amount;
}

double Wallet::getBalance(const std::string &currency)
{
    if (currencies.count(currency) == 0)
        return 0.0;
    return currencies[currency];
}

std::string Wallet::toString()
{
    std::string s;
    for (auto &pair : currencies)
    {
        s += pair.first + " : " + std::to_string(pair.second) + "\n";
    }
    return s;
}

bool Wallet::canFulfillOrder(OrderBookEntry order)
{
    auto parts = CSVReader::tokenise(order.product, '/');

    // User must own the base currency amount
    if (order.orderType == OrderBookType::ask)
        return containsCurrency(parts[0], order.amount);

    // User must own enough quote currency to cover amount * price
    if (order.orderType == OrderBookType::bid)
        return containsCurrency(parts[1], order.amount * order.price);

    return false;
}

void Wallet::processSale(OrderBookEntry &sale)
{
    auto parts = CSVReader::tokenise(sale.product, '/');

    if (sale.orderType == OrderBookType::asksale)
    {
        currencies[parts[0]] -= sale.amount;
        currencies[parts[1]] += sale.amount * sale.price;
    }
    else if (sale.orderType == OrderBookType::bidsale)
    {
        currencies[parts[1]] -= sale.amount * sale.price;
        currencies[parts[0]] += sale.amount;
    }
}

// load/save wallet
void Wallet::loadFromCSV(const std::string &username)
{
    currencies.clear();
    currentUsername = username;

    std::ifstream file("wallets.csv");
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line))
    {
        auto parts = CSVReader::tokenise(line, ',');
        if (parts.size() != 3)
            continue;

        // Load only the rows belonging to the currently logged-in user
        if (parts[0] == username)
            currencies[parts[1]] = std::stod(parts[2]);
    }
}

void Wallet::saveToCSV()
{
    // No logged in user context
    if (currentUsername.empty())
        return;

    // Read existing file lines (if file doesn't exist yet, that's fine)
    std::ifstream in("wallets.csv");
    std::vector<std::string> lines;
    std::string line;

    if (in.is_open())
    {
        while (std::getline(in, line))
            lines.push_back(line);
        in.close();
    }

    std::ofstream out("wallets.csv", std::ios::trunc);
    if (!out.is_open())
        return;

    for (const auto &l : lines)
    {
        auto parts = CSVReader::tokenise(l, ',');
        if (parts.size() != 3)
            continue;

        // Skip old rows for the active user so we don't duplicate blocks.
        if (parts[0] == currentUsername)
            continue;

        out << l << "\n";
    }

    // Write current user's wallet entries exactly once
    if (currencies.empty())
    {
        out << currentUsername << ",USDT,0\n";
    }
    else
    {
        for (const auto &c : currencies)
            out << currentUsername << "," << c.first << "," << c.second << "\n";
    }
}

void Wallet::clear()
{
    currencies.clear();
}

std::ostream &operator<<(std::ostream &os, Wallet &wallet)
{
    os << wallet.toString();
    return os;
}
