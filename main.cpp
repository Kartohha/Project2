
#define NOMINMAX
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <limits>
#include <windows.h>
#include <locale>
#include <iomanip>
#include <optional> 
#include <stdexcept>
#include <map>
#include <set>
#include "UserManager.h"
#include "ScholarshipTypeManager.h"
#include "ScholarshipType.h"        
#include "ApplicationManager.h"
#include "Utils.h"
#include "SecurityManager.h"
#include "Student.h"
#include <chrono>
#include "Admin.h"
#include "Application.h"
#include "ApplicationHistory.h"
#include <fstream>
#include <ctime>
#include "InputValidation.h"

using namespace std;

void studentMenu(std::shared_ptr<Student> student, UserManager& userManager,
    ScholarshipTypeManager& scholarshipManager, ApplicationManager& appManager);
void adminMenu(UserManager& userManager, ScholarshipTypeManager& scholarshipManager,
    ApplicationManager& appManager, SecurityManager& security,
    const std::string& adminUsername);
void studentManagementMenu(UserManager& userManager, ScholarshipTypeManager& scholarshipManager,
    ApplicationManager& appManager);
void viewApplicationHistory(const ApplicationHistory& history, UserManager& userManager);

void printStudentInfo(const std::shared_ptr<Student>& student,
    const ApplicationManager& appManager,
    const ScholarshipTypeManager& scholarshipManager,
    int number = 0) {
    if (number > 0) {
        InputUtils::printSection("СТУДЕНТ #" + std::to_string(number));
    }
    std::cout << "ФИО: " << student->getFio() << "\n";
    std::cout << "Логин: " << student->getUsername() << "\n";
    std::cout << "Средний балл: " << student->getAverageGrade() << "/10.0\n";
    std::cout << "Статус стипендии: " << (student->getHasScholarship() ? "[+] НАЗНАЧЕНА" : "[-] НЕ НАЗНАЧЕНА") << "\n"; // Изменено с ✅/❌
    std::cout << "Социальные льготы: " << (student->getHasSocialBenefits() ? "[+] ЕСТЬ" : "[-] НЕТ") << "\n"; // Изменено с ✅/❌
    std::cout << "Публикации: " << student->getHasScientificWorks() << "\n";
    std::cout << "Участие в конференциях: " << student->getConferencesCount() << "\n";
    std::cout << "Общественная активность: " << (student->getIsActiveInCommunity() ? "[+] АКТИВЕН" : "[-] НЕ АКТИВЕН") << "\n"; // Изменено с ✅/❌
}

void handleLogin(UserManager& userManager, ScholarshipTypeManager& scholarshipManager,
    ApplicationManager& appManager, SecurityManager& security) {

    InputUtils::printHeader("Вход в систему");

    try {
        // Ввод логина с валидацией
        std::string username = InputValidator::getStringInput(
            "Логин: ",
            Validators::validateUsername,
            "Логин должен быть от 3 до 20 символов",
            false
        );

        // Ввод пароля
        std::string password = InputValidator::getPasswordHidden("Пароль: ");

        auto user = userManager.findUser(username);
        if (!user || user->getPasswordHash() != Utils::hashPassword(password)) {
            InputUtils::printError("Неверный логин или пароль!");
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            return;
        }

        if (user->getRole() == "student") {
            InputUtils::printSuccess("Добро пожаловать, " + username + "!");
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");

            SafeExecutor::execute([&]() {
                studentMenu(std::static_pointer_cast<Student>(user),
                    userManager, scholarshipManager, appManager);
                }, "открытия меню студента");
        }
        else {
            // Администраторский доступ
            if (security.isLocked()) {
                InputUtils::printError("Система заблокирована. Попробуйте позже.");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                return;
            }

            int attempts = 3;
            bool authenticated = false;

            for (int i = 0; i < attempts; ++i) {
                std::string masterPassword = InputValidator::getStringInput(
                    "Мастер-пароль: ",
                    nullptr,
                    "",
                    false
                );

                if (security.verifyMasterPassword(masterPassword)) {
                    authenticated = true;
                    break;
                }
                else {
                    InputUtils::printError("Неверный мастер-пароль.");
                    if (security.isLocked()) {
                        InputUtils::printError("Система заблокирована.");
                        break;
                    }
                }
            }

            if (!authenticated) {
                InputUtils::printError("Доступ запрещен!");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                return;
            }

            InputUtils::printSuccess("Доступ разрешен. Добро пожаловать, администратор!");
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            adminMenu(userManager, scholarshipManager, appManager, security, username);
        }
    }
    catch (const InputException& e) {
        InputUtils::printError("Ошибка ввода: " + std::string(e.what()));
        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
    }
    catch (const std::exception& e) {
        InputUtils::printError("Ошибка: " + std::string(e.what()));
        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
    }
}

void handleStudentRegistration(UserManager& userManager) {
    InputUtils::printHeader("Регистрация студента");

    try {
        // Ввод логина с проверкой уникальности
        std::string username;
        bool usernameValid = false;
        int attempts = 0;

        while (!usernameValid && attempts < 3) {
            try {
                username = InputValidator::getStringInput(
                    "Придумайте логин: ",
                    [&userManager](const std::string& uname) {
                        Validators::validateUsername(uname);
                        if (userManager.findUser(uname)) {
                            throw RangeException("Пользователь с таким логином уже существует!");
                        }
                        return true;
                    },
                    "Логин должен быть от 3 до 20 символов"
                );
                usernameValid = true;
            }
            catch (const InputException& e) {
                attempts++;
                InputUtils::printError(std::string(e.what()) +
                    " (Попытка " + std::to_string(attempts) + " из 3)");

                if (attempts >= 3) {
                    InputUtils::printError("Превышено количество попыток.");
                    return;
                }
            }
        }

        // Ввод пароля
        std::string password;
        try {
            password = InputValidator::getPasswordInput("Придумайте пароль: ", 6);
        }
        catch (const PasswordException& e) {
            InputUtils::printError(e.what());
            return;
        }

        // Ввод ФИО
        std::string fio = InputValidator::getLineInput(
            "ФИО: ",
            Validators::validateFIO,
            "ФИО должно быть от 5 до 5 символов",
            false,
            5,
            100
        );

        // Ввод среднего балла
        double avg = InputValidator::getDoubleInput(
            "Средний балл (0.0-10.0): ",
            Validators::validateGrade,
            "Балл должен быть от 0.0 до 10.0"
        );

        // Ввод формы обучения
        std::cout << "\nФорма обучения:\n";
        std::cout << "1. Бюджетная\n";
        std::cout << "2. Платная\n";

        int formChoice = InputValidator::getIntInput(
            "Выберите форму обучения (1 или 2): ",
            [](int val) { return val == 1 || val == 2; },
            "Введите 1 или 2"
        );

        StudyForm studyForm = (formChoice == 2) ? StudyForm::Paid : StudyForm::Budget;

        // Ввод курса
        int course = InputValidator::getIntInput(
            "Курс (1-4): ",
            Validators::validateCourse,
            "Курс должен быть от 1 до 4"
        );

        // Ввод группы
        std::string group = InputValidator::getLineInput(
            "Группа: ",
            Validators::validateGroup,
            "Название группы должно быть от 2 до 20 символов",
            false,
            2,
            20
        );

        // Ввод факультета
        std::string faculty = InputValidator::getLineInput(
            "Факультет: ",
            Validators::validateFaculty,
            "Название факультета должно быть от 3 до 50 символов",
            false,
            3,
            50
        );

        // Ввод специальности
        std::string specialty = InputValidator::getLineInput(
            "Специальность: ",
            Validators::validateSpecialty,
            "Название специальности должно быть от 2 до 10 символов",
            false,
            2,
            10
        );

        InputUtils::printSection("Дополнительная информация");
        bool hasSocialBenefits = InputValidator::getYesNoInput("Есть социальные льготы?");
        bool hasScientificWorks = InputValidator::getYesNoInput("Есть научные работы?");

        int conferences = InputValidator::getIntInput(
            "Участие в конференциях (количество): ",
            Validators::validateConferences,
            "Количество не может быть отрицательным"
        );

        bool isActive = InputValidator::getYesNoInput("Активен в общественной жизни?");

        // Создание студента
        bool success = SafeExecutor::execute([&]() {
            auto student = std::make_shared<Student>(
                username,
                Utils::hashPassword(password),
                fio, avg, false,
                hasSocialBenefits, hasScientificWorks, conferences, isActive,
                studyForm, course, group, faculty, specialty
            );

            if (!userManager.addUser(student)) {
                throw std::runtime_error("Ошибка при добавлении пользователя в систему");
            }

            userManager.saveUsers();
            }, "регистрации студента");

        if (success) {
            InputUtils::printSuccess("Студент успешно зарегистрирован!");
        }

    }
    catch (const InputException& e) {
        InputUtils::printError("Регистрация прервана: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        InputUtils::printError("Ошибка при регистрации: " + std::string(e.what()));
    }

    InputUtils::waitForEnter("Нажмите Enter для продолжения...");
}

void handleAdminCreation(UserManager& userManager, SecurityManager& security) {
    InputUtils::printHeader("Создание администратора");

    try {
        if (security.isLocked()) {
            InputUtils::printError("Система заблокирована!");
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            return;
        }

        std::string masterPass = InputValidator::getStringInput(
            "Введите мастер-пароль: ",
            nullptr,
            "",
            false
        );

        if (!security.verifyMasterPassword(masterPass)) {
            InputUtils::printError("Неверный мастер-пароль!");
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            return;
        }

        // Ввод логина администратора
        std::string username;
        bool usernameValid = false;
        int attempts = 0;

        while (!usernameValid && attempts < 3) {
            try {
                username = InputValidator::getStringInput(
                    "Логин администратора: ",
                    [&userManager](const std::string& uname) {
                        Validators::validateUsername(uname);

                        if (userManager.findUser(uname)) {
                            throw RangeException("Пользователь с таким логином уже существует!");
                        }
                        return true;
                    },
                    "Логин должен быть от 3 до 20 символов"
                );
                usernameValid = true;
            }
            catch (const InputException& e) {
                attempts++;
                InputUtils::printError(std::string(e.what()) +
                    " (Попытка " + std::to_string(attempts) + " из 3)");

                if (attempts >= 3) {
                    InputUtils::printError("Превышено количество попыток.");
                    return;
                }
            }
        }

        // Ввод пароля
        std::string password;
        try {
            password = InputValidator::getPasswordInput("Пароль: ", 6);
        }
        catch (const PasswordException& e) {
            InputUtils::printError(e.what());
            return;
        }

        // Создание администратора
        bool success = SafeExecutor::execute([&]() {
            auto admin = std::make_shared<Admin>(username, Utils::hashPassword(password));

            if (!userManager.addUser(admin)) {
                throw std::runtime_error("Ошибка при добавлении администратора");
            }

            userManager.saveUsers();
            }, "создания администратора");

        if (success) {
            InputUtils::printSuccess("Администратор успешно создан!");
        }

    }
    catch (const InputException& e) {
        InputUtils::printError("Ошибка ввода: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        InputUtils::printError("Ошибка: " + std::string(e.what()));
    }

    InputUtils::waitForEnter("Нажмите Enter для продолжения...");
}

void handleExit(UserManager& userManager, ApplicationManager& appManager,
    ScholarshipTypeManager& scholarshipManager) {

    InputUtils::printHeader("Выход");

    try {
        SafeExecutor::executeWithRetry([&userManager]() {
            userManager.saveUsers();
            }, "сохранения пользователей");

        SafeExecutor::executeWithRetry([&appManager]() {
            appManager.saveApplications();
            }, "сохранения заявок");

        SafeExecutor::executeWithRetry([&scholarshipManager]() {
            scholarshipManager.saveScholarshipTypes();
            }, "сохранения типов стипендий");

        InputUtils::printSuccess("Все данные успешно сохранены!");
        InputUtils::printSuccess("Спасибо за использование системы!");

    }
    catch (const std::exception& e) {
        InputUtils::printWarning("Внимание: " + std::string(e.what()));
        InputUtils::printWarning("Некоторые данные могли не сохраниться.");
        InputUtils::printWarning("Программа будет завершена.");
    }

    InputUtils::waitForEnter("Нажмите Enter для выхода...");
}

//STUDENT MENU 
void studentMenu(std::shared_ptr<Student> student, UserManager& userManager,
    ScholarshipTypeManager& scholarshipManager, ApplicationManager& appManager) {

    while (true) {
        try {
            std::vector<std::string> options = {
                "Подать заявку на стипендию",
                "Просмотреть информацию о стипендиях",
                "Анализ моих возможностей",
                "Мои заявки",
                "Удалить мою заявку",
                "Изменить пароль",
                "Редактировать профиль",
                "Выйти из системы"
            };

            int choice = InputValidator::getMenuChoice(
                "Меню студента: " + student->getUsername(),
                options,
                [](int val) { return val >= 1 && val <= 8; },
                "Действие должно быть от 1 до 8"
            );

            if (choice == 8) break;

            switch (choice) {
            case 1: {  // Подача заявки на стипендию
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Подача заявки на стипендию");

                    // ПРОВЕРКА: Студенты с платной формой обучения не могут подавать заявки
                    if (student->getStudyForm() == StudyForm::Paid) {
                        throw std::runtime_error("Студенты с платной формой обучения не могут подавать заявки на стипендию!");
                    }

                    // Проверяем, нет ли уже активных заявок
                    std::vector<Application> studentApps = appManager.getApplicationsByStudent(student->getUsername());
                    std::vector<Application> pendingApps;

                    // Фильтруем только заявки в статусе "Ожидание"
                    for (const auto& app : studentApps) {
                        if (app.getStatus() == ApplicationStatus::Pending) {
                            pendingApps.push_back(app);
                        }
                    }

                    if (!pendingApps.empty()) {
                        std::stringstream errorMsg;
                        errorMsg << "\n!!! ВНИМАНИЕ: У ВАС УЖЕ ЕСТЬ АКТИВНЫЕ ЗАЯВКИ !!!\n\n";
                        errorMsg << "Найдено активных заявок: " << pendingApps.size() << " (в статусе 'Ожидание')\n\n";

                        if (!pendingApps.empty()) {
                            errorMsg << "Ваши активные заявки:\n";
                            errorMsg << "----------------------------------------\n";

                            for (const auto& app : pendingApps) {
                                errorMsg << "  Заявка #" << app.getId() << "\n";
                                errorMsg << "  Тип стипендии: " << ScholarshipType::categoryToString(app.getScholarshipCategory()) << "\n";
                                errorMsg << "  Средний балл: " << app.getAverageGrade() << "\n";
                                errorMsg << "  Статус: ОЖИДАНИЕ РАССМОТРЕНИЯ\n";
                                errorMsg << "----------------------------------------\n";
                            }
                        }

                        errorMsg << "\nЧТО ДЕЛАТЬ ДАЛЬШЕ?\n";
                        errorMsg << "----------------------------------------\n";
                        errorMsg << "1. Дождитесь рассмотрения текущих заявок администрацией\n";
                        errorMsg << "   * Проверьте статус в меню 'Мои заявки'\n";
                        errorMsg << "   * Обычно рассмотрение занимает 3-5 рабочих дней\n\n";

                        errorMsg << "2. Если хотите подать новую заявку, необходимо:\n";
                        errorMsg << "   * Отозвать текущую заявку через меню 'Удалить мою заявку'\n";
                        errorMsg << "   * Или дождаться окончания рассмотрения (одобрения/отклонения)\n\n";

                        errorMsg << "3. Правила подачи заявок:\n";
                        errorMsg << "   * Один студент -> одна активная заявка\n";
                        errorMsg << "   * После решения по заявке можно подать новую\n";
                        errorMsg << "   * Отозвать можно только заявки в статусе 'Ожидание'\n";

                        throw std::runtime_error(errorMsg.str());
                    }

                    // Показываем доступные типы стипендий
                    InputUtils::printSection("ВЫБОР ТИПА СТИПЕНДИИ");

                    auto scholarshipTypes = scholarshipManager.getAllScholarshipTypes();

                    // Фильтруем стипендии, недоступные для данного студента
                    std::vector<std::shared_ptr<ScholarshipType>> availableScholarships;

                    for (const auto& type : scholarshipTypes) {
                        // Пропускаем социальную стипендию, если нет льгот
                        if (type->getCategory() == ScholarshipCategory::Social &&
                            !student->getHasSocialBenefits()) {
                            continue;
                        }

                        // Проверяем требования для этого типа стипендии
                        auto requirements = type->checkRequirements(
                            student->getAverageGrade(),
                            student->getHasSocialBenefits(),
                            student->getHasScientificWorks(),
                            student->getConferencesCount(),
                            student->getIsActiveInCommunity()
                        );

                        // Проверяем, выполняются ли все требования
                        bool allRequirementsMet = true;
                        for (const auto& req : requirements) {
                            if (!req.isMet) {
                                allRequirementsMet = false;
                                break;
                            }
                        }

                        // Дополнительные проверки для конкретных стипендий
                        if (type->getCategory() == ScholarshipCategory::Academic) {
                            if (student->getAverageGrade() < type->getMinAverageGrade()) {
                                allRequirementsMet = false;
                            }
                        }
                        else if (type->getCategory() == ScholarshipCategory::Named) {
                            // Именная стипендия: минимум 1 конференция
                            if (student->getConferencesCount() < 1) {
                                allRequirementsMet = false;
                            }
                        }
                        else if (type->getCategory() == ScholarshipCategory::Personal ||
                            type->getCategory() == ScholarshipCategory::Presidential) {
                            // Персональная и президентская: минимум 3 конференции
                            if (student->getConferencesCount() < 3) {
                                allRequirementsMet = false;
                            }
                        }

                        if (allRequirementsMet) {
                            availableScholarships.push_back(type);
                        }
                    }

                    if (availableScholarships.empty()) {
                        InputUtils::printError("Вы не соответствуете требованиям ни для одного типа стипендии!");
                        std::cout << "\nТребования по стипендиям:\n";

                        for (const auto& type : scholarshipTypes) {
                            InputUtils::printDivider();
                            std::cout << type->getName() << ":\n";
                            auto requirements = type->checkRequirements(
                                student->getAverageGrade(),
                                student->getHasSocialBenefits(),
                                student->getHasScientificWorks(),
                                student->getConferencesCount(),
                                student->getIsActiveInCommunity()
                            );

                            for (const auto& req : requirements) {
                                std::cout << "  * " << req.description << ": "
                                    << (req.isMet ? "[+] ВЫПОЛНЕНО" : "[-] НЕ ВЫПОЛНЕНО") << "\n";
                            }

                            // Дополнительная информация по конференциям
                            if (type->getCategory() == ScholarshipCategory::Named) {
                                std::cout << "  * Требуется конференций: минимум 1 (ваши: "
                                    << student->getConferencesCount() << ")\n";
                            }
                            else if (type->getCategory() == ScholarshipCategory::Personal ||
                                type->getCategory() == ScholarshipCategory::Presidential) {
                                std::cout << "  * Требуется конференций: минимум 3 (ваши: "
                                    << student->getConferencesCount() << ")\n";
                            }

                            if (type->getCategory() == ScholarshipCategory::Academic) {
                                std::cout << "  * Минимальный средний балл: " << type->getMinAverageGrade()
                                    << " (ваш: " << student->getAverageGrade() << ")\n";
                            }
                        }

                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    // Выводим доступные стипендии
                    std::cout << "Доступные для вас стипендии:\n";
                    for (size_t i = 0; i < availableScholarships.size(); ++i) {
                        const auto& type = availableScholarships[i];
                        std::cout << i + 1 << ". " << type->getName() << "\n";
                        std::cout << "   Описание: " << type->getDescription() << "\n";

                        if (type->getCategory() == ScholarshipCategory::Academic) {
                            std::cout << "   Минимальный балл: " << type->getMinAverageGrade()
                                << " (ваш: " << student->getAverageGrade() << ")\n";
                        }
                        else if (type->getCategory() == ScholarshipCategory::Named) {
                            std::cout << "   Требуется: балл >= 8.5, научная работа, 1+ конференция\n"; 
                        }
                        else if (type->getCategory() == ScholarshipCategory::Personal) {
                            std::cout << "   Требуется: балл >= 8.0, общественная активность, 3+ конференции\n"; 
                        }
                        else if (type->getCategory() == ScholarshipCategory::Presidential) {
                            std::cout << "   Требуется: балл >= 9.0, научная работа, общественная активность, 3+ конференции\n";  
                        }

                        InputUtils::printDivider();
                    }

                    std::cout << "0. Отмена\n";
                    int scholarshipChoice = InputValidator::getIntInput(
                        "\nВыберите тип стипендии (введите номер): ",
                        [&availableScholarships](int val) {
                            return val >= 0 && val <= static_cast<int>(availableScholarships.size());
                        },
                        "Неверный выбор!"
                    );

                    if (scholarshipChoice == 0) {
                        InputUtils::printInfo("Подача заявки отменена.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    auto selectedScholarship = availableScholarships[scholarshipChoice - 1];

                    InputUtils::printSection("ПОДТВЕРЖДЕНИЕ ВЫБОРА");
                    std::cout << "Вы выбрали: " << selectedScholarship->getName() << "\n";
                    std::cout << "Описание: " << selectedScholarship->getDescription() << "\n";
                    std::cout << "Ваш средний балл: " << student->getAverageGrade() << "\n";
                    std::cout << "Ваши научные работы: " << (student->getHasScientificWorks() ? "Есть" : "Нет") << "\n";
                    std::cout << "Ваши конференции: " << student->getConferencesCount() << "\n";
                    std::cout << "Общественная активность: " << (student->getIsActiveInCommunity() ? "Есть" : "Нет") << "\n\n";

                    if (!InputValidator::confirmAction("Подать заявку на стипендию \"" + selectedScholarship->getName() + "\"?")) {
                        InputUtils::printInfo("Подача заявки отменена.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    try {
                        Application newApp(student->getUsername(),
                            student->getAverageGrade(),
                            selectedScholarship->getCategory(),
                            ApplicationStatus::Pending);

                        if (appManager.addApplication(newApp)) {
                            appManager.saveApplications();
                            InputUtils::printSuccess("Заявка #" + std::to_string(newApp.getId()) +
                                " на стипендию \"" + selectedScholarship->getName() +
                                "\" успешно подана!");
                        }
                        else {
                            throw std::runtime_error("Не удалось добавить заявку в систему. Возможно, заявка с таким ID уже существует.");
                        }
                    }
                    catch (const std::exception& e) {
                        throw std::runtime_error("Ошибка при создании заявки: " + std::string(e.what()));
                    }
                    }, "подачи заявки на стипендию");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 2: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Информация о видах стипендий");

                    std::vector<std::shared_ptr<ScholarshipType>> types = scholarshipManager.getAllScholarshipTypes();

                    for (const auto& type : types) {
                        InputUtils::printSection(type->getName());
                        std::cout << "Описание: " << type->getDescription() << "\n";
                        std::cout << "Периодичность пересчета: " << type->getRecalculationPeriod() << "\n";

                        if (type->getCategory() == ScholarshipCategory::Academic) {
                            std::cout << "Минимальный средний балл: " << type->getMinAverageGrade() << "\n";
                        }

                        // Проверяем соответствие студента требованиям
                        std::vector<ScholarshipRequirement> requirements = type->checkRequirements(
                            student->getAverageGrade(),
                            student->getHasSocialBenefits(),
                            student->getHasScientificWorks(),
                            student->getConferencesCount(),
                            student->getIsActiveInCommunity()
                        );

                        std::cout << "Требования (для вас):\n";
                        for (const auto& req : requirements) {
                            std::cout << "  • " << req.description << ": "
                                << (req.isMet ? "[+] ВЫПОЛНЕНО" : "[-] НЕ ВЫПОЛНЕНО") << "\n"; 
                        }

                        InputUtils::printDivider();
                    }

                    // Анализ возможностей
                    InputUtils::printSection("АНАЛИЗ ВАШИХ ВОЗМОЖНОСТЕЙ");

                    std::shared_ptr<ScholarshipType> academicScholarship = scholarshipManager.getAcademicScholarship();
                    if (academicScholarship) {
                        double requiredGrade = academicScholarship->getMinAverageGrade();
                        double currentGrade = student->getAverageGrade();

                        if (currentGrade >= requiredGrade) {
                            InputUtils::printSuccess("Вы соответствуете критериям учебной стипендии!");
                        }
                        else {
                            InputUtils::printInfo("Для учебной стипендии нужно повысить средний балл на "
                                + std::to_string(requiredGrade - currentGrade));
                        }
                    }
                    }, "просмотра информации о стипендиях");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 3: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Анализ моих возможностей для получения стипендий");

                    std::string analysis = scholarshipManager.getStudentScholarshipInfo(
                        student->getAverageGrade(),
                        student->getHasSocialBenefits(),
                        student->getHasScientificWorks(),
                        student->getConferencesCount(),
                        student->getIsActiveInCommunity()
                    );

                    std::cout << analysis;

                    InputUtils::printSection("РЕКОМЕНДАЦИИ ДЛЯ УЛУЧШЕНИЯ ПРОФИЛЯ");
                    std::vector<std::string> recommendations;

                    std::vector<std::shared_ptr<ScholarshipType>> types = scholarshipManager.getAllScholarshipTypes();

                    for (const auto& type : types) {
                        auto requirements = type->checkRequirements(
                            student->getAverageGrade(),
                            student->getHasSocialBenefits(),
                            student->getHasScientificWorks(),
                            student->getConferencesCount(),
                            student->getIsActiveInCommunity()
                        );

                        for (const auto& req : requirements) {
                            if (!req.isMet) {
                                std::string rec = "Для " + type->getName() + ": " + req.description;
                                recommendations.push_back(rec);
                            }
                        }
                    }

                    if (recommendations.empty()) {
                        InputUtils::printSuccess("Вы соответствуете всем основным требованиям! Подавайте заявки.");
                    }
                    else {
                        std::cout << "Чтобы повысить шансы на получение стипендий:\n";
                        for (size_t i = 0; i < recommendations.size(); ++i) {
                            std::cout << i + 1 << ". " << recommendations[i] << "\n";
                        }
                    }
                    }, "анализа возможностей");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 4: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Мои заявки");
                    std::vector<Application> apps = appManager.getApplicationsByStudent(student->getUsername());

                    if (apps.empty()) {
                        InputUtils::printInfo("У вас пока нет заявок.");
                    }
                    else {
                        std::cout << "Всего заявок: " << apps.size() << "\n";
                        InputUtils::printDivider();

                        for (const auto& app : apps) {
                            std::cout << "Заявка #" << app.getId() << "\n";
                            std::cout << "  Тип стипендии: " << ScholarshipType::categoryToString(app.getScholarshipCategory()) << "\n";
                            std::cout << "  Средний балл: " << app.getAverageGrade() << "\n";
                            std::cout << "  Статус: " << Utils::statusToString(static_cast<int>(app.getStatus())) << "\n";
                            InputUtils::printDivider();
                        }
                    }
                    }, "просмотра заявок");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 5: {
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Удаление заявки");
                    std::vector<Application> apps = appManager.getApplicationsByStudent(student->getUsername());

                    if (apps.empty()) {
                        InputUtils::printInfo("У вас нет заявок для удаления.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    // Показываем только заявки в статусе "Ожидание"
                    std::vector<Application> pendingApps;
                    for (const auto& app : apps) {
                        if (app.getStatus() == ApplicationStatus::Pending) {
                            pendingApps.push_back(app);
                        }
                    }

                    if (pendingApps.empty()) {
                        InputUtils::printInfo("У вас нет заявок в статусе 'Ожидание'.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    std::cout << "Ваши заявки для удаления:\n";
                    InputUtils::printDivider();
                    for (const auto& app : pendingApps) {
                        std::cout << "ID: " << app.getId() << " | Балл: " << app.getAverageGrade() << "\n";
                    }
                    InputUtils::printDivider();

                    int appId = InputValidator::getIntInput(
                        "Введите ID заявки для удаления (0 для отмены): ",
                        [](int val) { return val >= 0; },
                        "ID должен быть неотрицательным"
                    );

                    if (appId == 0) {
                        InputUtils::printInfo("Удаление отменено.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    // Проверяем, принадлежит ли заявка студенту и в правильном ли она статусе
                    bool found = false;
                    for (const auto& app : pendingApps) {
                        if (app.getId() == appId) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        throw std::runtime_error("Заявка не найдена или недоступна для удаления.");
                    }

                   
                    if (!InputValidator::confirmAction("Удалить заявку #" + std::to_string(appId) + "?")) {
                        InputUtils::printInfo("Удаление отменено.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    if (appManager.removeApplicationById(appId, "student_" + student->getUsername())) {
                        appManager.saveApplications();
                        InputUtils::printSuccess("Заявка #" + std::to_string(appId) + " успешно удалена!");
                    }
                    else {
                        throw std::runtime_error("Ошибка при удалении заявки.");
                    }
                    }, "удаления заявки");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 6: {
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Смена пароля");

                    std::string oldPassword = InputValidator::getStringInput(
                        "Введите текущий пароль: ",
                        nullptr,
                        "",
                        false
                    );

                    if (student->getPasswordHash() != Utils::hashPassword(oldPassword)) {
                        throw std::runtime_error("Неверный текущий пароль!");
                    }

                    std::string newPassword = InputValidator::getPasswordInput("Введите новый пароль: ", 6);

                    if (!InputValidator::confirmAction("Изменить пароль?")) {
                        InputUtils::printInfo("Смена пароля отменена.");
                        return;
                    }

                    if (userManager.updateUserPassword(student->getUsername(), Utils::hashPassword(newPassword))) {
                        userManager.saveUsers();
                        InputUtils::printSuccess("Пароль успешно изменен!");
                    }
                    else {
                        throw std::runtime_error("Ошибка при изменении пароля.");
                    }
                    }, "смены пароля");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 7: {  // Редактирование профиля
                SafeExecutor::execute([&]() {
                    bool editing = true;
                    while (editing) {
                        InputUtils::printHeader("Редактирование профиля: " + student->getFio());

                        std::cout << "=== ВАШИ ТЕКУЩИЕ ДАННЫЕ ===\n\n";
                        std::cout << "ОСНОВНАЯ ИНФОРМАЦИЯ:\n";
                        std::cout << "  [1] ФИО: " << student->getFio() << "\n";
                        std::cout << "  [2] Форма обучения: " << student->getStudyFormString() << "\n";
                        std::cout << "  [3] Курс: " << student->getCourse() << "\n";
                        std::cout << "  [4] Группа: " << student->getGroup() << "\n";
                        std::cout << "  [5] Факультет: " << student->getFaculty() << "\n";
                        std::cout << "  [6] Специальность: " << student->getSpecialty() << "\n\n";
                        std::cout << "АКАДЕМИЧЕСКАЯ ИНФОРМАЦИЯ:\n";
                        std::cout << "  [7] Средний балл: " << std::fixed << std::setprecision(2)
                            << student->getAverageGrade() << "/10.0\n";
                        std::cout << "  [8] Социальные льготы: " << (student->getHasSocialBenefits() ? "Да" : "Нет");
                        if (!student->getSocialBenefitsComment().empty()) {
                            std::cout << " (комментарий: " << student->getSocialBenefitsComment() << ")";
                        }
                        std::cout << "\n";

                        std::cout << "  [9] Научные работы: " << (student->getHasScientificWorks() ? "Да" : "Нет");
                        if (!student->getScientificWorksComment().empty()) {
                            std::cout << " (комментарий: " << student->getScientificWorksComment() << ")";
                        }
                        std::cout << "\n";

                        std::cout << "  [10] Участие в конференциях: " << student->getConferencesCount() << " раз(а)";
                        if (!student->getConferencesComment().empty()) {
                            std::cout << " (комментарий: " << student->getConferencesComment() << ")";
                        }
                        std::cout << "\n";

                        std::cout << "  [11] Общественная активность: " << (student->getIsActiveInCommunity() ? "Да" : "Нет");
                        if (!student->getCommunityActivityComment().empty()) {
                            std::cout << " (комментарий: " << student->getCommunityActivityComment() << ")";
                        }
                        std::cout << "\n\n";
                        std::cout << "КОММЕНТАРИИ:\n";
                        std::cout << "  [12] Комментарий к льготам: "
                            << (student->getSocialBenefitsComment().empty() ? "не указан" : student->getSocialBenefitsComment()) << "\n";
                        std::cout << "  [13] Комментарий к научным работам: "
                            << (student->getScientificWorksComment().empty() ? "не указан" : student->getScientificWorksComment()) << "\n";
                        std::cout << "  [14] Комментарий к конференциям: "
                            << (student->getConferencesComment().empty() ? "не указан" : student->getConferencesComment()) << "\n";
                        std::cout << "  [15] Комментарий к общественной активности: "
                            << (student->getCommunityActivityComment().empty() ? "не указан" : student->getCommunityActivityComment()) << "\n";

                        InputUtils::printDivider();
                        std::cout << "=== РЕДАКТИРОВАНИЕ ===\n";
                        std::cout << "Выберите номер пункта для редактирования:\n";
                        std::cout << "  1-15 - Редактировать соответствующий пункт\n";
                        std::cout << "  16   - Завершить редактирование\n\n";

                        int editChoice = InputValidator::getIntInput(
                            "Ваш выбор: ",
                            [](int val) { return val >= 1 && val <= 16; },
                            "Введите число от 1 до 16"
                        );

                        if (editChoice == 16) {
                            InputUtils::printInfo("Редактирование завершено.");
                            editing = false;
                            continue;
                        }
                        InputUtils::clearScreen();
                        InputUtils::printHeader("Редактирование профиля");

                        switch (editChoice) {
                        case 1: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ ФИО");
                            std::cout << "Текущее ФИО: " << student->getFio() << "\n\n";

                            std::string newFio = InputValidator::getLineInput(
                                "Введите новое ФИО: ",
                                Validators::validateFIO,
                                "ФИО должно быть от 5 до 30 символов",
                                false,
                                5,
                                100
                            );

                            student->setFio(newFio);
                            InputUtils::printSuccess("ФИО успешно обновлено!");
                            break;
                        }

                        case 2: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ ФОРМЫ ОБУЧЕНИЯ");
                            std::cout << "Текущая форма: " << student->getStudyFormString() << "\n\n";

                            std::cout << "Выберите форму обучения:\n";
                            std::cout << "1. Бюджетная\n";
                            std::cout << "2. Платная\n\n";

                            int formChoice = InputValidator::getIntInput(
                                "Ваш выбор (1 или 2): ",
                                [](int val) { return val == 1 || val == 2; },
                                "Введите 1 или 2"
                            );

                            StudyForm newForm = (formChoice == 2) ? StudyForm::Paid : StudyForm::Budget;
                            student->setStudyForm(newForm);
                            InputUtils::printSuccess("Форма обучения успешно обновлена!");
                            break;
                        }

                        case 3: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ КУРСА");
                            std::cout << "Текущий курс: " << student->getCourse() << "\n\n";

                            int newCourse = InputValidator::getIntInput(
                                "Введите новый курс (1-4): ",
                                Validators::validateCourse,
                                "Курс должен быть от 1 до 4"
                            );

                            student->setCourse(newCourse);
                            InputUtils::printSuccess("Курс успешно обновлен!");
                            break;
                        }

                        case 4: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ ГРУППЫ");
                            std::cout << "Текущая группа: " << student->getGroup() << "\n\n";

                            std::string newGroup = InputValidator::getLineInput(
                                "Введите новую группу (6 цифр): ",
                                Validators::validateGroup,
                                "Номер группы должен состоять из 6 цифр",
                                false
                            );

                            student->setGroup(newGroup);
                            InputUtils::printSuccess("Группа успешно обновлена!");
                            break;
                        }

                        case 5: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ ФАКУЛЬТЕТА");
                            std::cout << "Текущий факультет: " << student->getFaculty() << "\n\n";

                            std::string newFaculty = InputValidator::getLineInput(
                                "Введите новый факультет (2-5 букв): ",
                                Validators::validateFaculty,
                                "Название факультета должно быть от 2 до 5 символов и содержать только буквы",
                                false
                            );

                            student->setFaculty(newFaculty);
                            InputUtils::printSuccess("Факультет успешно обновлен!");
                            break;
                        }

                        case 6: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ СПЕЦИАЛЬНОСТИ");
                            std::cout << "Текущая специальность: " << student->getSpecialty() << "\n\n";

                            std::string newSpecialty = InputValidator::getLineInput(
                                "Введите новую специальность (1-7 букв): ",
                                Validators::validateSpecialty,
                                "Название специальности должно быть от 1 до 7 символов и содержать только буквы",
                                false
                            );

                            student->setSpecialty(newSpecialty);
                            InputUtils::printSuccess("Специальность успешно обновлена!");
                            break;
                        }

                        case 7: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ СРЕДНЕГО БАЛЛА");
                            std::cout << "Текущий средний балл: " << student->getAverageGrade() << "/10.0\n\n";

                            double newAvg = InputValidator::getDoubleInput(
                                "Введите новый средний балл (0.0-10.0): ",
                                Validators::validateGrade,
                                "Балл должен быть от 0.0 до 10.0"
                            );

                            student->setAverageGrade(newAvg);
                            InputUtils::printSuccess("Средний балл успешно обновлен!");
                            break;
                        }

                        case 8: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ СОЦИАЛЬНЫХ ЛЬГОТ");
                            std::cout << "Текущий статус: " << (student->getHasSocialBenefits() ? "Есть льготы" : "Нет льгот") << "\n";
                            std::cout << "Комментарий: " <<
                                (student->getSocialBenefitsComment().empty() ? "нет" : student->getSocialBenefitsComment()) << "\n\n";

                            bool newValue = InputValidator::getYesNoInput("Есть социальные льготы? ");
                            student->setHasSocialBenefits(newValue);
                            if (InputValidator::getYesNoInput("Добавить/изменить комментарий к льготам?")) {
                                std::string comment = InputValidator::getLineInput(
                                    "Введите комментарий (оставьте пустым для удаления): ",
                                    nullptr,
                                    "",
                                    true,
                                    0,
                                    500
                                );
                                student->setSocialBenefitsComment(comment);
                            }

                            InputUtils::printSuccess("Информация о льготах успешно обновлена!");
                            break;
                        }

                        case 9: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ НАУЧНЫХ РАБОТ");
                            std::cout << "Текущий статус: " << (student->getHasScientificWorks() ? "Есть научные работы" : "Нет научных работ") << "\n";
                            std::cout << "Комментарий: " <<
                                (student->getScientificWorksComment().empty() ? "нет" : student->getScientificWorksComment()) << "\n\n";

                            bool newValue = InputValidator::getYesNoInput("Есть научные работы? ");
                            student->setHasScientificWorks(newValue);

                            if (InputValidator::getYesNoInput("Добавить/изменить комментарий к научным работам?")) {
                                std::string comment = InputValidator::getLineInput(
                                    "Введите комментарий (оставьте пустым для удаления): ",
                                    nullptr,
                                    "",
                                    true,
                                    0,
                                    500
                                );
                                student->setScientificWorksComment(comment);
                            }

                            InputUtils::printSuccess("Информация о научных работах успешно обновлена!");
                            break;
                        }

                        case 10: { 
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ КОНФЕРЕНЦИЙ");
                            std::cout << "Текущее количество: " << student->getConferencesCount() << " конференций\n";
                            std::cout << "Комментарий: " <<
                                (student->getConferencesComment().empty() ? "нет" : student->getConferencesComment()) << "\n\n";

                            int confs = InputValidator::getIntInput(
                                "Введите новое количество конференций: ",
                                Validators::validateConferences,
                                "Количество не может быть отрицательным"
                            );

                            student->setConferencesCount(confs);
                            if (InputValidator::getYesNoInput("Добавить/изменить комментарий к конференциям?")) {
                                std::string comment = InputValidator::getLineInput(
                                    "Введите комментарий (оставьте пустым для удаления): ",
                                    nullptr,
                                    "",
                                    true,
                                    0,
                                    500
                                );
                                student->setConferencesComment(comment);
                            }

                            InputUtils::printSuccess("Информация о конференциях успешно обновлена!");
                            break;
                        }

                        case 11: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ ОБЩЕСТВЕННОЙ АКТИВНОСТИ");
                            std::cout << "Текущий статус: " << (student->getIsActiveInCommunity() ? "Активен" : "Не активен") << "\n";
                            std::cout << "Комментарий: " <<
                                (student->getCommunityActivityComment().empty() ? "нет" : student->getCommunityActivityComment()) << "\n\n";

                            bool newValue = InputValidator::getYesNoInput("Активен в общественной жизни? ");
                            student->setIsActiveInCommunity(newValue);
                            if (InputValidator::getYesNoInput("Добавить/изменить комментарий к общественной активности?")) {
                                std::string comment = InputValidator::getLineInput(
                                    "Введите комментарий (оставьте пустым для удаления): ",
                                    nullptr,
                                    "",
                                    true,
                                    0,
                                    500
                                );
                                student->setCommunityActivityComment(comment);
                            }
                            InputUtils::printSuccess("Информация об общественной активности успешно обновлена!");
                            break;
                        }
                        case 12: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ КОММЕНТАРИЯ К ЛЬГОТАМ");
                            std::cout << "Текущий комментарий: " <<
                                (student->getSocialBenefitsComment().empty() ? "не указан" : student->getSocialBenefitsComment()) << "\n\n";

                            std::string comment = InputValidator::getLineInput(
                                "Введите новый комментарий (оставьте пустым для удаления): ",
                                nullptr,
                                "",
                                true,
                                0,
                                500
                            );

                            student->setSocialBenefitsComment(comment);
                            InputUtils::printSuccess("Комментарий к льготам успешно обновлен!");
                            break;
                        }

                        case 13: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ КОММЕНТАРИЯ К НАУЧНЫМ РАБОТАМ");
                            std::cout << "Текущий комментарий: " <<
                                (student->getScientificWorksComment().empty() ? "не указан" : student->getScientificWorksComment()) << "\n\n";

                            std::string comment = InputValidator::getLineInput(
                                "Введите новый комментарий (оставьте пустым для удаления): ",
                                nullptr,
                                "",
                                true,
                                0,
                                500
                            );

                            student->setScientificWorksComment(comment);
                            InputUtils::printSuccess("Комментарий к научным работам успешно обновлен!");
                            break;
                        }

                        case 14: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ КОММЕНТАРИЯ К КОНФЕРЕНЦИЯМ");
                            std::cout << "Текущий комментарий: " <<
                                (student->getConferencesComment().empty() ? "не указан" : student->getConferencesComment()) << "\n\n";

                            std::string comment = InputValidator::getLineInput(
                                "Введите новый комментарий (оставьте пустым для удаления): ",
                                nullptr,
                                "",
                                true,
                                0,
                                500
                            );

                            student->setConferencesComment(comment);
                            InputUtils::printSuccess("Комментарий к конференциям успешно обновлен!");
                            break;
                        }

                        case 15: {  
                            InputUtils::printSection("РЕДАКТИРОВАНИЕ КОММЕНТАРИЯ К ОБЩЕСТВЕННОЙ АКТИВНОСТИ");
                            std::cout << "Текущий комментарий: " <<
                                (student->getCommunityActivityComment().empty() ? "не указан" : student->getCommunityActivityComment()) << "\n\n";

                            std::string comment = InputValidator::getLineInput(
                                "Введите новый комментарий (оставьте пустым для удаления): ",
                                nullptr,
                                "",
                                true,
                                0,
                                500
                            );

                            student->setCommunityActivityComment(comment);
                            InputUtils::printSuccess("Комментарий к общественной активности успешно обновлен!");
                            break;
                        }
                        }
                        userManager.saveUsers();
                        InputUtils::printDivider();
                        InputUtils::printInfo("Данные обновлены. Нажмите Enter для продолжения...");
                        std::cin.ignore();
                        std::cin.get();
                        InputUtils::clearScreen();
                    }
                    }, "редактирования профиля");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            default:
                InputUtils::printError("Неверный выбор меню.");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

        }
        catch (const InputException& e) {
            InputUtils::printError("Ошибка ввода: " + std::string(e.what()));

            if (!InputValidator::getYesNoInput("Продолжить работу?")) {
                break;
            }
        }
        catch (const std::exception& e) {
            InputUtils::printError("Ошибка: " + std::string(e.what()));
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
        }
    }
}

// ADMIN MENU 
void adminMenu(UserManager& userManager, ScholarshipTypeManager& scholarshipManager,
    ApplicationManager& appManager, SecurityManager& security,
    const std::string& adminUsername) {

    while (true) {
        try {
            std::vector<std::string> options = {
                "Управление критериями стипендий",
                "Поиск заявок",
                "Просмотр и модерация заявок",
                "Управление заявками (удаление)",
                "Управление пользователями",
                "Работа со студентами",
                "Просмотр истории заявок",
                "Сменить мастер-пароль",
                "Редактировать свой аккаунт",
                "Выйти из системы"
            };

            int choice = InputValidator::getMenuChoice(
                "Меню администратора: " + adminUsername,
                options,
                [](int val) { return val >= 1 && val <= 10; },
                "Действие должно быть от 1 до 10"
            );

            if (choice == 10) break;

            switch (choice) {
            case 1: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Управление критериями стипендий");

                    auto types = scholarshipManager.getAllScholarshipTypes();

                    std::cout << "Выберите вид стипендии для настройки:\n";
                    for (size_t i = 0; i < types.size(); ++i) {
                        std::cout << i + 1 << ". " << types[i]->getName() << "\n";
                    }
                    std::cout << "0. Назад\n";

                    int typeChoice = InputValidator::getIntInput(
                        "Ваш выбор: ",
                        [&types](int val) { return val >= 0 && val <= (int)types.size(); },
                        "Неверный выбор"
                    );

                    if (typeChoice == 0) {
                        return;
                    }

                    auto selectedType = types[typeChoice - 1];
                    InputUtils::printHeader("Настройка: " + selectedType->getName());

                    if (selectedType->getCategory() == ScholarshipCategory::Academic) {
                        std::cout << "Текущий минимальный балл: " << selectedType->getMinAverageGrade() << "\n";
                        double newMin = InputValidator::getDoubleInput(
                            "Новый минимальный балл (0.0-10.0): ",
                            Validators::validateGrade,
                            "Балл должен быть от 0.0 до 10.0"
                        );

                        if (scholarshipManager.updateScholarshipTypeManager(selectedType->getCategory(), newMin)) {
                            InputUtils::printSuccess("Критерии обновлены!");
                        }
                        else {
                            throw std::runtime_error("Ошибка при обновлении критериев");
                        }
                    }
                    else {
                        std::cout << "Описание: " << selectedType->getDescription() << "\n\n";
                        std::cout << "Периодичность пересчета: " << selectedType->getRecalculationPeriod() << "\n\n";
                        std::cout << "Требования:\n";
                        auto reqs = selectedType->getRequirements();
                        for (const auto& req : reqs) {
                            std::cout << "  • " << req.description << "\n";
                        }

                        std::cout << "\nПримечание: Критерии этой стипендии фиксированы.\n";
                    }
                    }, "управления критериями стипендий");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }
            case 2: {  
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Поиск заявки по ID");

                    std::vector<std::string> searchOptions = {
                        "Поиск по ID заявки",
                        "Поиск по логину студента",
                        "Назад"
                    };

                    int searchChoice = InputValidator::getMenuChoice(
                        "Выберите тип поиска",
                        searchOptions,
                        [](int val) { return val >= 1 && val <= 3; },
                        "Выберите от 1 до 3"
                    );

                    if (searchChoice == 3) {
                        return;
                    }

                    switch (searchChoice) {
                    case 1: {  
                        int appId = InputValidator::getIntInput(
                            "Введите ID заявки: ",
                            [](int val) { return val > 0; },
                            "ID должен быть положительным числом"
                        );

                        const Application* app = appManager.getApplicationById(appId);

                        if (!app) {
                            InputUtils::printError("Заявка с ID " + std::to_string(appId) + " не найдена.");
                        }
                        else {
                            InputUtils::printHeader("НАЙДЕНА ЗАЯВКА #" + std::to_string(appId));

                            // Получаем информацию о студенте
                            auto student = userManager.findUser(app->getStudentUsername());
                            std::string studentInfo = "Не найден";
                            if (student) {
                                studentInfo = std::static_pointer_cast<Student>(student)->getFio();
                            }

                            std::cout << "ID: " << app->getId() << "\n";
                            std::cout << "Студент: " << app->getStudentUsername() << " (" << studentInfo << ")\n";
                            std::cout << "Средний балл: " << app->getAverageGrade() << "\n";
                            std::cout << "Тип стипендии: " << ScholarshipType::categoryToString(app->getScholarshipCategory()) << "\n";
                            std::cout << "Статус: " << Utils::statusToString(static_cast<int>(app->getStatus())) << "\n";
                            InputUtils::printDivider();
                            std::vector<std::string> actionOptions = {
                                "Изменить статус заявки",
                                "Удалить заявку",
                                "Назад"
                            };

                            int action = InputValidator::getMenuChoice(
                                "Действия с заявкой",
                                actionOptions,
                                [](int val) { return val >= 1 && val <= 3; },
                                "Выберите от 1 до 3"
                            );

                            if (action == 1) {  // Изменить статус
                                std::vector<std::string> statusOptions = {
                                    "Одобрить",
                                    "Отклонить",
                                    "Вернуть в ожидание",
                                    "Отмена"
                                };

                                int statusChoice = InputValidator::getMenuChoice(
                                    "Выберите новый статус",
                                    statusOptions,
                                    [](int val) { return val >= 1 && val <= 4; },
                                    "Выберите от 1 до 4"
                                );

                                if (statusChoice == 4) return;

                                ApplicationStatus newStatus;
                                if (statusChoice == 1) newStatus = ApplicationStatus::Approved;
                                else if (statusChoice == 2) newStatus = ApplicationStatus::Rejected;
                                else newStatus = ApplicationStatus::Pending;

                                if (appManager.updateApplicationStatusById(appId, newStatus, adminUsername)) {
                                    InputUtils::printSuccess("Статус заявки обновлен!");
                                    appManager.saveApplications();
                                }
                            }
                            else if (action == 2) {  // Удалить заявку
                                if (InputValidator::confirmAction("Удалить заявку #" + std::to_string(appId) + "?")) {
                                    if (appManager.removeApplicationById(appId, adminUsername)) {
                                        InputUtils::printSuccess("Заявка удалена!");
                                        appManager.saveApplications();
                                    }
                                }
                            }
                        }
                        break;
                    }

                    case 2: {  // Поиск по логину студента
                        std::string username = InputValidator::getStringInput(
                            "Введите логин студента: ",
                            Validators::validateUsername,
                            "Логин должен быть от 3 до 20 символов",
                            false
                        );

                        auto apps = appManager.getApplicationsByStudent(username);

                        if (apps.empty()) {
                            InputUtils::printInfo("У студента " + username + " нет заявок.");
                        }
                        else {
                            InputUtils::printHeader("ЗАЯВКИ СТУДЕНТА: " + username);
                            std::cout << "Найдено заявок: " << apps.size() << "\n";
                            InputUtils::printDivider();

                            for (const auto& app : apps) {
                                std::cout << "ID: " << app.getId()
                                    << " | Балл: " << app.getAverageGrade()
                                    << " | Тип: " << ScholarshipType::categoryToString(app.getScholarshipCategory())
                                    << " | Статус: " << Utils::statusToString(static_cast<int>(app.getStatus()))
                                    << "\n";
                                InputUtils::printDivider();
                            }
                        }
                        break;
                    }
                    }
                    }, "поиска заявок");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 3: {  // Просмотр и модерация заявок
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Модерация заявок");
                    auto apps = appManager.getAllApplications();

                    if (apps.empty()) {
                        InputUtils::printInfo("Нет заявок для отображения.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    std::vector<Application> sortedApps = apps;
                    std::sort(sortedApps.begin(), sortedApps.end(),
                        [](const Application& a, const Application& b) {
                            return a.getAverageGrade() > b.getAverageGrade();
                        });

                    std::cout << "Всего заявок: " << sortedApps.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& app : sortedApps) {
                        std::cout << app << "\n";
                        InputUtils::printDivider();
                    }

                    int appId = InputValidator::getIntInput(
                        "\nВведите ID заявки для модерации (0 для отмены): ",
                        [](int val) { return val >= 0; },
                        "ID должен быть неотрицательным"
                    );

                    if (appId == 0) {
                        InputUtils::printInfo("Модерация отменена.");
                        InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                        return;
                    }

                    Application* appPtr = appManager.getApplicationById(appId);
                    if (!appPtr) {
                        throw std::runtime_error("Заявка с ID " + std::to_string(appId) + " не найдена.");
                    }

                    auto stu = std::static_pointer_cast<Student>(userManager.findUser(appPtr->getStudentUsername()));
                    if (!stu) {
                        throw std::runtime_error("Студент " + appPtr->getStudentUsername() + " не найден.");
                    }

                    InputUtils::printHeader("Модерация заявки #" + std::to_string(appId));
                    std::cout << "Студент: " << stu->getFio() << " (" << stu->getUsername() << ")\n";
                    std::cout << "Средний балл: " << appPtr->getAverageGrade() << "\n";
                    std::cout << "Текущий статус: " << Utils::statusToString(static_cast<int>(appPtr->getStatus())) << "\n";
                    InputUtils::printDivider();

                    std::vector<std::string> actionOptions = {
                        "Одобрить заявку",
                        "Отклонить заявку",
                        "Отмена"
                    };

                    int action = InputValidator::getMenuChoice(
                        "Выберите действие",
                        actionOptions,
                        [](int val) { return val >= 1 && val <= 3; },
                        "Выберите от 1 до 3"
                    );

                    if (action == 1 || action == 2) {
                        ApplicationStatus newStatus = (action == 1) ? ApplicationStatus::Approved : ApplicationStatus::Rejected;

                        std::string actionText = (action == 1) ? "одобрить" : "отклонить";
                        if (!InputValidator::confirmAction("Вы уверены, что хотите " + actionText + " эту заявку?")) {
                            InputUtils::printInfo("Действие отменено.");
                            return;
                        }

                        if (appManager.updateApplicationStatusById(appId, newStatus, adminUsername)) {
                            appManager.saveApplications();

                            if (action == 1) {
                                stu->setHasScholarship(true);
                                InputUtils::printSuccess("Заявка одобрена! Стипендия назначена.");
                            }
                            else {
                                stu->setHasScholarship(false);
                                InputUtils::printSuccess("Заявка отклонена.");
                            }

                            userManager.saveUsers();
                        }
                        else {
                            throw std::runtime_error("Ошибка при обновлении статуса заявки.");
                        }
                    }
                    else {
                        InputUtils::printInfo("Действие отменено.");
                    }
                    }, "модерации заявок");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 4: {  // Управление заявками (удаление)
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Управление заявками");

                    std::vector<std::string> actionOptions = {
                        "Удалить заявку по ID",
                        "Удалить все заявки студента",
                        "Просмотреть все заявки"
                    };

                    int action = InputValidator::getMenuChoice(
                        "Выберите действие",
                        actionOptions,
                        [](int val) { return val >= 1 && val <= 3; },
                        "Выберите от 1 до 3"
                    );

                    switch (action) {
                    case 1: {  // Удалить заявку по ID
                        int appId = InputValidator::getIntInput(
                            "Введите ID заявки для удаления: ",
                            Validators::validateId,
                            "ID должен быть положительным числом"
                        );

                        Application* app = appManager.getApplicationById(appId);

                        if (!app) {
                            throw std::runtime_error("Заявка с ID " + std::to_string(appId) + " не найдена.");
                        }

                        std::cout << "\nИнформация о заявке:\n";
                        std::cout << "ID: " << app->getId() << "\n";
                        std::cout << "Студент: " << app->getStudentUsername() << "\n";
                        std::cout << "Балл: " << app->getAverageGrade() << "\n";
                        std::cout << "Статус: " << Utils::statusToString(static_cast<int>(app->getStatus())) << "\n";

                        if (!InputValidator::confirmAction("Удалить эту заявку?")) {
                            InputUtils::printInfo("Удаление отменено.");
                            return;
                        }

                        if (appManager.removeApplicationById(appId, adminUsername)) {
                            appManager.saveApplications();
                            InputUtils::printSuccess("Заявка успешно удалена!");
                        }
                        else {
                            throw std::runtime_error("Ошибка при удалении заявки.");
                        }
                        break;
                    }

                    case 2: {  // Удалить все заявки студента
                        std::string username = InputValidator::getStringInput(
                            "Введите логин студента: ",
                            Validators::validateUsername,
                            "Логин должен быть от 3 до 20 символов",
                            false
                        );

                        auto student = userManager.findUser(username);

                        if (!student || student->getRole() != "student") {
                            throw std::runtime_error("Студент с таким логином не найден.");
                        }

                        int count = static_cast<int>(appManager.getApplicationsByStudent(username).size()); 
                        if (count == 0) {
                            InputUtils::printInfo("У студента нет заявок.");
                            return;
                        }

                        if (!InputValidator::confirmAction("Удалить все " + std::to_string(count) + " заявок студента " + username + "?")) {
                            InputUtils::printInfo("Удаление отменено.");
                            return;
                        }

                        if (appManager.removeApplicationsByStudent(username)) {
                            appManager.saveApplications();
                            InputUtils::printSuccess("Все заявки студента успешно удалены!");
                        }
                        else {
                            throw std::runtime_error("Ошибка при удалении заявок.");
                        }
                        break;
                    }

                    case 3: {  // Просмотреть все заявки
                        InputUtils::printHeader("Все заявки");
                        auto apps = appManager.getAllApplications();

                        if (apps.empty()) {
                            InputUtils::printInfo("Нет заявок для отображения.");
                        }
                        else {
                            std::cout << "Всего заявок: " << apps.size() << "\n";
                            InputUtils::printDivider();

                            for (const auto& app : apps) {
                                std::cout << app << "\n";
                                InputUtils::printDivider();
                            }
                        }
                        break;
                    }
                    }
                    }, "управления заявками");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 5: {  
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Управление пользователями");

                    std::vector<std::string> actionOptions = {
                        "Добавить студента",
                        "Добавить администратора",
                        "Удалить пользователя",
                        "Просмотреть всех пользователей"
                    };

                    int action = InputValidator::getMenuChoice(
                        "Выберите действие",
                        actionOptions,
                        [](int val) { return val >= 1 && val <= 4; },
                        "Выберите от 1 до 4"
                    );

                    switch (action) {
                    case 1: { 
                        InputUtils::printHeader("Добавление студента");

                        std::string username = InputValidator::getStringInput(
                            "Логин: ",
                            Validators::validateUsername,
                            "Логин должен быть от 3 до 20 символов",
                            false
                        );

                        if (userManager.findUser(username)) {
                            throw std::runtime_error("Пользователь с таким логином уже существует!");
                        }

                        std::string password = InputValidator::getPasswordInput("Пароль: ", 6);

                        std::string fio = InputValidator::getLineInput(
                            "ФИО: ",
                            Validators::validateFIO,
                            "ФИО должно быть от 5 до 30 символов",
                            false,
                            5,
                            100
                        );

                        double avg = InputValidator::getDoubleInput(
                            "Средний балл (0.0-10.0): ",
                            Validators::validateGrade,
                            "Балл должен быть от 0.0 до 10.0"
                        );

                        if (userManager.addUser(std::make_shared<Student>(username, Utils::hashPassword(password), fio, avg))) {
                            userManager.saveUsers();
                            InputUtils::printSuccess("Студент успешно добавлен!");
                        }
                        else {
                            throw std::runtime_error("Ошибка при добавлении студента.");
                        }
                        break;
                    }

                    case 2: {  
                        InputUtils::printHeader("Добавление администратора");

                        if (security.isLocked()) {
                            throw std::runtime_error("Система заблокирована!");
                        }

                        std::string masterPass = InputValidator::getStringInput(
                            "Введите мастер-пароль: ",
                            nullptr,
                            "",
                            false
                        );

                        if (!security.verifyMasterPassword(masterPass)) {
                            throw std::runtime_error("Неверный мастер-пароль!");
                        }

                        std::string username = InputValidator::getStringInput(
                            "Логин администратора: ",
                            Validators::validateUsername,
                            "Логин должен быть от 3 до 20 символов",
                            false
                        );

                        if (userManager.findUser(username)) {
                            throw std::runtime_error("Пользователь с таким логином уже существует!");
                        }

                        std::string password = InputValidator::getPasswordInput("Пароль: ", 6);

                        if (userManager.addUser(std::make_shared<Admin>(username, Utils::hashPassword(password)))) {
                            userManager.saveUsers();
                            InputUtils::printSuccess("Администратор успешно добавлен!");
                        }
                        else {
                            throw std::runtime_error("Ошибка при добавлении администратора.");
                        }
                        break;
                    }

                    case 3: { 
                        InputUtils::printHeader("Удаление пользователя");

                        std::string username = InputValidator::getStringInput(
                            "Введите логин пользователя: ",
                            Validators::validateUsername,
                            "Логин должен быть от 3 до 20 символов",
                            false
                        );

                        auto user = userManager.findUser(username);

                        if (!user) {
                            throw std::runtime_error("Пользователь не найден.");
                        }

                        if (user->getRole() == "admin") {
                            throw std::runtime_error("Нельзя удалить администратора!");
                        }

                        std::cout << "\nИнформация о пользователе:\n";
                        std::cout << "Логин: " << user->getUsername() << "\n";
                        std::cout << "Роль: " << user->getRole() << "\n";

                        if (!InputValidator::confirmAction("Удалить этого пользователя?")) {
                            InputUtils::printInfo("Удаление отменено.");
                            return;
                        }

                        if (userManager.removeUser(username)) {
                            if (user->getRole() == "student") {
                                appManager.removeApplicationsByStudent(username);
                                appManager.saveApplications();
                            }
                            userManager.saveUsers();
                            InputUtils::printSuccess("Пользователь успешно удален!");
                        }
                        else {
                            throw std::runtime_error("Ошибка при удалении пользователя.");
                        }
                        break;
                    }

                    case 4: {  
                        InputUtils::printHeader("Все пользователи");
                        auto users = userManager.getAllUsers();

                        if (users.empty()) {
                            InputUtils::printInfo("Нет пользователей в системе.");
                        }
                        else {
                            std::cout << "Всего пользователей: " << users.size() << "\n";
                            InputUtils::printDivider();

                            int studentCount = 0, adminCount = 0;
                            for (const auto& user : users) {
                                std::cout << "Логин: " << user->getUsername()
                                    << " | Роль: " << user->getRole() << "\n";
                                if (user->getRole() == "student") studentCount++;
                                else adminCount++;
                            }
                            InputUtils::printDivider();
                            std::cout << "Студентов: " << studentCount
                                << " | Администраторов: " << adminCount << "\n";
                        }
                        break;
                    }
                    }
                    }, "управления пользователями");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 6: {  // Работа со студентами
                studentManagementMenu(userManager, scholarshipManager, appManager);
                break;
            }

            case 7: {  // Просмотр истории заявок
                viewApplicationHistory(appManager.getHistory(), userManager);
                break;
            }

            case 8: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Смена мастер-пароля");

                    if (security.isLocked()) {
                        throw std::runtime_error("Система заблокирована.");
                    }

                    std::string oldPassword = InputValidator::getStringInput(
                        "Текущий мастер-пароль: ",
                        nullptr,
                        "",
                        false
                    );

                    if (!security.verifyMasterPassword(oldPassword)) {
                        throw std::runtime_error("Неверный мастер-пароль.");
                    }

                    std::string newPassword = InputValidator::getPasswordInput("Новый мастер-пароль: ", 6);

                    if (!InputValidator::confirmAction("Изменить мастер-пароль?")) {
                        InputUtils::printInfo("Смена пароля отменена.");
                        return;
                    }

                    if (security.setMasterPassword(newPassword)) {
                        InputUtils::printSuccess("Мастер-пароль обновлён.");
                    }
                    else {
                        throw std::runtime_error("Ошибка при обновлении мастер-пароля.");
                    }
                    }, "смены мастер-пароля");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 9: {  
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Редактирование своего аккаунтa");

                    std::string oldPassword = InputValidator::getStringInput(
                        "Текущий пароль: ",
                        nullptr,
                        "",
                        false
                    );

                    // Проверяем пароль текущего администратора
                    auto user = userManager.findUser(adminUsername);
                    if (!user || user->getPasswordHash() != Utils::hashPassword(oldPassword)) {
                        throw std::runtime_error("Неверный текущий пароль.");
                    }

                    std::string newPassword = InputValidator::getPasswordInput("Новый пароль: ", 6);

                    if (!InputValidator::confirmAction("Изменить пароль?")) {
                        InputUtils::printInfo("Смена пароля отменена.");
                        return;
                    }

                    if (userManager.updateUserPassword(adminUsername, Utils::hashPassword(newPassword))) {
                        userManager.saveUsers();
                        InputUtils::printSuccess("Пароль администратора изменён.");
                    }
                    else {
                        throw std::runtime_error("Ошибка при изменении пароля.");
                    }
                    }, "редактирования аккаунта");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            default:
                InputUtils::printError("Неверный выбор меню.");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

        }
        catch (const InputException& e) {
            InputUtils::printError("Ошибка ввода: " + std::string(e.what()));

            if (!InputValidator::getYesNoInput("Продолжить работу?")) {
                break;
            }
        }
        catch (const std::exception& e) {
            InputUtils::printError("Ошибка: " + std::string(e.what()));
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
        }
    }
}

//ПРОСМОТР ИСТОРИИ ЗАЯВОК=
void viewApplicationHistory(const ApplicationHistory& history, UserManager& userManager) {
    while (true) {
        try {
            std::vector<std::string> options = {
                "Просмотреть всю историю",
                "Просмотреть одобренные заявки",
                "Просмотреть отклоненные заявки",
                "Просмотреть удаленные заявки",
                "Поиск по студенту",
                "Поиск по администратору",
                "Общий поиск",
                "Статистика",
                "Экспорт истории в файл",
                "Назад"
            };

            int choice = InputValidator::getMenuChoice(
                "ИСТОРИЯ ЗАЯВОК НА СТИПЕНДИИ",
                options,
                [](int val) { return val >= 1 && val <= 10; },
                "Действие должно быть от 1 до 10"
            );

            if (choice == 10) break;

            switch (choice) {
            case 1: {  
                InputUtils::printHeader("ВСЯ ИСТОРИЯ ЗАЯВОК");
                auto allRecords = history.getAllRecords();

                if (allRecords.empty()) {
                    InputUtils::printInfo("История пуста.");
                }
                else {
                    std::cout << "Всего записей: " << allRecords.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& record : allRecords) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();
                    }

                    std::cout << "\nСТАТИСТИКА:\n";
                    std::cout << "  Создано: " << history.getActionCount(HistoryAction::CREATED) << "\n";
                    std::cout << "  Одобрено: " << history.getActionCount(HistoryAction::APPROVED) << "\n";
                    std::cout << "  Отклонено: " << history.getActionCount(HistoryAction::REJECTED) << "\n";
                    std::cout << "  Удалено: " << history.getActionCount(HistoryAction::DELETED) << "\n";
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 2: {  
                InputUtils::printHeader("ОДОБРЕННЫЕ ЗАЯВКИ");
                auto approved = history.getRecordsByAction(HistoryAction::APPROVED);

                if (approved.empty()) {
                    InputUtils::printInfo("Нет одобренных заявок в истории.");
                }
                else {
                    std::cout << "Всего одобрено: " << approved.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& record : approved) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();
                    }
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 3: {  
                InputUtils::printHeader("ОТКЛОНЕННЫЕ ЗАЯВКИ");
                auto rejected = history.getRecordsByAction(HistoryAction::REJECTED);

                if (rejected.empty()) {
                    InputUtils::printInfo("Нет отклоненных заявок в истории.");
                }
                else {
                    std::cout << "Всего отклонено: " << rejected.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& record : rejected) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();
                    }
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 4: { 
                InputUtils::printHeader("УДАЛЕННЫЕ ЗАЯВКИ");
                auto deleted = history.getRecordsByAction(HistoryAction::DELETED);

                if (deleted.empty()) {
                    InputUtils::printInfo("Нет удаленных заявок в истории.");
                }
                else {
                    std::cout << "Всего удалено: " << deleted.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& record : deleted) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();
                    }
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 5: {
                InputUtils::printHeader("ПОИСК ПО СТУДЕНТУ");

                std::string username = InputValidator::getStringInput(
                    "Введите логин студента: ",
                    Validators::validateUsername,
                    "Логин должен быть от 3 до 20 символов",
                    false
                );

                auto studentRecords = history.getRecordsByStudent(username);

                if (studentRecords.empty()) {
                    InputUtils::printInfo("Для студента " + username + " записей не найдено.");
                }
                else {
                    std::cout << "Найдено записей: " << studentRecords.size() << "\n";
                    InputUtils::printDivider();
                    int approved = 0, rejected = 0, created = 0, deleted = 0;
                    for (const auto& record : studentRecords) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();

                        switch (record.action) {
                        case HistoryAction::APPROVED: approved++; break;
                        case HistoryAction::REJECTED: rejected++; break;
                        case HistoryAction::CREATED: created++; break;
                        case HistoryAction::DELETED: deleted++; break;
                        }
                    }

                    std::cout << "\nСТАТИСТИКА ПО СТУДЕНТУ:\n";
                    std::cout << "  Всего заявок: " << (created + approved + rejected + deleted) << "\n";
                    std::cout << "  Создано: " << created << "\n";
                    std::cout << "  Одобрено: " << approved << "\n";
                    std::cout << "  Отклонено: " << rejected << "\n";
                    std::cout << "  Удалено: " << deleted << "\n";
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 6: {  
                InputUtils::printHeader("ПОИСК ПО АДМИНИСТРАТОРУ");

                std::string admin = InputValidator::getStringInput(
                    "Введите логин администратора: ",
                    Validators::validateUsername,
                    "Логин должен быть от 3 до 20 символов",
                    false
                );

                // метод поиска
                auto allRecords = history.getAllRecords();
                std::vector<HistoryRecord> adminRecords;

                for (const auto& record : allRecords) {
                    if (record.adminUsername == admin) {
                        adminRecords.push_back(record);
                    }
                }

                if (adminRecords.empty()) {
                    InputUtils::printInfo("Для администратора " + admin + " записей не найдено.");
                }
                else {
                    std::cout << "Найдено записей: " << adminRecords.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& record : adminRecords) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();
                    }
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 7: {  
                InputUtils::printHeader("ОБЩИЙ ПОИСК");

                std::string keyword = InputValidator::getLineInput(
                    "Введите поисковый запрос: ",
                    nullptr,
                    "",
                    true,
                    0,
                    100
                );

                auto results = history.searchRecords(keyword);

                if (results.empty()) {
                    InputUtils::printInfo("По запросу '" + keyword + "' ничего не найдено.");
                }
                else {
                    std::cout << "Найдено записей: " << results.size() << "\n";
                    InputUtils::printDivider();

                    for (const auto& record : results) {
                        std::cout << record.toString() << "\n";
                        InputUtils::printDivider();
                    }
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 8: {  // Статистика
                InputUtils::printHeader("СТАТИСТИКА ИСТОРИИ");

                int total = history.getTotalCount();
                int created = history.getActionCount(HistoryAction::CREATED);
                int approved = history.getActionCount(HistoryAction::APPROVED);
                int rejected = history.getActionCount(HistoryAction::REJECTED);
                int deleted = history.getActionCount(HistoryAction::DELETED);

                std::cout << "ОБЩАЯ СТАТИСТИКА:\n";
                std::cout << "  Всего записей в истории: " << total << "\n\n";

                if (total > 0) {
                    std::cout << "РАСПРЕДЕЛЕНИЕ ПО ДЕЙСТВИЯМ:\n";
                    std::cout << "  Создано: " << created << " ("
                        << (created * 100 / total) << "%)\n";
                    std::cout << "  Одобрено: " << approved << " ("
                        << (approved * 100 / total) << "%)\n";
                    std::cout << "  Отклонено: " << rejected << " ("
                        << (rejected * 100 / total) << "%)\n";
                    std::cout << "  Удалено: " << deleted << " ("
                        << (deleted * 100 / total) << "%)\n\n";

                    // Получаем топ студентов
                    auto allRecords = history.getAllRecords();
                    std::map<std::string, int> studentCount;
                    for (const auto& record : allRecords) {
                        studentCount[record.studentUsername]++;
                    }

                    std::cout << "АКТИВНЫЕ СТУДЕНТЫ (по количеству заявок):\n";
                    std::vector<std::pair<std::string, int>> sortedStudents(
                        studentCount.begin(), studentCount.end());
                    std::sort(sortedStudents.begin(), sortedStudents.end(),
                        [](const auto& a, const auto& b) {
                            return a.second > b.second;
                        });

                    for (int i = 0; i < std::min(5, (int)sortedStudents.size()); i++) {
                        std::cout << "  " << (i + 1) << ". " << sortedStudents[i].first
                            << " - " << sortedStudents[i].second << " заявок\n";
                    }
                }
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 9: {  // Экспорт в файл
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("ЭКСПОРТ ИСТОРИИ В ФАЙЛ");

                    std::string filename = InputValidator::getStringInput(
                        "Введите имя файла (например: history_export.txt): ",
                        [](const std::string& name) {
                            if (name.empty()) return false;
                            if (name.find('/') != std::string::npos ||
                                name.find('\\') != std::string::npos ||
                                name.find(':') != std::string::npos ||
                                name.find('*') != std::string::npos ||
                                name.find('?') != std::string::npos ||
                                name.find('"') != std::string::npos ||
                                name.find('<') != std::string::npos ||
                                name.find('>') != std::string::npos ||
                                name.find('|') != std::string::npos) {
                                throw RangeException("Имя файла содержит недопустимые символы");
                            }
                            return true;
                        },
                        "Недопустимое имя файла",
                        false
                    );

                    auto allRecords = history.getAllRecords();
                    std::ofstream file(filename);

                    if (!file.is_open()) {
                        throw std::runtime_error("Не удалось создать файл!");
                    }

                    file << "ЭКСПОРТ ИСТОРИИ ЗАЯВОК НА СТИПЕНДИИ\n";
                    file << "Дата экспорта: ";

                    auto now = std::chrono::system_clock::now();
                    auto now_time = std::chrono::system_clock::to_time_t(now);

                    char timeStr[26];
                    ctime_s(timeStr, sizeof(timeStr), &now_time);
                    file << timeStr << "\n";

                    file << "========================================\n\n";

                    for (const auto& record : allRecords) {
                        file << record.toString() << "\n";
                        file << "----------------------------------------\n";
                    }

                    file << "\nИТОГО: " << allRecords.size() << " записей\n";

                    file.close();
                    InputUtils::printSuccess("История экспортирована в файл: " + filename);
                    }, "экспорта истории");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            default:
                InputUtils::printError("Неверный выбор.");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

        }
        catch (const InputException& e) {
            InputUtils::printError("Ошибка ввода: " + std::string(e.what()));
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
        }
        catch (const std::exception& e) {
            InputUtils::printError("Ошибка: " + std::string(e.what()));
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
        }
    }
}

// УПРАВЛЕНИЕ СТУДЕНТАМИ
void studentManagementMenu(UserManager& userManager,
    ScholarshipTypeManager& scholarshipManager,
    ApplicationManager& appManager) {
    while (true) {
        try {
            std::vector<std::string> options = {
                "Просмотр всех студентов (кратко)",
                "Просмотр студентов со стипендией",
                "Поиск и подробный просмотр студента",
                "Анализ возможностей студента",
                "Статистика по студентам",
                "Назад в меню администратора"
            };

            int choice = InputValidator::getMenuChoice(
                "Управление студентами",
                options,
                [](int val) { return val >= 1 && val <= 6; },
                "Действие должно быть от 1 до 6"
            );

            if (choice == 6) break;

            switch (choice) {
            case 1: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Все студенты (краткая информация)");

                    auto students = userManager.getAllStudents();
                    if (students.empty()) {
                        InputUtils::printInfo("В системе нет студентов.");
                    }
                    else {
                        std::cout << "Всего студентов: " << students.size() << "\n";
                        InputUtils::printDivider();

                        // Сортируем по курсу и группе
                        std::sort(students.begin(), students.end(),
                            [](const std::shared_ptr<Student>& a, const std::shared_ptr<Student>& b) {
                                if (a->getCourse() != b->getCourse())
                                    return a->getCourse() < b->getCourse();
                                if (a->getFaculty() != b->getFaculty())
                                    return a->getFaculty() < b->getFaculty();
                                return a->getGroup() < b->getGroup();
                            });

                        int studentNumber = 1;
                        int currentCourse = -1;
                        std::string currentFaculty = "";

                        for (const auto& stu : students) {
                            // Вывод заголовка курса/факультета при изменении
                            if (stu->getCourse() != currentCourse || stu->getFaculty() != currentFaculty) {
                                currentCourse = stu->getCourse();
                                currentFaculty = stu->getFaculty();
                                std::cout << "\n--- Курс " << currentCourse << ", Факультет: "
                                    << currentFaculty << " ---\n";
                            }

                            std::cout << studentNumber++ << ". " << stu->getBasicInfo() << "\n";
                        }

                        InputUtils::printDivider();
                        std::cout << "\nДля просмотра подробной информации используйте пункт 3.\n";
                    }
                    }, "просмотра студентов");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 2: {  // Просмотр студентов со стипендией
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Студенты со стипендией");

                    auto students = userManager.getAllStudents();
                    std::vector<std::shared_ptr<Student>> scholarshipStudents;

                    for (const auto& stu : students) {
                        if (stu->getHasScholarship()) {
                            scholarshipStudents.push_back(stu);
                        }
                    }

                    if (scholarshipStudents.empty()) {
                        InputUtils::printInfo("Нет студентов со стипендией.");
                    }
                    else {
                        std::cout << "Стипендиатов: " << scholarshipStudents.size() << "\n";
                        InputUtils::printDivider();

                        // Группируем по типу стипендии
                        std::map<std::string, std::vector<std::shared_ptr<Student>>> byType;
                        for (const auto& stu : scholarshipStudents) {
                            std::string type = stu->getScholarshipType();
                            if (type.empty()) type = "Не указан";
                            byType[type].push_back(stu);
                        }

                        for (const auto& [type, studentsList] : byType) {
                            std::cout << "\nТип стипендии: " << type << " (количество: "
                                << studentsList.size() << ")\n";
                            for (size_t i = 0; i < studentsList.size(); i++) {
                                const auto& stu = studentsList[i];
                                std::cout << "  " << (i + 1) << ". " << stu->getFio()
                                    << " (" << stu->getGroup() << "), Средний: "
                                    << stu->getAverageGrade() << "\n";
                            }
                        }
                    }
                    }, "просмотра студентов со стипендией");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 3: {  // Поиск и подробный просмотр студента
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Поиск студента");

                    std::vector<std::string> searchOptions = {
                        "Поиск по логину",
                        "Поиск по ФИО",
                        "Поиск по группе",
                        "Список всех студентов для выбора"
                    };

                    int searchChoice = InputValidator::getMenuChoice(
                        "Выберите способ поиска",
                        searchOptions,
                        [](int val) { return val >= 1 && val <= 4; },
                        "Выберите от 1 до 4"
                    );

                    std::vector<std::shared_ptr<Student>> foundStudents;
                    std::string searchTerm;

                    switch (searchChoice) {
                    case 1: {
                        searchTerm = InputValidator::getStringInput(
                            "Введите логин: ",
                            Validators::validateUsername,
                            "Логин должен быть от 3 до 20 символов",
                            false
                        );

                        auto user = userManager.findUser(searchTerm);
                        if (user && user->getRole() == "student") {
                            foundStudents.push_back(std::static_pointer_cast<Student>(user));
                        }
                        break;
                    }
                    case 2: {
                        searchTerm = InputValidator::getLineInput(
                            "Введите ФИО или часть ФИО: ",
                            nullptr,
                            "",
                            true,
                            0,
                            100
                        );

                        auto students = userManager.getAllStudents();
                        for (const auto& stu : students) {
                            if (stu->getFio().find(searchTerm) != std::string::npos) {
                                foundStudents.push_back(stu);
                            }
                        }
                        break;
                    }
                    case 3: {
                        searchTerm = InputValidator::getLineInput(
                            "Введите номер группы: ",
                            nullptr,
                            "",
                            true,
                            0,
                            20
                        );

                        auto students = userManager.getAllStudents();
                        for (const auto& stu : students) {
                            if (stu->getGroup().find(searchTerm) != std::string::npos) {
                                foundStudents.push_back(stu);
                            }
                        }
                        break;
                    }
                    case 4: {
                        foundStudents = userManager.getAllStudents();
                        break;
                    }
                    }

                    if (foundStudents.empty()) {
                        InputUtils::printError("Студенты не найдены.");
                        return;
                    }

                    // Если нашли одного студента - сразу показываем
                    if (foundStudents.size() == 1) {
                        InputUtils::printHeader("Подробная информация о студенте");
                        std::cout << foundStudents[0]->getFullInfo();
                    }
                    // Если нашли несколько - предлагаем выбрать
                    else {
                        InputUtils::printHeader("Найдено студентов: " + std::to_string(foundStudents.size()));
                        for (size_t i = 0; i < foundStudents.size(); i++) {
                            const auto& stu = foundStudents[i];
                            std::cout << i + 1 << ". " << stu->getBasicInfo() << "\n";
                        }

                        int selected = InputValidator::getIntInput(
                            "\nВыберите номер студента для просмотра (0 для отмены): ",
                            [&foundStudents](int val) { return val >= 0 && val <= (int)foundStudents.size(); },
                            "Неверный выбор"
                        );

                        if (selected > 0) {
                            InputUtils::printHeader("Подробная информация о студенте");
                            std::cout << foundStudents[selected - 1]->getFullInfo();
                        }
                        else {
                            InputUtils::printInfo("Просмотр отменен.");
                        }
                    }
                    }, "поиска студента");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 4: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Анализ возможностей студента");

                    std::string username = InputValidator::getStringInput(
                        "Введите логин студента для анализа: ",
                        Validators::validateUsername,
                        "Логин должен быть от 3 до 20 символов",
                        false
                    );

                    auto user = userManager.findUser(username);
                    if (!user || user->getRole() != "student") {
                        throw std::runtime_error("Студент не найден!");
                    }

                    auto stu = std::static_pointer_cast<Student>(user);

                    InputUtils::printHeader("АНАЛИЗ СТУДЕНТА: " + stu->getFio());
                    std::cout << "Основная информация:\n";
                    std::cout << "  Курс: " << stu->getCourse() << ", Группа: " << stu->getGroup() << "\n";
                    std::cout << "  Факультет: " << stu->getFaculty() << "\n";
                    std::cout << "  Специальность: " << stu->getSpecialty() << "\n";
                    std::cout << "  Форма обучения: " << stu->getStudyFormString() << "\n";
                    InputUtils::printDivider();

                    // Анализ стипендий
                    std::string analysis = scholarshipManager.getStudentScholarshipInfo(
                        stu->getAverageGrade(),
                        stu->getHasSocialBenefits(),
                        stu->getHasScientificWorks(),
                        stu->getConferencesCount(),
                        stu->getIsActiveInCommunity()
                    );

                    std::cout << analysis;

                    InputUtils::printSection("РЕКОМЕНДАЦИИ");

                    auto types = scholarshipManager.getAllScholarshipTypes();
                    bool hasRecommendations = false;

                    for (const auto& type : types) {
                        auto requirements = type->checkRequirements(
                            stu->getAverageGrade(),
                            stu->getHasSocialBenefits(),
                            stu->getHasScientificWorks(),
                            stu->getConferencesCount(),
                            stu->getIsActiveInCommunity()
                        );

                        bool allMet = true;
                        for (const auto& req : requirements) {
                            if (!req.isMet) {
                                allMet = false;
                                if (!hasRecommendations) {
                                    std::cout << "Для повышения шансов на стипендии:\n";
                                    hasRecommendations = true;
                                }
                                std::cout << "  • " << type->getName() << ": " << req.description << "\n";
                            }
                        }

                        if (allMet && type->getName() != "Учебная стипендия") {
                            std::cout << "  [+] " << stu->getFio() << " соответствует всем требованиям для " 
                                << type->getName() << "!\n";
                        }
                    }

                    if (!hasRecommendations) {
                        InputUtils::printSuccess("Студент соответствует всем основным требованиям!");
                    }
                    }, "анализа возможностей студента");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            case 5: { 
                SafeExecutor::execute([&]() {
                    InputUtils::printHeader("Статистика по студентам");

                    auto students = userManager.getAllStudents();
                    if (students.empty()) {
                        InputUtils::printInfo("Нет студентов для анализа.");
                        return;
                    }

                    int totalStudents = static_cast<int>(students.size());
                    int budgetStudents = 0, paidStudents = 0;
                    int scholarshipStudents = 0;
                    int byCourse[4] = { 0, 0, 0, 0 };
                    double totalAverage = 0.0;

                    // Статистика по факультетам
                    std::map<std::string, int> facultyCount;
                    std::map<std::string, double> facultyAverage;

                    for (const auto& stu : students) {
                        // Форма обучения
                        if (stu->getStudyForm() == StudyForm::Budget) budgetStudents++;
                        else paidStudents++;

                        // Стипендия
                        if (stu->getHasScholarship()) scholarshipStudents++;

                        // Курс
                        int course = stu->getCourse() - 1;
                        if (course >= 0 && course < 4) {
                            byCourse[course]++;
                        }

                        // Факультеты
                        std::string faculty = stu->getFaculty();
                        if (faculty.empty()) faculty = "Не указан";
                        facultyCount[faculty]++;
                        facultyAverage[faculty] += stu->getAverageGrade();

                        totalAverage += stu->getAverageGrade();
                    }

                    std::cout << "ОБЩАЯ СТАТИСТИКА:\n";
                    std::cout << "  Всего студентов: " << totalStudents << "\n";
                    std::cout << "  Бюджетная форма: " << budgetStudents << " ("
                        << (budgetStudents * 100 / totalStudents) << "%)\n";
                    std::cout << "  Платная форма: " << paidStudents << " ("
                        << (paidStudents * 100 / totalStudents) << "%)\n";
                    std::cout << "  Стипендиатов: " << scholarshipStudents << " ("
                        << (scholarshipStudents * 100 / totalStudents) << "%)\n";
                    std::cout << "  Средний балл всех студентов: "
                        << (totalAverage / totalStudents) << "/10.0\n\n";

                    std::cout << "РАСПРЕДЕЛЕНИЕ ПО КУРСАМ:\n";
                    for (int i = 0; i < 4; i++) {
                        std::cout << "  Курс " << (i + 1) << ": " << byCourse[i] << " студентов ("
                            << (byCourse[i] * 100 / totalStudents) << "%)\n";
                    }
                    std::cout << "\n";

                    std::cout << "РАСПРЕДЕЛЕНИЕ ПО ФАКУЛЬТЕТАМ:\n";
                    for (const auto& [faculty, count] : facultyCount) {
                        double avg = facultyAverage[faculty] / count;
                        std::cout << "  " << faculty << ": " << count << " студентов ("
                            << (count * 100 / totalStudents) << "%), Средний балл: "
                            << avg << "/10.0\n";
                    }
                    }, "статистики по студентам");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

            default:
                InputUtils::printError("Неверный выбор меню.");
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
                break;
            }

        }
        catch (const InputException& e) {
            InputUtils::printError("Ошибка ввода: " + std::string(e.what()));
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
        }
        catch (const std::exception& e) {
            InputUtils::printError("Ошибка: " + std::string(e.what()));
            InputUtils::waitForEnter("Нажмите Enter для продолжения...");
        }
    }
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    std::set_terminate([]() {
        InputUtils::printError("Необработанное исключение! Программа будет завершена.");
        std::exit(1);
        });

    try {
        UserManager userManager;
        ScholarshipTypeManager scholarshipManager;
        ApplicationManager appManager;
        SecurityManager security("config.txt");
        SafeExecutor::execute([&security]() {
            security.ensureDefaultMaster();
            }, "инициализации безопасности");

        bool running = true;

        while (running) {
            try {
                // Главное меню
                std::vector<std::string> mainOptions = {
                    "Вход в систему",
                    "Регистрация студента",
                    "Создать аккаунт администратора",
                    "Выход из программы"
                };

                int choice = InputValidator::getMenuChoice(
                    "Система подачи заявок на стипендию",
                    mainOptions,
                    [](int val) { return val >= 1 && val <= 4; },
                    "Действие должно быть от 1 до 4"
                );

                switch (choice) {
                case 1: {
                    SafeExecutor::execute([&]() {
                        handleLogin(userManager, scholarshipManager, appManager, security);
                        }, "входа в систему");
                    break;
                }
                case 2: {
                    SafeExecutor::execute([&]() {
                        handleStudentRegistration(userManager);
                        }, "регистрации студента");
                    break;
                }
                case 3: {
                    SafeExecutor::execute([&]() {
                        handleAdminCreation(userManager, security);
                        }, "создания администратора");
                    break;
                }
                case 4: {
                    SafeExecutor::execute([&]() {
                        handleExit(userManager, appManager, scholarshipManager);
                        }, "выхода из программы");
                    running = false;
                    break;
                }
                }

            }
            catch (const InputException& e) {
                InputUtils::printError("Ошибка ввода: " + std::string(e.what()));
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            }
            catch (const FileException& e) {
                InputUtils::printError("Ошибка файла: " + std::string(e.what()));
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            }
            catch (const std::exception& e) {
                InputUtils::printError("Ошибка: " + std::string(e.what()));
                InputUtils::waitForEnter("Нажмите Enter для продолжения...");
            }
        }

    }
    catch (const std::exception& e) {
        InputUtils::printError("Фатальная ошибка инициализации: " + std::string(e.what()));
        InputUtils::printError("Программа будет завершена.");
        InputUtils::waitForEnter("Нажмите Enter для выхода...");
        return 1;
    }

    return 0;
}
