#include "FileManager.h"
#include <fstream>
#include <iostream>
#include <filesystem>

std::vector<std::string> FileManager::readLines(const std::string& filename) {
    std::vector<std::string> lines;

    try {
        // Проверяем существование файла
        std::filesystem::path filepath(filename);
        if (!std::filesystem::exists(filepath)) {
            std::cout << "Отладка: Файл " << filename << " не найден. Будет создан новый.\n";
            return lines;
        }

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Отладка FileManager: Не удалось открыть файл " << filename << std::endl;
            return lines;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Убираем возврат каретки, если есть
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty()) {
                lines.push_back(line);
            }
        }

        file.close();

    }
    catch (const std::exception& e) {
        std::cout << "Ошибка при чтении файла " << filename << ": " << e.what() << std::endl;
    }

    return lines;
}

void FileManager::writeLines(const std::string& filename, const std::vector<std::string>& lines) {
    try {
        std::filesystem::path filepath(filename);
        if (filepath.has_parent_path()) {
            std::filesystem::create_directories(filepath.parent_path());
        }

        std::ofstream file(filename, std::ios::trunc);

        if (!file.is_open()) {
            std::cout << "Отладка FileManager: Не удалось открыть файл для записи " << filename << std::endl;
            return;
        }

        for (const auto& line : lines) {
            file << line << "\n";
        }

        file.close();

    }
    catch (const std::exception& e) {
        std::cout << "Ошибка при записи файла " << filename << ": " << e.what() << std::endl;
    }
}