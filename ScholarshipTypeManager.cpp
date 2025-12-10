
#include "ScholarshipTypeManager.h"
#include "FileManager.h"
#include <sstream>
#include <fstream>

ScholarshipTypeManager::ScholarshipTypeManager() {
    loadScholarshipTypes();
}

void ScholarshipTypeManager::loadScholarshipTypes() {
    scholarshipTypes.clear();

    // Создаем стандартные типы стипендий с обновленными описаниями
    scholarshipTypes.push_back(std::make_shared<ScholarshipType>(
        ScholarshipCategory::Academic,
        "Учебная стипендия",
        "Назначается в соответствии с успехами в учебе. Основной критерий - средний балл успеваемости не ниже 6.0.",
        "Регулярно, по окончании сессии"
    ));

    scholarshipTypes.push_back(std::make_shared<ScholarshipType>(
        ScholarshipCategory::Social,
        "Социальная стипендия",
        "Наличие соответствующего права (сироты, инвалиды I и II группы, иные категории согласно законодательству).",
        "Назначается на весь период обучения"
    ));

    scholarshipTypes.push_back(std::make_shared<ScholarshipType>(
        ScholarshipCategory::Named,
        "Именная стипендия",
        "Высокая успеваемость (средний балл не ниже 8.5), научная активность (наличие научных работ) и участие минимум в 1 конференции.",
        "Ежегодно на конкурсной основе"
    ));

    scholarshipTypes.push_back(std::make_shared<ScholarshipType>(
        ScholarshipCategory::Personal,
        "Персональная стипендия",
        "Успеваемость (средний балл не ниже 8.0), общественная активность и участие минимум в 3 конференциях.",
        "Назначается на семестр"
    ));

    scholarshipTypes.push_back(std::make_shared<ScholarshipType>(
        ScholarshipCategory::Presidential,
        "Президентская стипендия",
        "Отличная успеваемость (средний балл не ниже 9.0), научная и общественная активность, участие минимум в 3 конференциях.",
        "Назначается на весь период обучения"
    ));


    // Загружаем настройки из файла
    auto lines = FileManager::readLines("scholarship_settings.txt");
    for (const auto& line : lines) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string categoryStr, valueStr;
        if (std::getline(ss, categoryStr, '=')) {
            std::getline(ss, valueStr);
            try {
                int category = std::stoi(categoryStr);
                double value = std::stod(valueStr);

                if (category >= 0 && category <= 4) {
                    ScholarshipCategory cat = static_cast<ScholarshipCategory>(category);
                    auto scholarship = getScholarshipByCategory(cat);
                    if (scholarship) {
                        scholarship->setMinAverageGrade(value);
                    }
                }
            }
            catch (...) {
                // Игнорируем ошибки парсинга
            }
        }
    }
}

void ScholarshipTypeManager::saveScholarshipTypes() const {
    std::vector<std::string> lines;

    for (const auto& scholarship : scholarshipTypes) {
        std::stringstream ss;
        ss << static_cast<int>(scholarship->getCategory())
            << "=" << scholarship->getMinAverageGrade();
        lines.push_back(ss.str());
    }

    FileManager::writeLines("scholarship_settings.txt", lines);
}

std::shared_ptr<ScholarshipType> ScholarshipTypeManager::getScholarshipByCategory(ScholarshipCategory cat) const {
    for (const auto& scholarship : scholarshipTypes) {
        if (scholarship->getCategory() == cat) {
            return scholarship;
        }
    }
    return nullptr;
}

std::shared_ptr<ScholarshipType> ScholarshipTypeManager::getAcademicScholarship() const {
    return getScholarshipByCategory(ScholarshipCategory::Academic);
}

bool ScholarshipTypeManager::updateScholarshipTypeManager(ScholarshipCategory cat, double minGrade) {
    auto scholarship = getScholarshipByCategory(cat);
    if (scholarship) {
        scholarship->setMinAverageGrade(minGrade);
        saveScholarshipTypes();
        return true;
    }
    return false;
}


std::string ScholarshipTypeManager::getStudentScholarshipInfo(
    double studentAverage, bool hasSocialBenefits,
    bool hasScientificWorks, int conferences, bool isActiveInCommunity) const {  // ИСПРАВЛЕНО!

    std::stringstream ss;
    ss << "АНАЛИЗ ВОЗМОЖНОСТИ ПОЛУЧЕНИЯ СТИПЕНДИЙ\n";
    ss << "========================================\n\n";

    for (const auto& scholarship : scholarshipTypes) {
        ss << "Вид стипендии: " << scholarship->getName() << "\n";
        ss << "Описание: " << scholarship->getDescription() << "\n";
        ss << "Периодичность пересчета: " << scholarship->getRecalculationPeriod() << "\n";

        auto requirements = scholarship->checkRequirements(
            studentAverage, hasSocialBenefits,
            hasScientificWorks, conferences, isActiveInCommunity  // ИСПРАВЛЕНО!
        );

        ss << "Основные требования:\n";
        int metCount = 0;
        for (const auto& req : requirements) {
            ss << "  • " << req.description << ": "
                << (req.isMet ? "[+] ВЫПОЛНЕНО" : "[-] НЕ ВЫПОЛНЕНО") << "\n"; // Изменено с ✓/✗
            if (req.isMet) metCount++;
        }

        // Дополнительная информация (при желании)
        if (scholarship->getCategory() == ScholarshipCategory::Named ||
            scholarship->getCategory() == ScholarshipCategory::Personal ||
            scholarship->getCategory() == ScholarshipCategory::Presidential) {

            ss << "\nДополнительные возможности (указываются при желании):\n";

            // Творческая активность
            if (isActiveInCommunity) {
                ss << "  • Творческая активность: [+] УЧАСТИЕ ЕСТЬ\n"; // Изменено с ✓
            }
            else {
                ss << "  • Творческая активность: [-] НЕТ УЧАСТИЯ\n"; // Изменено с ✗
            }

            // Конкретное участие в конференциях
            if (conferences > 0) {
                ss << "  • Участие в конференциях: [+] " << conferences << " раз(а)\n"; // Изменено с ✓
            }
            else {
                ss << "  • Участие в конференциях: [-] НЕТ УЧАСТИЯ\n"; // Изменено с ✗
            }


            if (hasScientificWorks) {  // БУЛЕВО!
                ss << "  • Научные работы: [+] ЕСТЬ\n"; // Изменено с ✓
            }
            else {
                ss << "  • Научные работы: [-] НЕТ\n"; // Изменено с ✗
            }
        }

        if (scholarship->getCategory() == ScholarshipCategory::Academic) {
            ss << "\nВаш средний балл: " << studentAverage << "\n";
            ss << "Требуемый средний балл: " << scholarship->getMinAverageGrade() << "\n";
        }

        ss << "\nСтатус соответствия: ";
        if (metCount == requirements.size()) {
            ss << "ВЫ ПОЛНОСТЬЮ СООТВЕТСТВУЕТЕ ОСНОВНЫМ ТРЕБОВАНИЯМ!\n";
        }
        else {
            ss << "Выполнено " << metCount << " из " << requirements.size()
                << " основных требований.\n";
        }

        ss << "----------------------------------------\n\n";
    }

    return ss.str();
}

std::vector<std::shared_ptr<ScholarshipType>> ScholarshipTypeManager::getAvailableScholarshipsForStudent(
    double studentAverage, bool hasSocialBenefits,
    bool hasScientificWorks, int conferences, bool isActiveInCommunity) const {  // ИСПРАВЛЕНО!

    std::vector<std::shared_ptr<ScholarshipType>> availableScholarships;

    for (const auto& type : scholarshipTypes) {
        auto requirements = type->checkRequirements(
            studentAverage, hasSocialBenefits,
            hasScientificWorks, conferences, isActiveInCommunity  // ИСПРАВЛЕНО!
        );

        bool allMet = true;
        for (const auto& req : requirements) {
            if (!req.isMet) {
                allMet = false;
                break;
            }
        }

        // Дополнительная проверка для учебной стипендии
        if (type->getCategory() == ScholarshipCategory::Academic) {
            if (studentAverage < type->getMinAverageGrade()) {
                allMet = false;
            }
        }

        if (allMet) {
            availableScholarships.push_back(type);
        }
    }
    return availableScholarships;
}
