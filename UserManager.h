#ifndef USERMANAGER_H
#define USERMANAGER_H

#include "IUser.h"
#include "Student.h"
#include "Admin.h"
#include <vector>
#include <memory>

class UserManager {
    std::vector<std::shared_ptr<IUser>> users;

public:
    UserManager();
    void loadUsers();
    void saveUsers() const;

    std::shared_ptr<IUser> findUser(const std::string& username) const;
    bool addUser(std::shared_ptr<IUser> user);
    bool removeUser(const std::string& username);
    bool updateUserPassword(const std::string& username, const std::string& newHash);

    std::vector<std::shared_ptr<Student>> getAllStudents() const;
    std::vector<std::shared_ptr<IUser>> getAllUsers() const { return users; }
};

#endif // USERMANAGER_H