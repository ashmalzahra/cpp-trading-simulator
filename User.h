#pragma once
#include <string>

struct User
{
    std::string username; // auto-generated unique 10-digit ID
    std::string fullName;
    std::string email;
    size_t passwordHash; // hashed with std::hash<std::string>
};
