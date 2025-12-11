#include "InputValidation.h"
#include <iostream>
#include <limits>
#include <conio.h>

void InputUtils::clearStdin() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void InputUtils::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
void InputUtils::waitForEnter(const std::string& message) {
    std::cout << message;
    std::cin.ignore();
    std::cin.get();
}

void InputUtils::printHeader(const std::string& title) {
    clearScreen();
    std::cout << "========================================================\n";
    std::cout << "                  " << title << "\n";
    std::cout << "========================================================\n\n";
}
void InputUtils::printMenu(const std::vector<std::string>& options) {
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << i + 1 << ". " << options[i] << "\n";
    }
    std::cout << "\n";
}

void InputUtils::printDivider() {
    std::cout << "----------------------------------------\n";
}

void InputUtils::printSection(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
}

void InputUtils::printSuccess(const std::string& message) {
    std::cout << "[OK] " << message << "\n"; 
}

void InputUtils::printError(const std::string& message) {
    std::cout << "[!] " << message << "\n";
}

void InputUtils::printInfo(const std::string& message) {
    std::cout << "[i] " << message << "\n";
}

void InputUtils::printWarning(const std::string& message) {
    std::cout << "[W] " << message << "\n";
}

std::string InputUtils::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");

    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}

int InputUtils::getIntInputOld(const std::string& prompt) {
    if (!prompt.empty()) {
        std::cout << prompt;
    }
    int value;
    while (!(std::cin >> value)) {
        std::cin.clear();
        InputUtils::clearStdin();
        std::cout << "Некорректный ввод. Пожалуйста, введите число: ";
    }
    InputUtils::clearStdin();
    return value;
}

double InputUtils::getDoubleInputOld(const std::string& prompt) {
    if (!prompt.empty()) {
        std::cout << prompt;
    }
    double value;
    while (!(std::cin >> value)) {
        std::cin.clear();
        InputUtils::clearStdin();
        std::cout << "Некорректный ввод. Пожалуйста, введите число: ";
    }
    InputUtils::clearStdin();
    return value;
}

std::string InputUtils::getStringInputOld(const std::string& prompt) {
    if (!prompt.empty()) {
        std::cout << prompt;
    }
    std::string value;
    std::cin >> value;
    InputUtils::clearStdin();
    return value;
}



std::string InputUtils::getLineInputOld(const std::string& prompt) {
    if (!prompt.empty()) {
        std::cout << prompt;
    }
    std::string value;
    std::getline(std::cin, value);

    size_t start = value.find_first_not_of(" \t\n\r");
    size_t end = value.find_last_not_of(" \t\n\r");

    if (start != std::string::npos && end != std::string::npos) {
        return value.substr(start, end - start + 1);
    }

    return "";
}
std::string InputValidator::getPasswordHidden(const std::string& prompt) {
    std::string password;
    std::cout << prompt;
 
    char ch;
    while (true) {
        ch = _getch();

        if (ch == '\r' || ch == '\n') {
            break;
        }
        else if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                std::cout << "\b \b";
            }
        }
        else if (ch >= 32 && ch <= 126) {
            password.push_back(ch);
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return password;
}

std::string InputValidator::getPasswordInput(
    const std::string& prompt,
    int minLength
) {
    std::string password;

    while (true) {
        try {
            password = InputValidator::getPasswordHidden(prompt);

            password = InputUtils::trim(password);

            if (password.empty()) {
                throw EmptyInputException("Пароль не может быть пустым");
            }

            if (static_cast<int>(password.length()) < minLength) {
                throw RangeException("Пароль должен содержать не менее " +
                    std::to_string(minLength) + " символов");
            }

            bool hasDigit = false;
            bool hasLetter = false;

            for (char c : password) {
                if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
                if (std::isalpha(static_cast<unsigned char>(c))) hasLetter = true;
            }

            if (!hasDigit || !hasLetter) {
                throw PasswordException("Пароль должен содержать буквы и цифры");
            }

            std::string confirm = InputValidator::getPasswordHidden("Повторите пароль: ");
            confirm = InputUtils::trim(confirm);

            if (password != confirm) {
                throw PasswordException("Пароли не совпадают");
            }

            return password;
        }
        catch (const InputException& e) {
            InputUtils::printError(std::string("Ошибка пароля: ") + e.what());
            InputUtils::printError("Пожалуйста, попробуйте снова.");
        }
    }
}