#include "MerkelMain.h"
#include <iostream>
#include <vector>
#include <set>
#include <iomanip>
#include <random>
#include <chrono>
#include <functional>
#include <algorithm>

#include "OrderBook.h"
#include "OrderBookEntry.h"
#include "CSVReader.h"
#include "Candlestick.h"
#include "CandlestickCalculator.h"
#include "UIHelpers.h"
#include "DateUtils.h"
#include "TransactionManager.h"

// Constructor
MerkelMain::MerkelMain()
{
}

void MerkelMain::loginMenu()
{
    // Keep showing the login menu until a valid login occurs
    while (!isLoggedIn)
    {
        UI::printTitle("MERKLEREX LOGIN");

        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;
        std::cout << "3. Reset Password" << std::endl;
        std::cout << "4. Recover Username" << std::endl;
        std::cout << "5. Exit" << std::endl;
        UI::printDivider();

        int choice = UI::readIntInRange(1, 5);

        if (choice == 1)
            loginUserFlow();
        else if (choice == 2)
            registerUserFlow();
        else if (choice == 3)
            resetPasswordFlow();
        else if (choice == 4)
            recoverUsernameFlow();
        else if (choice == 5)
            exit(0);
    }
}

// Initialisation
void MerkelMain::init()
{
    loginMenu(); // User must authenticate before any wallet/trading actions

    currentTime = orderBook.getEarliestTime();

    UI::printTitle("WELCOME TO MERKLEREX");
    bool running = true;

    while (running)
    {
        printMenu();
        int option = UI::readIntInRange(1, 10);
        processUserOption(option, running);
    }
}

// User Login
void MerkelMain::loginUserFlow()
{
    UI::printTitle("LOGIN");

    std::cout << "Username: ";
    std::string username = UI::readString();

    std::cout << "Password: ";
    std::string password = UI::readString();

    if (userManager.loginUser(username, password, currentUser))
    {
        isLoggedIn = true;

        // Load this user's wallet
        wallet.loadFromCSV(currentUser.username);

        UI::printTitle("LOGIN SUCCESS");
        std::cout << "Welcome, " << currentUser.fullName << "!" << std::endl;
        UI::printDivider();
    }
    else
    {
        UI::printError("Incorrect username or password.");
    }
}

void MerkelMain::registerUserFlow()
{
    UI::printTitle("REGISTER USER");

    std::cout << "Full name: ";
    std::string name = UI::readString();

    std::cout << "Email: ";
    std::string email = UI::readString();

    std::cout << "Password: ";
    std::string password = UI::readString();

    if (!userManager.isValidEmail(email))
    {
        UI::printError("Invalid email format.");
        return;
    }

    std::string newUsername;

    if (userManager.registerUser(name, email, password, newUsername))
    {
        UI::printSuccess("Registration successful!");
        std::cout << "Your username: " << newUsername << std::endl;
        UI::printDivider();

        // Auto-login after successful registration
        currentUser.username = newUsername;
        currentUser.fullName = name;
        currentUser.email = email;
        isLoggedIn = true;

        // Set wallet user context and ensure this new user appears in wallets csv
        wallet.loadFromCSV(newUsername); // set currentUsername internally
        wallet.saveToCSV();
    }
    else
    {
        UI::printError("A user with this name and email already exists.");
    }
}

void MerkelMain::resetPasswordFlow()
{
    UI::printTitle("RESET PASSWORD");

    std::cout << "Enter your registered email: ";
    std::string email = UI::readString();

    std::cout << "Enter new password: ";
    std::string newPass = UI::readString();

    if (userManager.resetPassword(email, newPass))
        UI::printSuccess("Password updated!");
    else
        UI::printError("Email not found.");
}

void MerkelMain::recoverUsernameFlow()
{
    UI::printTitle("RECOVER USERNAME");

    std::cout << "Enter your registered email: ";
    std::string email = UI::readString();

    for (auto &u : userManager.getUsers())
    {
        if (u.email == email)
        {
            UI::printSuccess("Account found!");
            std::cout << "Your username is: " << u.username << std::endl;
            UI::printDivider();
            return;
        }
    }

    UI::printError("No user found with this email.");
}

void MerkelMain::printMenu()
{
    UI::printTitle("MERKLEREX MAIN MENU");

    std::cout << "1: Print help" << std::endl;
    std::cout << "2: Print exchange stats" << std::endl;
    std::cout << "3: Make an offer (ask)" << std::endl;
    std::cout << "4: Make a bid" << std::endl;
    std::cout << "5: Print wallet" << std::endl;
    std::cout << "6: Continue to next TimeFrame" << std::endl;
    std::cout << "7: Show Candlestick summary" << std::endl;
    std::cout << "8: Wallet & Transactions Menu" << std::endl;
    std::cout << "9: Simulate trading activity " << std::endl;
    std::cout << "10: Exit program" << std::endl;

    UI::printDivider();
    std::cout << "Current time is: " << UI::displayTimestamp(currentTime) << std::endl;
    UI::printDivider();
}

void MerkelMain::printHelp()
{
    UI::printTitle("HELP");
    std::cout << "Your aim is to make money." << std::endl;
    std::cout << "Analyse the market and make bids and offers." << std::endl;
    UI::printDivider();
}

void MerkelMain::printMarketStats()
{
    UI::printTitle("EXCHANGE STATS");

    for (const std::string &product : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << product << std::endl;
        UI::printDivider();

        // Asks
        std::vector<OrderBookEntry> asks =
            orderBook.getOrders(OrderBookType::ask, product, currentTime);

        std::cout << "Asks seen:  " << asks.size() << std::endl;

        if (!asks.empty())
        {
            double maxAsk = orderBook.getHighPrice(asks);
            double minAsk = orderBook.getLowPrice(asks);

            std::cout << "Max ask:    " << maxAsk << std::endl;
            std::cout << "Min ask:    " << minAsk << std::endl;
        }
        else
        {
            std::cout << "Max ask:    N/A" << std::endl;
            std::cout << "Min ask:    N/A" << std::endl;
        }

        // Bids
        std::vector<OrderBookEntry> bids =
            orderBook.getOrders(OrderBookType::bid, product, currentTime);

        std::cout << "Bids seen:  " << bids.size() << std::endl;

        if (!bids.empty())
        {
            double maxBid = orderBook.getHighPrice(bids);
            double minBid = orderBook.getLowPrice(bids);

            std::cout << "Max bid:    " << maxBid << std::endl;
            std::cout << "Min bid:    " << minBid << std::endl;
        }
        else
        {
            std::cout << "Max bid:    N/A" << std::endl;
            std::cout << "Min bid:    N/A" << std::endl;
        }

        UI::printDivider();
        std::cout << std::endl;
    }
}

OrderBookEntry MerkelMain::collectOrderDetails(OrderBookType type)
{
    UI::printTitle(type == OrderBookType::ask ? "CREATE ASK ORDER" : "CREATE BID ORDER");

    auto products = orderBook.getKnownProducts();

    std::cout << "Select a product:" << std::endl;
    for (int i = 0; i < products.size(); i++)
        std::cout << (i + 1) << ": " << products[i] << std::endl;

    UI::printDivider();

    int pChoice = UI::readIntInRange(1, products.size());
    std::string product = products[pChoice - 1];

    double price, amount;

    std::cout << "Enter price: ";
    price = UI::readDouble();

    std::cout << "Enter amount: ";
    amount = UI::readDouble();

    OrderBookEntry obe =
        // reusing the existing OBE
        CSVReader::stringsToOBE(
            std::to_string(price),
            std::to_string(amount),
            currentTime,
            product,
            type);

    obe.username = currentUser.username;
    return obe;
}

void MerkelMain::showOrderSummary(const OrderBookEntry &obe)
{
    UI::printTitle("ORDER SUMMARY");

    std::cout << "Type:    "
              << (obe.orderType == OrderBookType::ask ? "ASK" : "BID") << std::endl;

    std::cout << "Product: " << obe.product << std::endl;
    std::cout << "Price:   " << std::fixed << std::setprecision(8) << obe.price << std::endl;
    std::cout << "Amount:  " << obe.amount << std::endl;
    std::cout << "Time:    " << UI::displayTimestamp(obe.timestamp) << std::endl;

    UI::printDivider();
    UI::printSuccess("Order created successfully!");
    UI::printDivider();
}

void MerkelMain::enterAsk()
{
    OrderBookEntry obe = collectOrderDetails(OrderBookType::ask);

    UI::printDivider();
    std::cout << "Checking wallet..." << std::endl;

    // Validate the order against the wallet before placing it
    if (wallet.canFulfillOrder(obe))
    {
        std::cout << "Wallet looks good." << std::endl;
        orderBook.insertOrder(obe);

        // Log Ask placement
        transactionManager.logTransaction(
            "ask",
            obe.product,
            obe.amount,
            obe.amount * obe.price,
            obe.timestamp);

        wallet.saveToCSV();

        showOrderSummary(obe);
    }
    else
    {
        UI::printError("Wallet has insufficient funds.");
    }
}

void MerkelMain::enterBid()
{
    OrderBookEntry obe = collectOrderDetails(OrderBookType::bid);

    UI::printDivider();
    std::cout << "Checking wallet..." << std::endl;

    if (wallet.canFulfillOrder(obe))
    {
        std::cout << "Wallet looks good." << std::endl;
        orderBook.insertOrder(obe);

        // Log Bid placement
        transactionManager.logTransaction(
            "bid",
            obe.product,
            obe.amount,
            obe.amount * obe.price, // potential cost
            obe.timestamp);

        wallet.saveToCSV();

        showOrderSummary(obe);
    }
    else
    {
        UI::printError("Wallet has insufficient funds.");
    }
}

void MerkelMain::printWallet()
{
    UI::printTitle("WALLET");
    std::cout << wallet.toString() << std::endl;
    UI::printDivider();
}

// Process Sale
void MerkelMain::gotoNextTimeframe()
{
    UI::printTitle("ADVANCE TO NEXT TIMEFRAME");

    std::cout << "Processing market activity at time:" << std::endl;
    std::cout << UI::displayTimestamp(currentTime) << std::endl;
    UI::printDivider();

    for (const std::string &p : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << p << std::endl;

        std::vector<OrderBookEntry> sales =
            orderBook.matchAsksToBids(p, currentTime);

        std::cout << "Matched trades: " << sales.size() << std::endl;

        for (OrderBookEntry &sale : sales)
        {
            std::cout << "  - Price: "
                      << std::fixed << std::setprecision(8)
                      << sale.price
                      << " | Amount: "
                      << sale.amount
                      << std::endl;

            // Apply executed trade results to this user wallet balance
            if (sale.username == currentUser.username)
            {
                wallet.processSale(sale);

                transactionManager.logTransaction(
                    "matched",
                    sale.product,
                    sale.amount,
                    sale.amount * sale.price,
                    sale.timestamp);
            }
        }

        UI::printDivider();
    }

    wallet.saveToCSV();

    currentTime = orderBook.getNextTime(currentTime);

    UI::printSuccess("Timeframe advanced successfully.");
    UI::printDivider();
    std::cout << "Current time is now: " << UI::displayTimestamp(currentTime) << std::endl;
    UI::printDivider();
}

// Candlestick view
void MerkelMain::showCandlesticks()
{
    while (true)
    {
        // Product selection
        auto products = orderBook.getKnownProducts();
        UI::printTitle("AVAILABLE PRODUCTS");

        for (int i = 0; i < (int)products.size(); i++)
        {
            std::cout << (i + 1) << ": " << products[i] << std::endl;
        }

        UI::printDivider();
        int pChoice = UI::readIntInRange(1, products.size());
        std::string product = products[pChoice - 1];

        // Gather orders
        std::vector<OrderBookEntry> all = orderBook.getAllOrders();
        // Include user trade history so candles reflect executed trades too
        auto userTrades = transactionManager.getTradesAsOrderBookEntries(currentUser.username);
        all.insert(all.end(), userTrades.begin(), userTrades.end());

        std::vector<OrderBookEntry> asks, bids;

        for (auto &e : all)
        {
            if (e.product == product)
            {
                if (e.orderType == OrderBookType::ask)
                    asks.push_back(e);
                else if (e.orderType == OrderBookType::bid)
                    bids.push_back(e);
            }
        }

        // Default yearly summary
        auto yearlyAsks = CandlestickCalculator::computeYearlyCandles(asks);
        auto yearlyBids = CandlestickCalculator::computeYearlyCandles(bids);

        UI::printTitle("YEARLY SUMMARY (" + product + ")");

        std::cout << "ASK Candlesticks (Yearly)" << std::endl;
        UI::printDivider();

        std::cout << std::left
                  << std::setw(14) << "Year"
                  << std::setw(18) << "Open"
                  << std::setw(18) << "High"
                  << std::setw(18) << "Low"
                  << std::setw(18) << "Close"
                  << std::endl;

        for (auto &c : yearlyAsks)
            c.displayRow();

        std::cout << std::endl
                  << "BID Candlesticks (Yearly)" << std::endl;
        UI::printDivider();

        std::cout << std::left
                  << std::setw(14) << "Year"
                  << std::setw(18) << "Open"
                  << std::setw(18) << "High"
                  << std::setw(18) << "Low"
                  << std::setw(18) << "Close"
                  << std::endl;

        for (auto &c : yearlyBids)
            c.displayRow();

        UI::printDivider();

        // Filter menu
        while (true)
        {
            std::cout << "Additional Options:" << std::endl;
            std::cout << "1. View Daily Summary" << std::endl;
            std::cout << "2. View Monthly Summary" << std::endl;
            std::cout << "3. View Yearly Summary" << std::endl;
            std::cout << "4. View Summary by Date Range" << std::endl;
            std::cout << "5. Change Product" << std::endl;
            std::cout << "6. Exit Candlestick View" << std::endl;

            UI::printDivider();
            int choice = UI::readIntInRange(1, 6);

            if (choice == 6)
                return;
            if (choice == 5)
                break;

            // Daily summary
            if (choice == 1)
            {
                auto dA = CandlestickCalculator::computeDailyCandles(asks);
                auto dB = CandlestickCalculator::computeDailyCandles(bids);

                UI::printTitle("DAILY SUMMARY (" + product + ")");

                std::cout << "ASK (Daily)" << std::endl;
                UI::printDivider();
                std::cout << std::left
                          << std::setw(14) << "Date"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : dA)
                    c.displayRow();

                std::cout << std::endl
                          << "BID (Daily)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Date"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : dB)
                    c.displayRow();

                UI::printDivider();
            }

            // Monthly summary
            if (choice == 2)
            {
                auto mA = CandlestickCalculator::computeMonthlyCandles(asks);
                auto mB = CandlestickCalculator::computeMonthlyCandles(bids);

                UI::printTitle("MONTHLY SUMMARY (" + product + ")");

                std::cout << "ASK (Monthly)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Month"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : mA)
                    c.displayRow();

                std::cout << std::endl
                          << "BID (Monthly)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Month"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : mB)
                    c.displayRow();

                UI::printDivider();
            }

            // Yearly again
            if (choice == 3)
            {
                UI::printTitle("YEARLY SUMMARY (" + product + ")");

                std::cout << "ASK (Yearly)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Year"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : yearlyAsks)
                    c.displayRow();

                std::cout << std::endl
                          << "BID (Yearly)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Year"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : yearlyBids)
                    c.displayRow();

                UI::printDivider();
            }

            // Range filter
            if (choice == 4)
            {
                // Determine min/max day available so the user can only choose valid ranges
                std::string minDate = "9999-99-99";
                std::string maxDate = "0000-00-00";

                for (auto &e : asks)
                {
                    std::string d = DateUtils::extractDay(e.timestamp);
                    if (d < minDate)
                        minDate = d;
                    if (d > maxDate)
                        maxDate = d;
                }

                UI::printTitle("DATE RANGE FILTER");
                std::cout << "Available range: [" << minDate << " - " << maxDate << "]" << std::endl;
                UI::printDivider();

                std::string start, end;

                // Keep asking until valid date input is provided
                while (true)
                {
                    start = UI::readString("Enter start date (YYYY-MM-DD):");
                    if (!DateUtils::isValidUserDate(start))
                    {
                        UI::printError("Invalid start date format. Example: 2020-03-17");
                        continue;
                    }

                    end = UI::readString("Enter end date (YYYY-MM-DD):");
                    if (!DateUtils::isValidUserDate(end))
                    {
                        UI::printError("Invalid end date format. Example: 2020-03-17");
                        continue;
                    }
                    // Enforce ordering and bounds so a blank table isn't displayed due to invalid input
                    if (end < start)
                    {
                        UI::printError("End date must be the same as or after the start date.");
                        continue;
                    }

                    if (start < minDate || end > maxDate)
                    {
                        UI::printError("Date out of available range. Choose within [" + minDate + " - " + maxDate + "].");
                        continue;
                    }

                    break;
                }

                std::vector<OrderBookEntry> fA, fB;

                for (auto &e : asks)
                {
                    std::string d = DateUtils::extractDay(e.timestamp);
                    if (DateUtils::isWithinRange(d, start, end))
                        fA.push_back(e);
                }

                for (auto &e : bids)
                {
                    std::string d = DateUtils::extractDay(e.timestamp);
                    if (DateUtils::isWithinRange(d, start, end))
                        fB.push_back(e);
                }

                auto fdA = CandlestickCalculator::computeDailyCandles(fA);
                auto fdB = CandlestickCalculator::computeDailyCandles(fB);

                UI::printTitle("FILTERED RANGE (" + product + ")");

                std::cout << "ASK (Filtered)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Date"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : fdA)
                    c.displayRow();

                std::cout << std::endl
                          << "BID (Filtered)" << std::endl;
                UI::printDivider();

                std::cout << std::left
                          << std::setw(14) << "Date"
                          << std::setw(18) << "Open"
                          << std::setw(18) << "High"
                          << std::setw(18) << "Low"
                          << std::setw(18) << "Close"
                          << std::endl;

                for (auto &c : fdB)
                    c.displayRow();

                // If both are empty, tell the user why they see no rows
                if (fdA.empty() && fdB.empty())
                {
                    UI::printError("No trading data found for the selected date range.");
                }

                UI::printDivider();
            }
        }
    }
}

void MerkelMain::simulateTradingActivity()
{
    UI::printTitle("SIMULATING TRADING ACTIVITY");

    // Prevent accidental duplication of simulation trades unless the user explicitly confirms
    if (transactionManager.hasSimulatedTradesForUser(currentUser.username))
    {
        std::cout << "Trading simulation has already been executed for this user." << std::endl;
        std::cout << "Do you want to run the simulation again? (y/n): ";

        std::string choice = UI::readString();
        if (choice != "y" && choice != "Y")
        {
            UI::printError("Simulation cancelled by user.");
            return;
        }

        UI::printDivider();
    }

    std::vector<std::string> products = orderBook.getKnownProducts();

    auto quoteRank = [&](const std::string &product) -> int
    {
        auto parts = CSVReader::tokenise(product, '/');
        if (parts.size() != 2)
            return 99;
        const std::string &quote = parts[1];
        if (quote == "USDT")
            return 0;
        if (quote == "BTC")
            return 1;
        return 2;
    };

    std::stable_sort(products.begin(), products.end(),
                     [&](const std::string &a, const std::string &b)
                     {
                         return quoteRank(a) < quoteRank(b);
                     });

    std::string now = UI::currentTimestamp();
    std::string baseDate = now.substr(0, 10);
    std::string timePart = now.substr(11, 8);

    // RNG the same trades aren't simulated every run
    static unsigned long long runCounter = 0;
    runCounter++;

    unsigned long long seed =
        (unsigned long long)std::chrono::high_resolution_clock::now().time_since_epoch().count() ^ (unsigned long long)std::hash<std::string>{}(currentUser.username) ^ (unsigned long long)std::hash<std::string>{}(currentTime) ^ runCounter;

    std::mt19937 rng((unsigned int)(seed & 0xffffffffULL));
    std::uniform_real_distribution<double> noisePct(-0.006, 0.006);
    std::uniform_real_distribution<double> amtScale(0.8, 1.4); // amount multiplier
    std::uniform_real_distribution<double> spreadPct(0.0005, 0.0015);

    auto addDays = [&](const std::string &ymd, int add) -> std::string
    {
        int y = std::stoi(ymd.substr(0, 4));
        int m = std::stoi(ymd.substr(5, 2));
        int d = std::stoi(ymd.substr(8, 2));

        std::tm t{};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d + add;
        std::mktime(&t);

        char buf[16];
        std::snprintf(buf, sizeof(buf), "%04d/%02d/%02d",
                      t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        return std::string(buf);
    };

    auto two = [&](int x) -> std::string
    {
        return (x < 10 ? "0" : "") + std::to_string(x);
    };

    // Offset seconds to make multiple entries per day
    int baseHH = std::stoi(timePart.substr(0, 2));
    int baseMM = std::stoi(timePart.substr(3, 2));
    int baseSS = std::stoi(timePart.substr(6, 2));
    int baseTotalSec = baseHH * 3600 + baseMM * 60 + baseSS;

    const int daysToGenerate = 5;
    const int ticksPerDay = 2;

    int totalExecuted = 0;
    int totalSkipped = 0;
    int totalAdjusted = 0;

    for (const auto &product : products)
    {
        auto parts = CSVReader::tokenise(product, '/');
        if (parts.size() != 2)
            continue;

        std::string base = parts[0];
        std::string quote = parts[1];

        // Use dataset at currentTime to estimate market level + trend
        auto asksNow = orderBook.getOrders(OrderBookType::ask, product, currentTime);
        auto bidsNow = orderBook.getOrders(OrderBookType::bid, product, currentTime);

        double marketNow =
            !asksNow.empty() ? OrderBook::getLowPrice(asksNow)
                             : (!bidsNow.empty() ? OrderBook::getHighPrice(bidsNow) : 100.0);

        std::string nextT = orderBook.getNextTime(currentTime);

        auto asksNext = orderBook.getOrders(OrderBookType::ask, product, nextT);
        auto bidsNext = orderBook.getOrders(OrderBookType::bid, product, nextT);

        double marketNext =
            !asksNext.empty() ? OrderBook::getLowPrice(asksNext)
                              : (!bidsNext.empty() ? OrderBook::getHighPrice(bidsNext) : marketNow);

        // Trend drives drift across days (clamped)
        double trend = 0.0;
        if (marketNow > 0.0)
            trend = (marketNext - marketNow) / marketNow;
        trend = std::clamp(trend, -0.03, 0.03);

        // Base trade size by asset
        double baseAmt = 0.1;
        if (base == "BTC")
            baseAmt = 0.005;
        if (base == "DOGE")
            baseAmt = 150.0;

        int executedForProduct = 0;
        int skippedForProduct = 0;
        int adjustedForProduct = 0;

        for (int i = 0; i < daysToGenerate; i++)
        {
            std::string day = addDays(baseDate, i);
            double dayDrift = trend * (0.8 + 0.25 * i);

            for (int j = 0; j < ticksPerDay; j++)
            {
                // Offset seconds within the same day
                int offset = j * 17; // 0, 17 seconds
                int totalSec = (baseTotalSec + offset) % 86400;

                int hh = totalSec / 3600;
                int mm = (totalSec % 3600) / 60;
                int ss = totalSec % 60;

                std::string ts = day + " " + two(hh) + ":" + two(mm) + ":" + two(ss);

                // Desired amount
                double amount = baseAmt * (1.0 + 0.12 * i) * (0.95 + 0.10 * j) * amtScale(rng);

                // Mid price follows trend + noise
                double mid = marketNow * (1.0 + dayDrift + noisePct(rng));
                if (mid <= 0.0)
                    mid = marketNow;

                double sp = mid * spreadPct(rng);
                double marketBid = mid - sp;
                double marketAsk = mid + sp;
                if (marketBid <= 0.0)
                    marketBid = mid;

                // Buys and sells across ticks
                bool wantBuy = ((i + j) % 2 == 0);

                double quoteBal = wallet.getBalance(quote);
                double baseBal = wallet.getBalance(base);

                bool canBuy = (quoteBal > 0.0);
                bool canSell = (baseBal > 0.0);

                if (wantBuy && !canBuy && canSell)
                    wantBuy = false;
                if (!wantBuy && !canSell && canBuy)
                    wantBuy = true;

                if (!canBuy && !canSell)
                {
                    std::cout << "[SKIP] " << product << " at " << ts
                              << " (no available balance to buy or sell)" << std::endl;
                    skippedForProduct++;
                    totalSkipped++;
                    continue;
                }

                if (wantBuy)
                {
                    // User will match the dataset ask
                    double execPrice = marketAsk;

                    // Max amount affordable in base units
                    double maxAmt = wallet.getBalance(quote) / execPrice;

                    if (maxAmt <= 0.0)
                    {
                        std::cout << "[SKIP] " << product << " BUY at " << ts
                                  << " (insufficient " << quote << ")" << std::endl;
                        skippedForProduct++;
                        totalSkipped++;
                        continue;
                    }
                    // Resize amount down to what the wallet can afford instead of failing outright to avoid negative balance
                    if (amount > maxAmt)
                    {
                        std::cout << "[ADJUST] " << product << " BUY at " << ts
                                  << " resized from " << amount << " to " << maxAmt
                                  << " (insufficient " << quote << ")" << std::endl;
                        amount = maxAmt * 0.999;
                        adjustedForProduct++;
                        totalAdjusted++;
                    }

                    if (amount <= 0.0)
                    {
                        skippedForProduct++;
                        totalSkipped++;
                        continue;
                    }

                    const double SIM_SPREAD = 0.001;

                    double bidPrice = execPrice * (1.0 + SIM_SPREAD);

                    OrderBookEntry userBid(bidPrice, amount, ts, product, OrderBookType::bid);
                    userBid.username = currentUser.username;

                    OrderBookEntry datasetAsk(execPrice, amount, ts, product, OrderBookType::ask);
                    datasetAsk.username = "dataset";

                    // Log the user's BID placement
                    transactionManager.logTransaction(
                        "bid",
                        product,
                        amount,
                        amount * execPrice,
                        ts);
                    orderBook.insertOrder(userBid);
                    orderBook.insertOrder(datasetAsk);

                    auto sales = orderBook.matchAsksToBids(product, ts);

                    bool userUpdated = false;
                    for (auto &sale : sales)
                    {
                        if (sale.username == currentUser.username)
                        {
                            wallet.processSale(sale);
                            userUpdated = true;
                            executedForProduct++;
                            totalExecuted++;
                        }
                    }

                    if (!userUpdated)
                    {
                        // Trade may have occurred but not attributed correctly
                        std::cout << "[SKIP] " << product << " BUY at " << ts
                                  << " (matched but not attributed to user)" << std::endl;
                        skippedForProduct++;
                        totalSkipped++;
                    }
                }
                else
                {
                    // User will match the dataset bid
                    double execPrice = marketBid;

                    double maxAmt = wallet.getBalance(base);

                    if (maxAmt <= 0.0)
                    {
                        std::cout << "[SKIP] " << product << " SELL at " << ts
                                  << " (insufficient " << base << ")" << std::endl;
                        skippedForProduct++;
                        totalSkipped++;
                        continue;
                    }

                    if (amount > maxAmt)
                    {
                        std::cout << "[ADJUST] " << product << " SELL at " << ts
                                  << " resized from " << amount << " to " << maxAmt
                                  << " (insufficient " << base << ")" << std::endl;
                        amount = maxAmt * 0.999;
                        adjustedForProduct++;
                        totalAdjusted++;
                    }

                    if (amount <= 0.0)
                    {
                        skippedForProduct++;
                        totalSkipped++;
                        continue;
                    }

                    const double SIM_SPREAD = 0.001;

                    OrderBookEntry userAsk(execPrice, amount, ts, product, OrderBookType::ask);
                    userAsk.username = currentUser.username;

                    double datasetBidPrice = execPrice * (1.0 + SIM_SPREAD);
                    OrderBookEntry datasetBid(datasetBidPrice, amount, ts, product, OrderBookType::bid);
                    datasetBid.username = "dataset";

                    // Log the user's ASK placement
                    transactionManager.logTransaction(
                        "ask",
                        product,
                        amount,
                        amount * execPrice,
                        ts);

                    orderBook.insertOrder(userAsk);
                    orderBook.insertOrder(datasetBid);

                    auto sales = orderBook.matchAsksToBids(product, ts);

                    bool userUpdated = false;
                    for (auto &sale : sales)
                    {
                        if (sale.username == currentUser.username)
                        {
                            wallet.processSale(sale);
                            userUpdated = true;
                            executedForProduct++;
                            totalExecuted++;
                        }
                    }

                    if (!userUpdated)
                    {
                        std::cout << "[SKIP] " << product << " SELL at " << ts
                                  << " (matched but not attributed to user)" << std::endl;
                        skippedForProduct++;
                        totalSkipped++;
                    }
                }
            }
        }

        UI::printDivider();
        std::cout << "Product " << product
                  << " -> Executed: " << executedForProduct
                  << " | Adjusted: " << adjustedForProduct
                  << " | Skipped: " << skippedForProduct
                  << std::endl;
        UI::printDivider();
    }

    wallet.saveToCSV();

    UI::printSuccess("Trading simulation completed successfully.");

    std::cout << "Summary: Executed trades: " << totalExecuted
              << " | Adjusted orders: " << totalAdjusted
              << " | Skipped attempts: " << totalSkipped
              << std::endl;
}

void MerkelMain::processUserOption(int userOption, bool &running)
{
    if (userOption == 1)
        printHelp();
    else if (userOption == 2)
        printMarketStats();
    else if (userOption == 3)
        enterAsk();
    else if (userOption == 4)
        enterBid();
    else if (userOption == 5)
        printWallet();
    else if (userOption == 6)
        gotoNextTimeframe();
    else if (userOption == 7)
        showCandlesticks();
    else if (userOption == 8)
        transactionManager.showMenu(currentTime);
    else if (userOption == 9)
        simulateTradingActivity();
    else if (userOption == 10)
    {
        UI::printSuccess("Exiting program. Goodbye!");
        running = false;
    }

    else
    {
        UI::printError("Invalid choice. Choose 1-10.");
    }
}