#include "DateUtils.h"
#include <string>

namespace DateUtils
{
    // Normalize date separators
    std::string normalize(const std::string &timestamp)
    {
        std::string date = timestamp.substr(0, 10);
        for (char &c : date)
            if (c == '/')
                c = '-';
        return date;
    }

    std::string extractYear(const std::string &timestamp)
    {
        return normalize(timestamp).substr(0, 4);
    }

    std::string extractMonth(const std::string &timestamp)
    {
        return normalize(timestamp).substr(0, 7);
    }

    std::string extractDay(const std::string &timestamp)
    {
        return normalize(timestamp).substr(0, 10);
    }

    bool isWithinRange(const std::string &date, const std::string &start, const std::string &end)
    {
        return date >= start && date <= end;
    }
}

namespace DateUtils
{
    bool isValidUserDate(const std::string &s)
    {
        if (s.size() != 10)
            return false;

        char sep1 = s[4], sep2 = s[7];
        if (!((sep1 == '-' && sep2 == '-') || (sep1 == '/' && sep2 == '/')))
            return false;

        for (int i : {0, 1, 2, 3, 5, 6, 8, 9})
            if (s[i] < '0' || s[i] > '9')
                return false;

        int y = std::stoi(s.substr(0, 4));
        int m = std::stoi(s.substr(5, 2));
        int d = std::stoi(s.substr(8, 2));

        if (y < 1900 || y > 2100)
            return false;
        if (m < 1 || m > 12)
            return false;
        if (d < 1 || d > 31)
            return false;

        if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30)
            return false;
        if (m == 2 && d > 29)
            return false;

        return true;
    }

    std::string toDashedDate(std::string s)
    {
        for (char &c : s)
            if (c == '/')
                c = '-';
        return s;
    }
}
