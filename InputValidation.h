#ifndef INPUTVALIDATION_H
#define INPUTVALIDATION_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <limits>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <random>

//КЛАССЫ ИСКЛЮЧЕНИЙ
class InputException : public std::exception {
protected:
    std::string message;
public:
    InputException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class RangeException : public InputException {
public:
    RangeException(const std::string& msg) : InputException(msg) {}
};

class TypeException : public InputException {
public:
    TypeException(const std::string& msg) : InputException(msg) {}
};

class EmptyInputException : public InputException {
public:
    EmptyInputException(const std::string& msg) : InputException(msg) {}
};

class PasswordException : public InputException {
public:
    PasswordException(const std::string& msg) : InputException(msg) {}
};

class FileException : public std::runtime_error {
public:
    FileException(const std::string& filename, const std::string& operation)
        : std::runtime_error("Ошибка " + operation + " файла: " + filename) {}

    FileException(const std::string& message)
        : std::runtime_error(message) {}
};

class FileNotFoundException : public FileException {
public:
    FileNotFoundException(const std::string& filename)
        : FileException(filename, "открытия (файл не найден)") {}
};

class FileReadException : public FileException {
public:
    FileReadException(const std::string& filename)
        : FileException(filename, "чтения") {}
};

class FileWriteException : public FileException {
public:
    FileWriteException(const std::string& filename)
        : FileException(filename, "записи") {}
};


class InputUtils {
public:
    static void clearStdin();
    static void clearScreen();
    static void waitForEnter(const std::string& message);
    static void printHeader(const std::string& title);
    static void printMenu(const std::vector<std::string>& options);
    static void printDivider();
    static void printSection(const std::string& title);
    static void printSuccess(const std::string& message);
    static void printError(const std::string& message);
    static void printInfo(const std::string& message);
    static void printWarning(const std::string& message);
    static std::string trim(const std::string& str);

 
    static int getIntInputOld(const std::string& prompt = "");
    static double getDoubleInputOld(const std::string& prompt = "");
    static std::string getStringInputOld(const std::string& prompt = "");
    static std::string getLineInputOld(const std::string& prompt = "");
};

//ВАЛИДАЦИЯ ВВОДА
class InputValidator {
public:
    //для ввода числа
    template<typename T>
    static T getNumberInput(
        const std::string& prompt,
        std::function<bool(const T&)> validator = nullptr,
        const std::string& errorMessage = "Некорректное значение"
    ) {
        T value;

        while (true) {
            try {
                std::cout << prompt;

                if (!(std::cin >> value)) {
                    std::cin.clear();
                    InputUtils::clearStdin();
                    throw TypeException(getTypeErrorMessage<T>());
                }

                InputUtils::clearStdin();

                if (validator && !validator(value)) {
                    throw RangeException(errorMessage);
                }

                return value;
            }
            catch (const InputException& e) {
                InputUtils::printError(std::string(e.what()));
                InputUtils::printError("Пожалуйста, попробуйте снова.");
            }
        }
    }

    // Ввод целого числа
    static int getIntInput(
        const std::string& prompt,
        std::function<bool(int)> validator = nullptr,
        const std::string& errorMessage = "Некорректное значение"
    ) {
        return getNumberInput<int>(prompt, validator, errorMessage);
    }

    // Ввод числа с плавающей точкой
    static double getDoubleInput(
        const std::string& prompt,
        std::function<bool(double)> validator = nullptr,
        const std::string& errorMessage = "Некорректное значение"
    ) {
        return getNumberInput<double>(prompt, validator, errorMessage);
    }

    // Ввод строки (одно слово)
    static std::string getStringInput(
        const std::string& prompt,
        std::function<bool(const std::string&)> validator = nullptr,
        const std::string& errorMessage = "Некорректный ввод",
        bool allowEmpty = false
    ) {
        std::string value;

        while (true) {
            try {
                std::cout << prompt;
                std::cin >> value;
                InputUtils::clearStdin();

                value = InputUtils::trim(value);

                if (!allowEmpty && value.empty()) {
                    throw EmptyInputException("Ввод не может быть пустым");
                }

                if (validator && !validator(value)) {
                    throw RangeException(errorMessage);
                }

                return value;
            }
            catch (const InputException& e) {
                InputUtils::printError(std::string(e.what()));
                InputUtils::printError("Пожалуйста, попробуйте снова.");
            }
        }
    }

    // Ввод строки с пробелами
    static std::string getLineInput(
        const std::string& prompt,
        std::function<bool(const std::string&)> validator = nullptr,
        const std::string& errorMessage = "Некорректный ввод",
        bool allowEmpty = false,
        int minLength = 0,
        int maxLength = 1000
    ) {
        std::string value;

        while (true) {
            try {
                std::cout << prompt;
                std::getline(std::cin, value);

                value = InputUtils::trim(value);

                if (!allowEmpty && value.empty()) {
                    throw EmptyInputException("Ввод не может быть пустым");
                }

                if (static_cast<int>(value.length()) < minLength) {
                    throw RangeException("Слишком короткий ввод (минимум " +
                        std::to_string(minLength) + " символов)");
                }

                if (static_cast<int>(value.length()) > maxLength) {
                    throw RangeException("Слишком длинный ввод (максимум " +
                        std::to_string(maxLength) + " символов)");
                }

                if (validator && !validator(value)) {
                    throw RangeException(errorMessage);
                }

                return value;
            }
            catch (const InputException& e) {
                InputUtils::printError(std::string(e.what()));
                InputUtils::printError("Пожалуйста, попробуйте снова.");
            }
        }
    }

    // Ввод yes/no
    static bool getYesNoInput(const std::string& prompt) {
        char choice;

        while (true) {
            try {
                std::cout << prompt << " (y/n): ";
                std::cin >> choice;
                InputUtils::clearStdin();

                choice = std::tolower(static_cast<unsigned char>(choice));

                // Поддержка русской и английской раскладки
                if (choice == 'y' || choice == 'н' || choice == 'д') {
                    return true;
                }
                else if (choice == 'n' || choice == 'т') {
                    return false;
                }
                else {
                    InputUtils::printError("Введите 'y' (да) или 'n' (нет).");
                }
            }
            catch (...) {
                InputUtils::printError("Ошибка ввода. Пожалуйста, попробуйте снова.");
            }
        }
    }

    // Ввод пароля со звездочками
    static std::string getPasswordHidden(const std::string& prompt = "Пароль: ");

    // Ввод пароля с проверкой
    static std::string getPasswordInput(
        const std::string& prompt = "Пароль: ",
        int minLength = 6
    );

  
    static int getMenuChoice(
        const std::string& title,
        const std::vector<std::string>& options,
        std::function<bool(int)> validator = nullptr,
        const std::string& errorMessage = "Неверный выбор"
    ) {
        int choice;

        while (true) {
            try {
                InputUtils::printHeader(title);
                InputUtils::printMenu(options);

                choice = getIntInput(
                    "Выберите действие: ",
                    validator,
                    errorMessage
                );

                return choice;
            }
            catch (const InputException& e) {
                InputUtils::printError(std::string(e.what()));

                if (!getYesNoInput("Повторить выбор?")) {
                    throw;
                }
            }
        }
    }

    // Ввод диапазона значений
    template<typename T>
    static std::pair<T, T> getRangeInput(
        const std::string& promptMin,
        const std::string& promptMax,
        const T& minLimit,
        const T& maxLimit
    ) {
        T minVal, maxVal;

        while (true) {
            try {
                minVal = getNumberInput<T>(promptMin, nullptr, "");
                maxVal = getNumberInput<T>(promptMax, nullptr, "");

                if (minVal < minLimit || minVal > maxLimit) {
                    throw RangeException("Минимальное значение вне допустимого диапазона");
                }

                if (maxVal < minLimit || maxVal > maxLimit) {
                    throw RangeException("Максимальное значение вне допустимого диапазона");
                }

                if (minVal > maxVal) {
                    throw RangeException("Минимальное значение не может быть больше максимального");
                }

                return { minVal, maxVal };
            }
            catch (const InputException& e) {
                InputUtils::printError(std::string("Ошибка диапазона: ") + e.what());
                InputUtils::printError("Пожалуйста, попробуйте снова.");
            }
        }
    }

    // Подтверждение действия
    static bool confirmAction(const std::string& actionDescription) {
        InputUtils::printWarning(actionDescription);
        return getYesNoInput("Вы уверены?");
    }

private:
    template<typename T>
    static std::string getTypeErrorMessage() {
        if (std::is_same<T, int>::value) {
            return "Ожидается целое число";
        }
        else if (std::is_same<T, double>::value) {
            return "Ожидается число";
        }
        else {
            return "Ожидается значение определенного типа";
        }
    }
};


class SafeExecutor {
public:
    template<typename Func, typename... Args>
    static bool execute(Func func, const std::string& operationName, Args&&... args) {
        try {
            func(std::forward<Args>(args)...);
            return true;
        }
        catch (const InputException& e) {
            InputUtils::printError("Ошибка ввода при " + operationName + ": " + e.what());
            return false;
        }
        catch (const FileException& e) {
            InputUtils::printError("Ошибка файла при " + operationName + ": " + e.what());
            return false;
        }
        catch (const std::exception& e) {
            InputUtils::printError("Ошибка при " + operationName + ": " + e.what());
            return false;
        }
        catch (...) {
            InputUtils::printError("Неизвестная ошибка при " + operationName);
            return false;
        }
    }

    template<typename Func, typename... Args>
    static auto executeWithResult(Func func, const std::string& operationName, Args&&... args)
        -> std::pair<bool, typename std::invoke_result<Func, Args...>::type> {

        using ReturnType = typename std::invoke_result<Func, Args...>::type;

        try {
            ReturnType result = func(std::forward<Args>(args)...);
            return { true, result };
        }
        catch (const std::exception& e) {
            InputUtils::printError("Ошибка при " + operationName + ": " + e.what());
            return { false, ReturnType() };
        }
        catch (...) {
            InputUtils::printError("Неизвестная ошибка при " + operationName);
            return { false, ReturnType() };
        }
    }
    template<typename Func, typename... Args>
    static auto executeWithRetry(Func func, const std::string& operationName,
        int maxRetries = 3, Args&&... args)
        -> typename std::invoke_result<Func, Args...>::type {

        using ReturnType = typename std::invoke_result<Func, Args...>::type;

        int attempts = 0;

        while (attempts < maxRetries) {
            try {
                return func(std::forward<Args>(args)...);
            }
            catch (const std::exception& e) {
                attempts++;
                InputUtils::printWarning("Ошибка при " + operationName + ": " + e.what());

                if (attempts >= maxRetries) {
                    InputUtils::printError("Не удалось выполнить операцию после " +
                        std::to_string(maxRetries) + " попыток.");
                    throw;
                }

                InputUtils::printInfo("Повторная попытка...");
            }
        }

        return ReturnType();
    }
};

//ВАЛИДАТОРЫ
class Validators {
public:
    // Проверка логина
    static bool validateUsername(const std::string& username) {
        if (username.length() < 3 || username.length() > 20) {
            throw RangeException("Логин должен быть от 3 до 20 символов");
        }

        // Только буквы, цифры и подчеркивание
        for (char c : username) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                throw RangeException("Логин может содержать только буквы, цифры и подчеркивание");
            }
        }

        return true;
    }

    // Проверка ФИО - ТОЛЬКО буквы, пробелы и дефисы
    static bool validateFIO(const std::string& fio) {
        if (fio.length() < 5 || fio.length() > 30) {
            throw RangeException("ФИО должно быть от 5 до 30 символов");
        }

        // Проверка на наличие только букв, пробелов и дефисов
        for (char c : fio) {
            // Проверяем русские и английские буквы
            bool isRussianLetter = (c >= 'А' && c <= 'я') || c == 'ё' || c == 'Ё';
            bool isEnglishLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
            bool isSpace = std::isspace(static_cast<unsigned char>(c));
            bool isHyphen = (c == '-');

            if (!isRussianLetter && !isEnglishLetter && !isSpace && !isHyphen) {
                throw RangeException("ФИО может содержать только буквы, пробелы и дефисы");
            }
        }

        return true;
    }

    // Проверка среднего балла
    static bool validateGrade(double grade) {
        if (grade < 0.0 || grade > 10.0) {
            throw RangeException("Средний балл должен быть от 0.0 до 10.0");
        }
        return true;
    }

    // Проверка курса - 1-4
    static bool validateCourse(int course) {
        if (course < 1 || course > 4) {
            throw RangeException("Курс должен быть от 1 до 4");
        }
        return true;
    }

    // Проверка группы - ТОЛЬКО 6 цифр
    static bool validateGroup(const std::string& group) {
        if (group.length() != 6) {
            throw RangeException("Номер группы должен состоять из 6 цифр");
        }

        // Проверяем, что все символы - цифры
        for (char c : group) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                throw RangeException("Номер группы должен содержать только цифры");
            }
        }

        return true;
    }

    // Проверка факультета - буквы, 2-5 символов
    static bool validateFaculty(const std::string& faculty) {
        if (faculty.length() < 2 || faculty.length() > 5) {
            throw RangeException("Название факультета должно быть от 2 до 5 символов");
        }

        // Проверка, что все символы - буквы
        for (char c : faculty) {
            // Проверка русские и английские буквы
            bool isRussianLetter = (c >= 'А' && c <= 'я') || c == 'ё' || c == 'Ё';
            bool isEnglishLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');

            if (!isRussianLetter && !isEnglishLetter) {
                throw RangeException("Название факультета должно содержать только буквы");
            }
        }

        return true;
    }

    // Проверка специальности - ТОЛЬКО буквы, 1-7 символов
    static bool validateSpecialty(const std::string& specialty) {
        if (specialty.length() < 1 || specialty.length() > 7) {
            throw RangeException("Название специальности должно быть от 1 до 7 символов");
        }

        // Проверка, что все символы - буквы
        for (char c : specialty) {
            // Проверка русские и английские буквы
            bool isRussianLetter = (c >= 'А' && c <= 'я') || c == 'ё' || c == 'Ё';
            bool isEnglishLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');

            if (!isRussianLetter && !isEnglishLetter) {
                throw RangeException("Название специальности должно содержать только буквы");
            }
        }

        return true;
    }

    // Проверка количества конференций
    static bool validateConferences(int conferences) {
        if (conferences < 0) {
            throw RangeException("Количество конференций не может быть отрицательным");
        }
        return true;
    }

    // Проверка ID
    static bool validateId(int id) {
        if (id <= 0) {
            throw RangeException("ID должен быть положительным числом");
        }
        return true;
    }

    // Проверка статуса
    static bool validateStatus(int status) {
        if (status < 0 || status > 2) {
            throw RangeException("Статус должен быть от 0 до 2");
        }
        return true;
    }

    // Проверка категории стипендии
    static bool validateScholarshipCategory(int category) {
        if (category < 0 || category > 4) {
            throw RangeException("Категория стипендии должна быть от 0 до 4");
        }
        return true;
    }
    
};

#endif 