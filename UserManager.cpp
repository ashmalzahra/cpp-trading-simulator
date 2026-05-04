#include "UserManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <functional>
#include <algorithm>

std::string toLower(const std::string &s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

UserManager::UserManager(const std::string &filename)
    : filename(filename)
{
    loadUsers();
}

bool UserManager::loadUsers()
{
    users.clear();
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    std::string line;
    // Read user records from CSV into memory (username, name, email, hashed password)
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string username, fullName, email, hashStr;

        std::getline(ss, username, ',');
        std::getline(ss, fullName, ',');
        std::getline(ss, email, ',');
        std::getline(ss, hashStr, ',');

        if (!username.empty())
        {
            User u;
            u.username = username;
            u.fullName = fullName;
            u.email = email;
            // Stored hash is written as a number in the CSV, to parse it back into size_t-compatible form
            u.passwordHash = std::stoull(hashStr);
            users.push_back(u);
        }
    }
    return true;
}

bool UserManager::saveUsers()
{
    std::ofstream file(filename);
    if (!file.is_open())
        return false;

    for (auto &u : users)
    {
        file << u.username << ","
             << u.fullName << ","
             << u.email << ","
             << u.passwordHash << std::endl;
    }
    return true;
}

bool UserManager::userExists(const std::string &fullName, const std::string &email)
{
    std::string nameLower = toLower(fullName);
    std::string emailLower = toLower(email);

    // Case-insensitive duplicate check
    for (auto &u : users)
    {
        if (toLower(u.fullName) == nameLower &&
            toLower(u.email) == emailLower)
        {
            return true;
        }
    }
    return false;
}

size_t UserManager::hashPassword(const std::string &password)
{
    // Hash the password for storage/comparison
    std::hash<std::string> hasher;
    return hasher(password);
}

std::string UserManager::generateUsername()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 9);

    std::string id;
    for (int i = 0; i < 10; i++)
        id += std::to_string(dist(gen));

    return id;
}

bool UserManager::registerUser(const std::string &fullName,
                               const std::string &email,
                               const std::string &password,
                               std::string &generatedUsername)
{
    if (userExists(fullName, email))
        return false;

    if (!isValidEmail(email))
    {
        return false; // or handle error in calling function
    }

    User u;
    u.fullName = fullName;
    u.email = email;
    u.passwordHash = hashPassword(password);
    u.username = generateUsername();

    // Ensure username is unique among existing users
    while (std::any_of(users.begin(), users.end(),
                       [&](const User &existing)
                       {
                           return existing.username == u.username;
                       }))
    {
        u.username = generateUsername();
    }

    users.push_back(u);
    saveUsers();
    generatedUsername = u.username;
    return true;
}

bool UserManager::loginUser(const std::string &username,
                            const std::string &password,
                            User &loggedInUser)
{
    // Hash the input password and compare it to the stored hash value
    size_t hashed = hashPassword(password);

    for (auto &u : users)
    {
        if (u.username == username && u.passwordHash == hashed)
        {
            loggedInUser = u;
            return true;
        }
    }
    return false;
}

bool UserManager::resetPassword(const std::string &email,
                                const std::string &newPassword)
{
    for (auto &u : users)
    {
        if (u.email == email)
        {
            u.passwordHash = hashPassword(newPassword);
            saveUsers();
            return true;
        }
    }
    return false;
}

bool UserManager::isValidEmail(const std::string &email)
{
    auto atPos = email.find('@');
    auto dotPos = email.find('.', atPos + 1);

    return (atPos != std::string::npos &&
            dotPos != std::string::npos &&
            atPos < dotPos);
}
