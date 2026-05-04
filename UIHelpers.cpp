#include "UIHelpers.h"
#include <chrono>
#include <ctime>

namespace UI
{
    void printTitle(const std::string &title)
    {
        int width = 60;
        int padding = (width - title.size()) / 2;
        std::cout << std::string(padding, '=') << " " << title << " "
                  << std::string(padding, '=') << std::endl;
    }

    void printDivider()
    {
        std::cout << std::string(60, '-') << std::endl;
    }

    int readIntInRange(int min, int max)
    {
        // Keep prompting until a valid integer choice in the allowed range
        // Using cin.clear() + ignore() to prevent the input stream from staying in a failed state
        int value;
        while (true)
        {
            std::cout << "Enter choice (" << min << "-" << max << "): " << std::endl;

            if (std::cin >> value)
            {
                if (value >= min && value <= max)
                {
                    // Discard the rest of the line so later getline() calls don't immediately return an empty string
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    return value;
                }
            }

            printError("Invalid input. Enter number between " + std::to_string(min) + " and " + std::to_string(max));
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    std::string readString(const std::string &prompt)
    {
        if (!prompt.empty())
            std::cout << prompt << std::endl;

        std::string input;
        // std::ws consumes any leftover whitespace/newlines from previous numeric reads
        std::getline(std::cin >> std::ws, input);
        return input;
    }

    void printError(const std::string &msg)
    {
        std::cout << "[ERROR] " << msg << std::endl;
    }

    void printSuccess(const std::string &msg)
    {
        std::cout << "[OK] " << msg << std::endl;
    }
}

double UI::readDouble()
{
    while (true)
    {
        std::string line;
        std::getline(std::cin, line);
        try
        {
            // Parse from a full line so bad numeric input doesn't break the stream state
            return std::stod(line);
        }
        catch (...)
        {
            UI::printError("Invalid number. Try again:");
        }
    }
}

std::string UI::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_t = std::chrono::system_clock::to_time_t(now);

    char buffer[32];
    // string ordering matches chronological ordering for comparisons
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_t));

    return std::string(buffer);
}

std::string UI::displayTimestamp(std::string ts)
{
    // Consistent display
    for (char &c : ts)
        if (c == '-')
            c = '/';

    // Strip microseconds if present
    size_t dot = ts.find('.');
    if (dot != std::string::npos)
        ts = ts.substr(0, dot);

    return ts;
}
