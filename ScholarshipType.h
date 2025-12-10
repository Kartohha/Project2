#ifndef SCHOLARSHIPTYPE_H
#define SCHOLARSHIPTYPE_H

#include <string>
#include <vector>

enum class ScholarshipCategory {
    Academic,      // Учебная
    Social,        // Социальная
    Named,         // Именная
    Personal,      // Персональная
    Presidential   // Президентская
};

struct ScholarshipRequirement {
    std::string description;
    bool isMet;
};

class ScholarshipType {
private:
    ScholarshipCategory category;
    std::string name;
    std::string description;
    std::string recalculationPeriod;
    std::vector<ScholarshipRequirement> requirements;
    double minAverageGrade;  // Для учебной стипендии
    bool requiresApplication; // Требуется ли заявка
    static constexpr double ACADEMIC_MIN_GRADE = 6.0;
    static constexpr double NAMED_MIN_GRADE = 8.5;
    static constexpr double PERSONAL_MIN_GRADE = 8.0;
    static constexpr double PRESIDENTIAL_MIN_GRADE = 9.0;

public:
    ScholarshipType(ScholarshipCategory cat, const std::string& name,
        const std::string& desc, const std::string& period);

    // Геттеры
    ScholarshipCategory getCategory() const { return category; }
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    std::string getRecalculationPeriod() const { return recalculationPeriod; }
    std::vector<ScholarshipRequirement> getRequirements() const { return requirements; }
    double getMinAverageGrade() const { return minAverageGrade; }
    bool getRequiresApplication() const { return requiresApplication; }

    // Сеттеры
    void setMinAverageGrade(double grade) { minAverageGrade = grade; }
    void setRequiresApplication(bool requires) { requiresApplication = requires; }

    // Методы для работы с требованиями
    void addRequirement(const std::string& req);
    void clearRequirements() { requirements.clear(); }

    // ИСПРАВЛЕННАЯ СИГНАТУРА: третий параметр bool (hasScientificWorks), а не int (publications)
    std::vector<ScholarshipRequirement> checkRequirements(
        double studentAverage,
        bool hasSocialBenefits,
        bool hasScientificWorks,  // БУЛЕВО, а не int!
        int conferences,
        bool isActiveInCommunity) const;

    // Получение названия категории
    static std::string categoryToString(ScholarshipCategory cat);

    std::string getRecommendations(double studentAverage,
        int publications,  // Оставляем int для обратной совместимости
        int conferences,
        bool isActiveInCommunity) const;
};

#endif // SCHOLARSHIPTYPE_H