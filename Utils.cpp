#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <cctype>

namespace Utils {
    // Временное решение - добавляем соль
    std::string generateSalt() {
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::string salt;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

        for (int i = 0; i < 16; ++i) {
            salt += alphanum[dis(gen)];
        }
        return salt;
    }

    std::string hashPassword(const std::string& password) {
        // TODO: Заменить на bcrypt/scrypt/Argon2
        // Временное решение: std::hash с солью

        std::string salt = "scholarship_system_salt_2024"; // Фиксированная соль пока
        std::string saltedPassword = password + salt;

        std::hash<std::string> hasher;
        size_t hash1 = hasher(saltedPassword);

        // Двойное хэширование для увеличения сложности
        std::string hashStr = std::to_string(hash1) + salt;
        size_t hash2 = hasher(hashStr);

        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash2;
        return ss.str();
    }

    std::string toLower(const std::string& s) {
        std::string r = s;
        std::transform(r.begin(), r.end(), r.begin(), ::tolower);
        return r;
    }

    std::string statusToString(int status) {
        switch (status) {
        case 0: return "Ожидание";
        case 1: return "Одобрено";
        case 2: return "Отклонено";
        default: return "Неизвестно";
        }
    }

    int stringToStatus(const std::string& s) {
        if (s == "Pending" || s == "Ожидание") return 0;
        if (s == "Approved" || s == "Одобрено") return 1;
        if (s == "Rejected" || s == "Отклонено") return 2;
        return 0;
    }

    long long currentTimeSeconds() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    // Новые функции для работы с CSV
    std::string escapeCSV(const std::string& field) {
        if (field.empty()) return field;

        // Если поле содержит запятые, кавычки или переносы строк, заключаем в кавычки
        bool needsQuotes = field.find(',') != std::string::npos ||
            field.find('"') != std::string::npos ||
            field.find('\n') != std::string::npos ||
            field.find('\r') != std::string::npos;

        if (!needsQuotes) return field;

        std::string result = "\"";
        for (char c : field) {
            if (c == '"') {
                result += "\"\""; // Экранируем кавычку
            }
            else {
                result += c;
            }
        }
        result += "\"";
        return result;
    }

    std::string unescapeCSV(const std::string& field) {
        if (field.empty()) return field;

        // Если поле не заключено в кавычки, возвращаем как есть
        if (field.size() < 2 || field.front() != '"' || field.back() != '"') {
            return field;
        }

        std::string result = field.substr(1, field.length() - 2);
        size_t pos = 0;
        while ((pos = result.find("\"\"", pos)) != std::string::npos) {
            result.replace(pos, 2, "\"");
            pos += 1;
        }
        return result;
    }
}