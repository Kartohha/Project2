#include "UserManager.h"
#include "FileManager.h"
#include <sstream>
#include <algorithm>
#include <iostream>

UserManager::UserManager() {
    std::cout << "=== ИНИЦИАЛИЗАЦИЯ UserManager ===" << std::endl;
    std::cout << "Загружаем пользователей из файла..." << std::endl;
    loadUsers();
    std::cout << "Загрузка завершена. Всего пользователей: " << users.size() << std::endl;
}

// В loadUsers() обновляем загрузку:
void UserManager::loadUsers() {
    users.clear();
    auto lines = FileManager::readLines("users.txt");

    std::cout << "DEBUG: Найдено строк в файле: " << lines.size() << std::endl;

    for (size_t i = 0; i < lines.size(); ++i) {
        const auto& line = lines[i];

        if (line.empty()) {
            std::cout << "DEBUG: Строка " << i << " пустая, пропускаем" << std::endl;
            continue;
        }

        std::cout << "DEBUG: Обрабатываем строку " << i << ": " << line.substr(0, 50) << "..." << std::endl;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;

        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        std::cout << "DEBUG: Полей в строке: " << fields.size() << std::endl;

        if (fields.size() < 3) {
            std::cout << "DEBUG: Слишком мало полей (< 3), пропускаем" << std::endl;
            continue;
        }

        std::string role = fields[0];
        std::string username = fields[1];
        std::string passwordHash = fields[2];

        std::cout << "DEBUG: Роль: " << role << ", Логин: " << username << std::endl;

        if (role == "student") {
            // Убедимся, что достаточно полей
            if (fields.size() < 16) {  // Минимум 16 полей для студента
                std::cout << "DEBUG: У студента " << username
                    << " недостаточно полей (" << fields.size()
                    << " вместо минимум 16), пропускаем" << std::endl;
                continue;
            }

            try {
                // Извлекаем поля с проверками
                std::string fio = fields[3];

                double avg = 0.0;
                if (!fields[4].empty()) avg = std::stod(fields[4]);

                bool hasScholarship = (fields.size() > 5 && fields[5] == "1");
                bool hasSocialBenefits = (fields.size() > 6 && fields[6] == "1");
                bool hasScientificWorks = (fields.size() > 7 && fields[7] == "1");

                int conferences = 0;
                if (fields.size() > 8 && !fields[8].empty())
                    conferences = std::stoi(fields[8]);

                bool isActiveInCommunity = (fields.size() > 9 && fields[9] == "1");
                StudyForm studyForm = (fields.size() > 10 && fields[10] == "1") ? StudyForm::Paid : StudyForm::Budget;

                int course = 1;
                if (fields.size() > 11 && !fields[11].empty())
                    course = std::stoi(fields[11]);
                if (course < 1 || course > 4) course = 1;

                std::string group = (fields.size() > 12) ? fields[12] : "";
                std::string faculty = (fields.size() > 13) ? fields[13] : "";
                std::string specialty = (fields.size() > 14) ? fields[14] : "";
                std::string scholarshipType = (fields.size() > 15) ? fields[15] : "";

                // Комментарии (могут быть пустыми)
                std::string socialComment = (fields.size() > 16) ? fields[16] : "";
                std::string scientificComment = (fields.size() > 17) ? fields[17] : "";
                std::string conferencesComment = (fields.size() > 18) ? fields[18] : "";
                std::string activityComment = (fields.size() > 19) ? fields[19] : "";

                // Создаем студента
                auto student = std::make_shared<Student>(
                    username, passwordHash, fio, avg, hasScholarship,
                    hasSocialBenefits, hasScientificWorks, conferences,
                    isActiveInCommunity, studyForm, course, group,
                    faculty, specialty, scholarshipType,
                    socialComment, scientificComment, conferencesComment, activityComment
                );

                users.push_back(student);
                std::cout << "DEBUG: Успешно загружен студент: " << fio
                    << " (логин: " << username << ")" << std::endl;

            }
            catch (const std::exception& e) {
                std::cout << "DEBUG: Ошибка при загрузке студента " << username
                    << ": " << e.what() << std::endl;
            }
        }
        else if (role == "admin") {
            auto admin = std::make_shared<Admin>(username, passwordHash);
            users.push_back(admin);
            std::cout << "DEBUG: Загружен админ: " << username << std::endl;
        }
    }

    std::cout << "DEBUG: Загрузка завершена. Всего пользователей: " << users.size() << std::endl;
}

// В saveUsers() обновляем сохранение:
void UserManager::saveUsers() const {
   

    std::vector<std::string> lines;

    for (const auto& user : users) {
        

        std::stringstream ss;

        if (user->getRole() == "student") {
            auto stu = std::static_pointer_cast<Student>(user);

            ss << "student,"
                << stu->getUsername() << ","
                << stu->getPasswordHash() << ","
                << stu->getFio() << ","
                << stu->getAverageGrade() << ","
                << (stu->getHasScholarship() ? "1" : "0") << ","
                << (stu->getHasSocialBenefits() ? "1" : "0") << ","
                << (stu->getHasScientificWorks() ? "1" : "0") << ","
                << stu->getConferencesCount() << ","
                << (stu->getIsActiveInCommunity() ? "1" : "0") << ","
                << (stu->getStudyForm() == StudyForm::Budget ? "0" : "1") << ","
                << stu->getCourse() << ","
                << stu->getGroup() << ","
                << stu->getFaculty() << ","
                << stu->getSpecialty() << ","
                << stu->getScholarshipType() << ","
                << stu->getSocialBenefitsComment() << ","
                << stu->getScientificWorksComment() << ","
                << stu->getConferencesComment() << ","
                << stu->getCommunityActivityComment();
        }
        else {
            ss << "admin," << user->getUsername() << "," << user->getPasswordHash();
        }

        lines.push_back(ss.str());
       
    }

    FileManager::writeLines("users.txt", lines);
    std::cout << "=== СОХРАНЕНИЕ ЗАВЕРШЕНО ===" << std::endl;
}

std::shared_ptr<IUser> UserManager::findUser(const std::string& username) const {
    for (const auto& user : users) {
        if (user->getUsername() == username) return user;
    }
    return nullptr;
}

bool UserManager::addUser(std::shared_ptr<IUser> user) {
    if (findUser(user->getUsername())) return false;
    users.push_back(user);
    return true;
}

bool UserManager::removeUser(const std::string& username) {
    for (auto it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getUsername() == username) {
            users.erase(it);
            return true;
        }
    }
    return false;
}

bool UserManager::updateUserPassword(const std::string& username, const std::string& newHash) {
    auto u = findUser(username);
    if (!u) return false;
    u->setPasswordHash(newHash);
    return true;
}

std::vector<std::shared_ptr<Student>> UserManager::getAllStudents() const {
    std::vector<std::shared_ptr<Student>> result;
    for (const auto& user : users) {
        if (user->getRole() == "student") {
            result.push_back(std::static_pointer_cast<Student>(user));
        }
    }
    return result;
}