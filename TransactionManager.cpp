#include "TransactionManager.h"
#include "OrderBookEntry.h"
#include "UIHelpers.h"
#include "DateUtils.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <algorithm>

// Constructor
TransactionManager::TransactionManager(Wallet &w, User &u)
    : wallet(w), user(u)
{
    loadFromCSV();
}

// Log transaction (user scoped)
void TransactionManager::logTransaction(const std::string &type,
                                        const std::string &asset,
                                        double amount,
                                        double value,
                                        const std::string &timestamp)
{
    // Store username inside each row so transactions csv can hold a combined history for multiple users
    Transaction t{user.username, type, asset, amount, value, timestamp};
    transactions.push_back(t);
    saveToCSV();
}

// load/save csv
void TransactionManager::loadFromCSV()
{
    transactions.clear();

    std::ifstream file(filename);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        Transaction t;

        std::getline(ss, t.username, ',');
        std::getline(ss, t.type, ',');
        std::getline(ss, t.asset, ',');
        // Parse numeric fields (amount/value) from CSV, ignore commas between fields
        ss >> t.amount;
        ss.ignore();
        ss >> t.value;
        ss.ignore();
        std::getline(ss, t.timestamp);

        transactions.push_back(t);
    }
}

void TransactionManager::saveToCSV()
{
    if (transactions.empty())
        return;

    // Append only the most recent transaction
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open())
        return;

    const auto &t = transactions.back();

    file << t.username << ","
         << t.type << ","
         << t.asset << ","
         << t.amount << ","
         << t.value << ","
         << t.timestamp << "\n";
}

// Main menu
void TransactionManager::showMenu(const std::string &marketTime)
{
    menuMarketTime = marketTime;
    while (true)
    {
        UI::printTitle("WALLET & TRANSACTIONS");

        std::cout << "1. View Wallet Balance\n";
        std::cout << "2. Deposit Funds\n";
        std::cout << "3. Withdraw Funds\n";
        std::cout << "4. View Transactions\n";
        std::cout << "5. Activity Summary\n";
        std::cout << "6. Back\n";

        UI::printDivider();
        int choice = UI::readIntInRange(1, 6);

        if (choice == 1)
            showWalletBalance();
        else if (choice == 2)
            depositFunds();
        else if (choice == 3)
            withdrawFunds();
        else if (choice == 4)
            showTransactions();
        else if (choice == 5)
            showSummaryMenu();
        else
            return;
    }
}

// Wallet
void TransactionManager::showWalletBalance()
{
    UI::printTitle("WALLET");
    std::cout << wallet;
    UI::printDivider();
}

void TransactionManager::depositFunds()
{
    UI::printTitle("DEPOSIT FUNDS");

    std::cout << "Amount (USDT): ";
    double amount = UI::readDouble();
    if (amount <= 0)
        return;

    wallet.insertCurrency("USDT", amount);
    wallet.saveToCSV();

    logTransaction("deposit", "USDT", amount, amount, menuMarketTime);
    UI::printSuccess("Deposit successful");
}

void TransactionManager::withdrawFunds()
{
    UI::printTitle("WITHDRAW FUNDS");

    double max = wallet.getBalance("USDT");
    std::cout << "Available: " << max << " USDT\n";
    std::cout << "Amount: ";

    double amount = UI::readDouble();
    if (!wallet.removeCurrency("USDT", amount))
    {
        UI::printError("Invalid withdrawal");
        return;
    }

    wallet.saveToCSV();
    logTransaction("withdraw", "USDT", amount, amount, menuMarketTime);
    UI::printSuccess("Withdrawal successful");
}

// Transaction table header
void TransactionManager::printTransactionHeader()
{
    std::cout << std::left
              << std::setw(22) << "TIMESTAMP"
              << std::setw(12) << "TYPE"
              << std::setw(16) << "ASSET"
              << std::setw(18) << "AMOUNT"
              << std::setw(20) << "VALUE"
              << std::endl;

    UI::printDivider();
}

// View Transactions (user scoped)
void TransactionManager::showTransactions()
{
    while (true)
    {
        UI::printTitle("VIEW TRANSACTIONS");

        std::cout << "1. Last 5 transactions\n";
        std::cout << "2. Transactions by product\n";
        std::cout << "3. View all transactions\n";
        std::cout << "4. Back\n";

        UI::printDivider();
        int choice = UI::readIntInRange(1, 4);

        // Back
        if (choice == 4)
            return;

        // Last 5
        if (choice == 1)
        {
            printTransactionHeader();

            int shown = 0;
            // Iterate backwards to show the newest transactions first
            for (int i = transactions.size() - 1; i >= 0 && shown < 5; i--)
            {
                const auto &t = transactions[i];
                if (t.username != user.username)
                    continue;

                std::cout << std::left
                          << std::setw(22) << UI::displayTimestamp(t.timestamp)
                          << std::setw(12) << t.type
                          << std::setw(16) << t.asset
                          << std::setw(18) << std::fixed << std::setprecision(8) << t.amount
                          << std::setw(20) << std::fixed << std::setprecision(8) << t.value
                          << std::endl;

                shown++;
            }

            if (shown == 0)
                UI::printError("No transactions for this user.");
        }

        // By Product
        else if (choice == 2)
        {
            std::set<std::string> products;
            for (const auto &t : transactions)
            {
                if (t.username == user.username &&
                    t.asset.find('/') != std::string::npos)
                {
                    products.insert(t.asset);
                }
            }

            if (products.empty())
            {
                UI::printError("No product-based transactions found.");
                continue;
            }

            UI::printTitle("SELECT PRODUCT");

            std::vector<std::string> prodList(products.begin(), products.end());
            for (int i = 0; i < prodList.size(); i++)
                std::cout << i + 1 << ". " << prodList[i] << std::endl;

            UI::printDivider();
            int pChoice = UI::readIntInRange(1, prodList.size());
            std::string selected = prodList[pChoice - 1];

            printTransactionHeader();

            bool found = false;
            for (int i = static_cast<int>(transactions.size()) - 1; i >= 0; --i)
            {
                const auto &t = transactions[i];

                if (t.username == user.username && t.asset == selected)
                {
                    found = true;

                    std::cout << std::left
                              << std::setw(22) << UI::displayTimestamp(t.timestamp)
                              << std::setw(12) << t.type
                              << std::setw(16) << t.asset
                              << std::setw(18) << std::fixed << std::setprecision(8) << t.amount
                              << std::setw(20) << std::fixed << std::setprecision(8) << t.value
                              << std::endl;
                }
            }

            if (!found)
                UI::printError("No transactions for selected product.");
        }

        // Show all
        else if (choice == 3)
        {
            printTransactionHeader();

            bool found = false;

            // Iterate backwards to show newest transactions first
            for (int i = static_cast<int>(transactions.size()) - 1; i >= 0; --i)
            {
                const auto &t = transactions[i];

                if (t.username == user.username)
                {
                    found = true;

                    std::cout << std::left
                              << std::setw(22) << UI::displayTimestamp(t.timestamp)
                              << std::setw(12) << t.type
                              << std::setw(16) << t.asset
                              << std::setw(18) << std::fixed << std::setprecision(8) << t.amount
                              << std::setw(20) << std::fixed << std::setprecision(8) << t.value
                              << std::endl;
                }
            }

            if (!found)
                UI::printError("No trades found for this user.");
        }
    }
}

// Activity summary (user scoped)
void TransactionManager::showSummaryMenu()
{
    UI::printTitle("ACTIVITY SUMMARY");

    bool hasAny = false;
    std::string earliest;
    std::string latest;

    for (const auto &t : transactions)
    {
        if (t.username != user.username)
            continue;

        if (!hasAny)
        {
            earliest = t.timestamp;
            latest = t.timestamp;
            hasAny = true;
        }
        else
        {
            if (t.timestamp < earliest)
                earliest = t.timestamp;
            if (t.timestamp > latest)
                latest = t.timestamp;
        }
    }

    if (!hasAny)
    {
        UI::printError("No transactions available for this user.");
        UI::printDivider();
        return;
    }

    std::cout << "TIMEFRAME: " << UI::displayTimestamp(earliest)
              << " -> " << UI::displayTimestamp(latest) << std::endl;
    UI::printDivider();

    // Placements only
    int asksPlaced = 0;
    int bidsPlaced = 0;

    int deposits = 0;
    int withdrawals = 0;

    // Totals are tracked per quote currency (USDT vs BTC)
    double spentUSDT = 0.0;
    double spentBTC = 0.0;
    double receivedUSDT = 0.0;
    double receivedBTC = 0.0;

    double totalDeposited = 0.0;
    double totalWithdrew = 0.0;

    std::map<std::string, int> bidsPerProduct;
    std::map<std::string, int> asksPerProduct;

    std::map<std::string, double> spentPerProduct;
    std::map<std::string, double> receivedPerProduct;

    std::map<std::string, double> bidPlacementValueByKey;
    std::map<std::string, double> askPlacementValueByKey;

    std::set<std::string> hasBidPlacementKey;
    std::set<std::string> hasAskPlacementKey;

    std::map<std::string, double> matchedBuyValueByKey;
    std::map<std::string, double> matchedSellValueByKey;

    std::set<std::string> hasMatchedBuyKey;
    std::set<std::string> hasMatchedSellKey;

    auto getQuoteCurrency = [](const std::string &product) -> std::string
    {
        const std::size_t slashPos = product.find('/');
        if (slashPos == std::string::npos)
            return "";
        return product.substr(slashPos + 1);
    };

    auto addToCurrencyTotal = [&](double amount, const std::string &quote,
                                  double &usdtTotal, double &btcTotal)
    {
        if (quote == "BTC")
            btcTotal += amount;
        else
            usdtTotal += amount; // default to USDT for anything else
    };

    auto printAmount = [&](double amount, const std::string &quote)
    {
        std::ios oldState(nullptr);
        oldState.copyfmt(std::cout);

        if (quote == "BTC")
            std::cout << std::fixed << std::setprecision(8) << amount << " BTC";
        else
            std::cout << std::fixed << std::setprecision(6) << amount << " USDT";

        std::cout.copyfmt(oldState);
    };

    // Pass 1: gather placements + deposits/withdrawals
    for (const auto &t : transactions)
    {
        if (t.username != user.username)
            continue;

        const std::string key = t.timestamp + "|" + t.asset;

        if (t.type == "ask")
        {
            asksPlaced++;
            asksPerProduct[t.asset]++;

            hasAskPlacementKey.insert(key);
            askPlacementValueByKey[key] += t.value;
        }
        else if (t.type == "bid")
        {
            bidsPlaced++;
            bidsPerProduct[t.asset]++;

            hasBidPlacementKey.insert(key);
            bidPlacementValueByKey[key] += t.value;
        }
        else if (t.type == "deposit")
        {
            deposits++;
            if (t.asset == "USDT")
                totalDeposited += t.amount;
        }
        else if (t.type == "withdraw")
        {
            withdrawals++;
            if (t.asset == "USDT")
                totalWithdrew += t.amount;
        }
    }

    // Pass 2: gather matched/trade values
    for (const auto &t : transactions)
    {
        if (t.username != user.username)
            continue;

        if (t.type != "matched" && t.type != "trade")
            continue;

        const std::string key = t.timestamp + "|" + t.asset;

        const bool hasBid = (hasBidPlacementKey.count(key) > 0);
        const bool hasAsk = (hasAskPlacementKey.count(key) > 0);

        if (hasBid && !hasAsk)
        {
            hasMatchedBuyKey.insert(key);
            matchedBuyValueByKey[key] += t.value;
        }
        else if (hasAsk && !hasBid)
        {
            hasMatchedSellKey.insert(key);
            matchedSellValueByKey[key] += t.value;
        }
    }

    // Compute SPENT (buys) per key:
    for (const auto &pair : bidPlacementValueByKey)
    {
        const std::string &key = pair.first;
        const double placementValue = pair.second;

        const std::size_t barPos = key.find('|');
        const std::string product = (barPos == std::string::npos) ? "" : key.substr(barPos + 1);
        const std::string quote = getQuoteCurrency(product);

        const double finalSpend =
            (hasMatchedBuyKey.count(key) > 0) ? matchedBuyValueByKey[key] : placementValue;

        addToCurrencyTotal(finalSpend, quote, spentUSDT, spentBTC);
        spentPerProduct[product] += finalSpend;
    }

    // Compute RECEIVED (sells) per key:
    for (const auto &pair : askPlacementValueByKey)
    {
        const std::string &key = pair.first;
        const double placementValue = pair.second;

        const std::size_t barPos = key.find('|');
        const std::string product = (barPos == std::string::npos) ? "" : key.substr(barPos + 1);
        const std::string quote = getQuoteCurrency(product);

        const double finalReceive =
            (hasMatchedSellKey.count(key) > 0) ? matchedSellValueByKey[key] : placementValue;

        addToCurrencyTotal(finalReceive, quote, receivedUSDT, receivedBTC);
        receivedPerProduct[product] += finalReceive;
    }

    std::cout << "Asks placed: " << asksPlaced << std::endl;
    std::cout << "Bids placed: " << bidsPlaced << std::endl;

    // Totals (USDT and BTC labeled separately)
    std::cout << "Total money spent: ";
    if (spentUSDT != 0.0)
    {
        printAmount(spentUSDT, "USDT");
        if (spentBTC != 0.0)
            std::cout << " | ";
    }
    if (spentBTC != 0.0)
        printAmount(spentBTC, "BTC");
    if (spentUSDT == 0.0 && spentBTC == 0.0)
        std::cout << "0 USDT";
    std::cout << std::endl;

    std::cout << "Total money received: ";
    if (receivedUSDT != 0.0)
    {
        printAmount(receivedUSDT, "USDT");
        if (receivedBTC != 0.0)
            std::cout << " | ";
    }
    if (receivedBTC != 0.0)
        printAmount(receivedBTC, "BTC");
    if (receivedUSDT == 0.0 && receivedBTC == 0.0)
        std::cout << "0 USDT";
    std::cout << std::endl;

    std::cout << "Net change from trading: ";
    const double netUSDT = receivedUSDT - spentUSDT;
    const double netBTC = receivedBTC - spentBTC;

    printAmount(netUSDT, "USDT");
    if (netBTC != 0.0)
    {
        std::cout << " | ";
        printAmount(netBTC, "BTC");
    }
    std::cout << std::endl;

    UI::printDivider();

    std::cout << "Deposits: " << deposits << " (total " << totalDeposited << " USDT)" << std::endl;
    std::cout << "Withdrawals: " << withdrawals << " (total " << totalWithdrew << " USDT)" << std::endl;

    UI::printDivider();
    std::cout << "----- Per Product -----" << std::endl;

    std::set<std::string> products;
    for (auto &p : bidsPerProduct)
        products.insert(p.first);
    for (auto &p : asksPerProduct)
        products.insert(p.first);

    for (const auto &product : products)
    {
        const std::string quote = getQuoteCurrency(product);

        std::cout << product << ": "
                  << bidsPerProduct[product] << " bids, "
                  << asksPerProduct[product] << " asks, "
                  << "spent ";
        printAmount(spentPerProduct[product], quote);

        std::cout << ", received ";
        printAmount(receivedPerProduct[product], quote);

        std::cout << std::endl;
    }

    UI::printDivider();
}

bool TransactionManager::hasSimulatedTradesForUser(const std::string &username)
{
    int count = 0;

    for (const auto &t : transactions)
    {
        if (t.username != username)
            continue;

        // Simulation uses current system timestamps
        bool isOrder = (t.type == "bid" || t.type == "ask");
        bool isSimTimestamp = (t.timestamp.size() >= 4 && t.timestamp.substr(0, 4) != "2020");

        if (isOrder && isSimTimestamp)
        {
            count++;
            if (count >= 5) // at least five simulated orders
                return true;
        }
    }
    return false;
}

std::vector<OrderBookEntry>
TransactionManager::getTradesAsOrderBookEntries(const std::string &username)
{
    std::vector<OrderBookEntry> result;

    for (const auto &t : transactions)
    {
        if (t.username != username)
            continue;

        // Simulation uses current system dates
        if (t.timestamp.size() < 4 || t.timestamp.substr(0, 4) == "2020")
            continue;

        // Current build that logs simulated activity as "bid" / "ask"
        bool isSimRecord = (t.type == "trade" || t.type == "bid" || t.type == "ask");
        if (!isSimRecord)
            continue;

        if (t.amount <= 0.0)
            continue;

        double price = t.value / t.amount;
        if (price <= 0.0)
            continue;

        // Tiny synthetic spread so ASK/BID candle series don't collapse.
        double sp = price * 0.001; // 0.1%
        double askPrice = price - sp;
        double bidPrice = price + sp;

        if (askPrice <= 0.0)
            askPrice = price;

        OrderBookEntry bidEntry(
            bidPrice,
            t.amount,
            t.timestamp,
            t.asset,
            OrderBookType::bid,
            t.username);

        OrderBookEntry askEntry(
            askPrice,
            t.amount,
            t.timestamp,
            t.asset,
            OrderBookType::ask,
            t.username);

        result.push_back(bidEntry);
        result.push_back(askEntry);
    }

    return result;
}
