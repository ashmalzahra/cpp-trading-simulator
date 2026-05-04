#pragma once
#include <string>
#include <vector>
#include "User.h"

class UserManager
{
public:
    UserManager(const std::string &filename = "users.csv");

    bool loadUsers();
    bool saveUsers();

    bool registerUser(const std::string &fullName,
                      const std::string &email,
                      const std::string &password,
                      std::string &generatedUsername);

    bool loginUser(const std::string &username,
                   const std::string &password,
                   User &loggedInUser);

    bool resetPassword(const std::string &email,
                       const std::string &newPassword);

    bool isValidEmail(const std::string &email);

    const std::vector<User> &getUsers() const { return users; }

private:
    std::string filename;
    std::vector<User> users;

    bool userExists(const std::string &fullName, const std::string &email);
    size_t hashPassword(const std::string &password);
    std::string generateUsername();
};
