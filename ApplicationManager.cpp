#include "ApplicationManager.h"
#include "FileManager.h"
#include "ScholarshipType.h"  // Добавляем
#include <algorithm>
#include <sstream>
#include <iostream>  // Добавить эту строку для std::cout/std::cerr

ApplicationManager::ApplicationManager(const std::string& filename)
    : applicationsFile(filename), history("history.txt") {
    loadApplications();
}

void ApplicationManager::loadApplications() {
    applications.clear();
    auto lines = FileManager::readLines(applicationsFile);

    int maxId = 0;  // Для отслеживания максимального ID

    for (const auto& line : lines) {
        if (line.empty()) continue;

        try {
            Application app;
            if (app.loadFromString(line)) {
                applications.push_back(app);
                // Обновляем максимальный найденный ID
                if (app.getId() > maxId) {
                    maxId = app.getId();
                }
            }
        }
        catch (...) {
            // Игнорируем некорректные строки
        }
    }

    // Устанавливаем следующий ID как максимальный + 1
    if (maxId > 0) {
        Application::setNextId(maxId + 1);
    }
    // Если заявок нет, nextId остается = 1 (по умолчанию)
}

void ApplicationManager::saveApplications() const {
    std::vector<std::string> lines;

    for (const auto& app : applications) {
        lines.push_back(app.saveToString());
    }

    FileManager::writeLines(applicationsFile, lines);
}

bool ApplicationManager::addApplication(const Application& app) {
    // Проверяем, нет ли заявки с таким же ID
    for (const auto& existingApp : applications) {
        if (existingApp.getId() == app.getId()) {
            return false;
        }
    }

    // Добавляем запись в историю
    history.addRecord(app.getId(), app.getStudentUsername(),
        ScholarshipType::categoryToString(app.getScholarshipCategory()),
        HistoryAction::CREATED, "", "Заявка создана");

    applications.push_back(app);
    saveApplications();

    return true;
}

bool ApplicationManager::removeApplicationById(int id, const std::string& deleter) {
    for (auto it = applications.begin(); it != applications.end(); ++it) {
        if (it->getId() == id) {
            // Записываем в историю
            history.addRecord(id, it->getStudentUsername(),
                ScholarshipType::categoryToString(it->getScholarshipCategory()),
                HistoryAction::DELETED, deleter, "Заявка удалена");

            applications.erase(it);
            saveApplications();
            return true;
        }
    }
    return false;
}

bool ApplicationManager::removeApplicationsByStudent(const std::string& username) {
    bool removed = false;

    for (auto it = applications.begin(); it != applications.end();) {
        if (it->getStudentUsername() == username) {
            // Записываем в историю
            history.addRecord(it->getId(), username,
                ScholarshipType::categoryToString(it->getScholarshipCategory()),
                HistoryAction::DELETED, "system", "Заявка удалена вместе со студентом");

            it = applications.erase(it);
            removed = true;
        }
        else {
            ++it;
        }
    }

    if (removed) {
        saveApplications();
    }
    return removed;
}

bool ApplicationManager::updateApplicationStatusById(int id, ApplicationStatus newStatus,
    const std::string& adminUsername) {
    for (auto& app : applications) {
        if (app.getId() == id) {
            // Записываем в историю
            HistoryAction action = (newStatus == ApplicationStatus::Approved) ?
                HistoryAction::APPROVED : HistoryAction::REJECTED;

            history.addRecord(id, app.getStudentUsername(),
                ScholarshipType::categoryToString(app.getScholarshipCategory()),
                action, adminUsername, "Изменение статуса");

            app.setStatus(newStatus);
            saveApplications();
            return true;
        }
    }
    return false;
}

std::vector<Application> ApplicationManager::getAllApplications() const {
    return applications;
}

std::vector<Application> ApplicationManager::getApplicationsByStudent(const std::string& username) const {
    std::vector<Application> result;

    for (const auto& app : applications) {
        if (app.getStudentUsername() == username) {
            result.push_back(app);
        }
    }

    return result;
}

Application* ApplicationManager::getApplicationById(int id) {
    for (auto& app : applications) {
        if (app.getId() == id) {
            return &app;
        }
    }
    return nullptr;
}

const Application* ApplicationManager::getApplicationById(int id) const {
    for (const auto& app : applications) {
        if (app.getId() == id) {
            return &app;
        }
    }
    return nullptr;
}

std::vector<Application> ApplicationManager::searchApplications(double minAvg, double maxAvg, int statusFilter) const {
    std::vector<Application> result;

    for (const auto& app : applications) {
        // Фильтр по баллу
        if (app.getAverageGrade() < minAvg || app.getAverageGrade() > maxAvg) {
            continue;
        }

        // Фильтр по статусу
        if (statusFilter != 0) {
            int appStatus = static_cast<int>(app.getStatus());
            if (appStatus != statusFilter) {
                continue;
            }
        }

        result.push_back(app);
    }

    return result;
}

int ApplicationManager::getPendingApplicationsCount(const std::string& username) const {
    int count = 0;

    for (const auto& app : applications) {
        if (app.getStudentUsername() == username &&
            app.getStatus() == ApplicationStatus::Pending) {
            count++;
        }
    }

    return count;
}