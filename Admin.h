#ifndef ADMIN_H
#define ADMIN_H

#include "IUser.h"

class Admin : public IUser {
public:
    Admin(const std::string& user, const std::string& hash) : IUser(user, hash) {}
    std::string getRole() const override { return "admin"; }
};

#endif // ADMIN_H