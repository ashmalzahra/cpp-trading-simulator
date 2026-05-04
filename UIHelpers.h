#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <limits>

namespace UI
{
    // Centered title with borders
    void printTitle(const std::string &title);

    // Divider line
    void printDivider();

    // Read an integer safely within a range
    int readIntInRange(int min, int max);

    // Read a line safely
    std::string readString(const std::string &prompt = "");

    // Styled error + success messages
    void printError(const std::string &msg);
    void printSuccess(const std::string &msg);

    double readDouble();
    std::string currentTimestamp();
    std::string displayTimestamp(std::string ts);

}
