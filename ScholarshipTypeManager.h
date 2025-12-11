#ifndef SCHOLARSHIPTYPEMANAGER_H
#define SCHOLARSHIPTYPEMANAGER_H

#include "ScholarshipType.h"
#include <vector>
#include <memory>

class ScholarshipTypeManager {
private:
    std::vector<std::shared_ptr<ScholarshipType>> scholarshipTypes;

public:
    ScholarshipTypeManager();

    void loadScholarshipTypes();
    void saveScholarshipTypes() const;

    // Получение списка
    std::vector<std::shared_ptr<ScholarshipType>> getAllScholarshipTypes() const { return scholarshipTypes; }

    // Поиск по категории
    std::shared_ptr<ScholarshipType> getScholarshipByCategory(ScholarshipCategory cat) const;

    // Получение учебной стипендии (для совместимости)
    std::shared_ptr<ScholarshipType> getAcademicScholarship() const;

    // Обновление критериев
    bool updateScholarshipTypeManager(ScholarshipCategory cat, double minGrade = 0.0);

    // Получение информации для студента
    std::string getStudentScholarshipInfo(double studentAverage,
        bool hasSocialBenefits = false,
        bool hasScientificWorks = false,
        int conferences = 0,
        bool isActiveInCommunity = false) const;

    // Получение доступных стипендий
    std::vector<std::shared_ptr<ScholarshipType>> getAvailableScholarshipsForStudent(
        double studentAverage,
        bool hasSocialBenefits = false,
        bool hasScientificWorks = false,
        int conferences = 0,
        bool isActiveInCommunity = false) const;
};

#endif 