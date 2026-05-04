#pragma once
#include <string>
#include "OrderBook.h"
#include "Wallet.h"
#include "User.h"
#include "UserManager.h"
#include "TransactionManager.h"

class MerkelMain
{
public:
    MerkelMain();
    void init(); // Start login flow, then run the main menu loop

private:
    // Login system
    void loginMenu();
    void loginUserFlow();
    void registerUserFlow();
    void resetPasswordFlow();
    void recoverUsernameFlow();
    void simulateTradingActivity();

    bool isLoggedIn = false;
    User currentUser;
    UserManager userManager;
    // wallet declared before transactionManager to avoid referencing an unconstructed object
    Wallet wallet;
    TransactionManager transactionManager{wallet, currentUser};

    void printMenu();
    void processUserOption(int userOption, bool &running);

    void printHelp();
    void printMarketStats();
    void enterAsk();
    void enterBid();
    void printWallet();
    void gotoNextTimeframe();

    void showCandlesticks();
    void showOrderSummary(const OrderBookEntry &obe);

    OrderBookEntry collectOrderDetails(OrderBookType type);

    std::string currentTime;
    OrderBook orderBook{"20200601.csv"};
};
