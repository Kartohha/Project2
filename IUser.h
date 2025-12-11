#ifndef IUSER_H
#define IUSER_H

#include <string>

class IUser {
protected:
    std::string username;
    std::string passwordHash;
public:
    IUser(const std::string& user, const std::string& hash) : username(user), passwordHash(hash) {}
    virtual ~IUser() {}
    std::string getUsername() const { return username; }
    std::string getPasswordHash() const { return passwordHash; }
    void setPasswordHash(const std::string& hash) { passwordHash = hash; }
    virtual std::string getRole() const = 0;
};

#endif
