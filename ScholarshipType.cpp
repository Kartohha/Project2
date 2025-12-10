#include "ScholarshipType.h"
#include <algorithm>
#include <sstream>

// Конструктор
ScholarshipType::ScholarshipType(ScholarshipCategory cat, const std::string& name,
    const std::string& desc, const std::string& period)
    : category(cat), name(name), description(desc), recalculationPeriod(period),
    minAverageGrade(0.0), requiresApplication(false) {

    // Устанавливаем минимальный средний балл в зависимости от категории
    switch (cat) {
    case ScholarshipCategory::Academic:
        minAverageGrade = ACADEMIC_MIN_GRADE;
        addRequirement("Успеваемость (средний балл не ниже " + std::to_string(ACADEMIC_MIN_GRADE) + ")");
        requiresApplication = false;
        break;
    case ScholarshipCategory::Social:
        addRequirement("Наличие права на социальную стипендию (документы, подтверждающие льготу)");
        requiresApplication = true;
        break;
    case ScholarshipCategory::Named:
        minAverageGrade = NAMED_MIN_GRADE;
        addRequirement("Успеваемость (средний балл не ниже " + std::to_string(NAMED_MIN_GRADE) + ")");
        addRequirement("Научная активность (наличие научных работ)");
        addRequirement("Участие в конференциях (минимум 1)");
        addRequirement("Общественная активность");
        requiresApplication = true;
        break;
    case ScholarshipCategory::Personal:
        minAverageGrade = PERSONAL_MIN_GRADE;
        addRequirement("Успеваемость (средний балл не ниже " + std::to_string(PERSONAL_MIN_GRADE) + ")");
        addRequirement("Общественная активность");
        addRequirement("Участие в конференциях (минимум 3)");
        requiresApplication = true;
        break;
    case ScholarshipCategory::Presidential:
        minAverageGrade = PRESIDENTIAL_MIN_GRADE;
        addRequirement("Успеваемость (средний балл не ниже " + std::to_string(PRESIDENTIAL_MIN_GRADE) + ")");
        addRequirement("Научная активность (наличие научных работ)");
        addRequirement("Общественная активность");
        addRequirement("Участие в конференциях (минимум 3)");
        requiresApplication = true;
        break;
    }
}



std::vector<ScholarshipRequirement> ScholarshipType::checkRequirements(
    double studentAverage, bool hasSocialBenefits,
    bool hasScientificWorks, int conferences, bool isActiveInCommunity) const {

    std::vector<ScholarshipRequirement> result = requirements;

    for (auto& req : result) {
        std::string desc = req.description;
        std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

        if (desc.find("успеваемость") != std::string::npos ||
            desc.find("средний балл") != std::string::npos) {
            req.isMet = (studentAverage >= minAverageGrade);
        }
        else if (desc.find("льгот") != std::string::npos ||
            desc.find("социальн") != std::string::npos ||
            desc.find("законодатель") != std::string::npos) {
            req.isMet = hasSocialBenefits;
        }
        else if (desc.find("научная") != std::string::npos ||
            desc.find("исследовани") != std::string::npos ||
            desc.find("научно") != std::string::npos ||
            desc.find("работ") != std::string::npos) {
            req.isMet = hasScientificWorks;
        }
        else if (desc.find("конференц") != std::string::npos) {
            // Проверяем количество конференций
            if (desc.find("минимум 1") != std::string::npos) {
                req.isMet = (conferences >= 1);
            }
            else if (desc.find("минимум 3") != std::string::npos) {
                req.isMet = (conferences >= 3);
            }
            else {
                req.isMet = (conferences > 0);
            }
        }
        else if (desc.find("обществен") != std::string::npos ||
            desc.find("активность") != std::string::npos) {
            req.isMet = isActiveInCommunity;
        }
        else {
            req.isMet = true;
        }
    }

    return result;
}

void ScholarshipType::addRequirement(const std::string& req) {
    requirements.push_back({ req, false });
}

std::string ScholarshipType::categoryToString(ScholarshipCategory cat) {
    switch (cat) {
    case ScholarshipCategory::Academic: return "Учебная";
    case ScholarshipCategory::Social: return "Социальная";
    case ScholarshipCategory::Named: return "Именная";
    case ScholarshipCategory::Personal: return "Персональная";
    case ScholarshipCategory::Presidential: return "Президентская";
    default: return "Неизвестная";
    }
}

std::string ScholarshipType::getRecommendations(double studentAverage,
    int publications, int conferences, bool isActiveInCommunity) const {

    std::stringstream ss;

    if (category == ScholarshipCategory::Academic) {
        if (studentAverage < minAverageGrade) {
            ss << "Для получения " << name << " необходимо повысить средний балл до "
                << minAverageGrade << " (текущий: " << studentAverage << ")\n";
        }
    }
    else if (category == ScholarshipCategory::Social) {
        ss << "Для получения " << name << " необходимо предоставить документы, подтверждающие право на льготу\n";
    }
    else if (category == ScholarshipCategory::Named) {
        ss << "Для получения " << name << " необходимо:\n";
        if (studentAverage < minAverageGrade) {
            ss << "• Повысить средний балл до " << minAverageGrade
                << " (текущий: " << studentAverage << ")\n";
        }
        if (publications == 0) {
            ss << "• Выполнить научную работу\n";
        }
        if (conferences < 1) {
            ss << "• Принять участие минимум в 1 научной конференции\n";
        }
        if (!isActiveInCommunity) {
            ss << "• Проявить активность в общественной жизни университета\n";
        }
    }
    else if (category == ScholarshipCategory::Personal) {
        ss << "Для получения " << name << " необходимо:\n";
        if (studentAverage < minAverageGrade) {
            ss << "• Повысить средний балл до " << minAverageGrade
                << " (текущий: " << studentAverage << ")\n";
        }
        if (!isActiveInCommunity) {
            ss << "• Проявить активность в общественной жизни\n";
        }
        if (conferences < 3) {
            ss << "• Принять участие минимум в 3 конференциях (текущее количество: " << conferences << ")\n";
        }
    }
    else if (category == ScholarshipCategory::Presidential) {
        ss << "Для получения " << name << " необходимо:\n";
        if (studentAverage < minAverageGrade) {
            ss << "• Повысить средний балл до " << minAverageGrade
                << " (текущий: " << studentAverage << ")\n";
        }
        if (publications == 0) {
            ss << "• Выполнить научную работу\n";
        }
        if (!isActiveInCommunity) {
            ss << "• Проявить активность в общественной жизни университета\n";
        }
        if (conferences < 3) {
            ss << "• Принять участие минимум в 3 конференциях (текущее количество: " << conferences << ")\n";
        }
    }

    return ss.str();
}