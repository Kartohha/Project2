#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {
    std::string hashPassword(const std::string& password);
    std::string toLower(const std::string& s);
    std::string statusToString(int status);
    int stringToStatus(const std::string& s);
    long long currentTimeSeconds();

    std::string escapeCSV(const std::string& field);
    std::string unescapeCSV(const std::string& field);

    // Константы для проверки баллов
    static constexpr double MIN_GRADE = 0.0;
    static constexpr double MAX_GRADE = 10.0;

    static bool isValidGrade(double grade) {
        return grade >= MIN_GRADE && grade <= MAX_GRADE;
    }
}

#endif 