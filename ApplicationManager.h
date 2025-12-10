#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <vector>
#include <string>
#include "Application.h"
#include "ScholarshipType.h"  // Добавляем для categoryToString
#include "ApplicationHistory.h"  // Добавляем

class ApplicationManager {
private:
    std::vector<Application> applications;
    std::string applicationsFile;
    ApplicationHistory history;

public:
    ApplicationManager(const std::string& filename = "applications.txt");

    void loadApplications();
    void saveApplications() const;

    bool addApplication(const Application& app);
    bool removeApplicationById(int id, const std::string& deleter = "");
    bool removeApplicationsByStudent(const std::string& username);

    bool updateApplicationStatusById(int id, ApplicationStatus newStatus,
        const std::string& adminUsername = "");

    std::vector<Application> getAllApplications() const;
    std::vector<Application> getApplicationsByStudent(const std::string& username) const;
    Application* getApplicationById(int id);
    const Application* getApplicationById(int id) const;

    std::vector<Application> searchApplications(double minAvg, double maxAvg, int statusFilter = 0) const;

    int getPendingApplicationsCount(const std::string& username) const;

    // Метод для доступа к истории
    ApplicationHistory& getHistory() { return history; }
    const ApplicationHistory& getHistory() const { return history; }
};

#endif // APPLICATIONMANAGER_H