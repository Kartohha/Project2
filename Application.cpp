#include "Application.h"
#include <iostream>
#include <sstream>
#include "ScholarshipType.h"  // Для доступа к ScholarshipType::categoryToString

// Инициализация статической переменной
int Application::nextId = 1;

Application::Application(const std::string& user, double avg,
    ScholarshipCategory category, ApplicationStatus st)
    : id(nextId++), studentUsername(user), averageGrade(avg),
    status(st), scholarshipCategory(category) {

    // Валидация данных
    if (studentUsername.empty()) {
        throw std::invalid_argument("Имя пользователя не может быть пустым");
    }
    if (avg < 0.0 || avg > 10.0) {
        throw std::invalid_argument("Средний балл должен быть от 0.0 до 10.0");
    }
}

std::ostream& operator<<(std::ostream& os, const Application& app) {
    os << "ID: " << app.getId()
        << ", Студент: " << app.getStudentUsername()
        << ", Средний балл: " << app.getAverageGrade()
        << ", Тип стипендии: " << ScholarshipType::categoryToString(app.getScholarshipCategory())
        << ", Статус: ";
    switch (app.getStatus()) {
    case ApplicationStatus::Pending: os << "Ожидание"; break;
    case ApplicationStatus::Approved: os << "Одобрено"; break;
    case ApplicationStatus::Rejected: os << "Отклонено"; break;
    }
    return os;
}

bool Application::loadFromString(const std::string& str) {
    std::stringstream ss(str);
    char delimiter;

    ss >> id >> delimiter;
    if (delimiter != '|') return false;

    std::getline(ss, studentUsername, '|');
    ss >> averageGrade >> delimiter;
    if (delimiter != '|') return false;

    int categoryInt, statusInt;
    ss >> categoryInt >> delimiter;
    if (delimiter != '|') return false;
    ss >> statusInt;

    scholarshipCategory = static_cast<ScholarshipCategory>(categoryInt);
    status = static_cast<ApplicationStatus>(statusInt);

    return true;
}

std::string Application::saveToString() const {
    std::stringstream ss;
    ss << id << "|"
        << studentUsername << "|"
        << averageGrade << "|"
        << static_cast<int>(scholarshipCategory) << "|"
        << static_cast<int>(status);
    return ss.str();
}
void Application::setNextId(int id) {
    nextId = id;
}