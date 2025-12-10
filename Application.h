#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include "ScholarshipType.h"  // Включаем для использования ScholarshipCategory

enum class ApplicationStatus { Pending = 0, Approved = 1, Rejected = 2 };

class Application {
    int id;
    std::string studentUsername;
    double averageGrade;
    ApplicationStatus status;
    ScholarshipCategory scholarshipCategory;

    static int nextId;

public:
    Application() = default;
    Application(const std::string& user, double avg,
        ScholarshipCategory category = ScholarshipCategory::Academic,
        ApplicationStatus st = ApplicationStatus::Pending);

    // Геттеры
    int getId() const { return id; }
    std::string getStudentUsername() const { return studentUsername; }
    double getAverageGrade() const { return averageGrade; }
    ApplicationStatus getStatus() const { return status; }
    ScholarshipCategory getScholarshipCategory() const { return scholarshipCategory; }

    // Сеттеры
    void setStatus(ApplicationStatus s) { status = s; }
    void setScholarshipCategory(ScholarshipCategory category) { scholarshipCategory = category; }

    // Методы для сериализации
    bool loadFromString(const std::string& str);
    std::string saveToString() const;

    // Операторы
    bool operator==(const Application& other) const {
        return id == other.id;
    }

    bool operator<(const Application& other) const {
        if (averageGrade != other.averageGrade)
            return averageGrade < other.averageGrade;
        return studentUsername < other.studentUsername;
    }

    friend std::ostream& operator<<(std::ostream& os, const Application& app);

    // Статические методы для управления ID
    static void resetIdCounter() { nextId = 1; }
    static void setNextId(int id); // Добавить этот метод
};

#endif // APPLICATION_H