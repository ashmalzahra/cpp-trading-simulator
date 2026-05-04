#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include "Wallet.h"
#include "User.h"

struct Transaction
{
    std::string username;
    std::string type;
    std::string asset;
    double amount;
    double value;
    std::string timestamp;
};

class TransactionManager
{
public:
    TransactionManager(Wallet &w, User &u);

    void logTransaction(const std::string &type,
                        const std::string &asset,
                        double amount,
                        double value,
                        const std::string &timestamp);

    void loadFromCSV();
    void saveToCSV();

    void showMenu(const std::string &marketTime);

    // Wallet actions
    void showWalletBalance();
    void depositFunds();
    void withdrawFunds();

    // Transactions
    void showTransactions();

    // Summaries
    void showSummaryMenu();
    void showSummaryByTimeframe();

    bool hasSimulatedTradesForUser(const std::string &username);
    std::vector<OrderBookEntry> getTradesAsOrderBookEntries(const std::string &username);

private:
    Wallet &wallet;
    User &user; // Reference so every transaction is tied to the logged-in username
    std::vector<Transaction> transactions;

    const std::string filename = "transactions.csv";
    std::string menuMarketTime;

    void printTransactionHeader();
};
