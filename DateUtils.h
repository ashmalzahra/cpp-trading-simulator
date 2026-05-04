#pragma once
#include <string>

namespace DateUtils
{
    std::string normalize(const std::string &timestamp);

    std::string extractYear(const std::string &timestamp);
    std::string extractMonth(const std::string &timestamp);
    std::string extractDay(const std::string &timestamp);

    bool isWithinRange(const std::string &date, const std::string &start, const std::string &end);
}

namespace DateUtils
{
    bool isValidUserDate(const std::string &s);
    std::string toDashedDate(std::string s); // converts '/' to '-' for internal comparisons
}
