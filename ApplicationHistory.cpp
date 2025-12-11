#include "ApplicationHistory.h"
#include "FileManager.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream> 

ApplicationHistory::ApplicationHistory(const std::string& filename)
    : historyFile(filename) {
    loadFromFile();
}

void ApplicationHistory::loadFromFile() {
    records.clear();
    auto lines = FileManager::readLines(historyFile);

    for (const auto& line : lines) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string part;
        std::vector<std::string> parts;

        while (std::getline(ss, part, '|')) {
            parts.push_back(part);
        }

        if (parts.size() < 7) continue;  

        try {
            HistoryRecord record;
            record.applicationId = std::stoi(parts[0]);
            record.studentUsername = parts[1];
            record.scholarshipType = parts[2];
            record.action = static_cast<HistoryAction>(std::stoi(parts[3]));
            record.adminUsername = parts[4];
            record.comment = parts[5];
            record.timestamp = std::stoll(parts[6]);

            records.push_back(record);
        }
        catch (...) {
            // Пропускаем некорректные записи
        }
    }
}

void ApplicationHistory::saveToFile() const {
    std::vector<std::string> lines;

    for (const auto& record : records) {
        std::stringstream ss;
        ss << record.applicationId << "|"
            << record.studentUsername << "|"
            << record.scholarshipType << "|"
            << static_cast<int>(record.action) << "|"
            << record.adminUsername << "|"
            << record.comment << "|"
            << record.timestamp;
        lines.push_back(ss.str());
    }

    FileManager::writeLines(historyFile, lines);
}

void ApplicationHistory::addRecord(int appId, const std::string& student,
    const std::string& scholarship, HistoryAction action,
    const std::string& admin, const std::string& comment) {
    HistoryRecord record;
    record.applicationId = appId;
    record.studentUsername = student;
    record.scholarshipType = scholarship;
    record.action = action;
    record.adminUsername = admin;
    record.comment = comment;
    record.timestamp = std::time(nullptr);

    records.push_back(record);
    saveToFile();
}

std::vector<HistoryRecord> ApplicationHistory::getAllRecords() const {
    auto sorted = records;
    std::sort(sorted.begin(), sorted.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return sorted;
}

std::vector<HistoryRecord> ApplicationHistory::getRecordsByStudent(
    const std::string& username) const {
    std::vector<HistoryRecord> result;
    for (const auto& record : records) {
        if (record.studentUsername == username) {
            result.push_back(record);
        }
    }
    std::sort(result.begin(), result.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return result;
}

std::vector<HistoryRecord> ApplicationHistory::getRecordsByAction(
    HistoryAction action) const {
    std::vector<HistoryRecord> result;
    for (const auto& record : records) {
        if (record.action == action) {
            result.push_back(record);
        }
    }
    std::sort(result.begin(), result.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return result;
}

std::vector<HistoryRecord> ApplicationHistory::getRecordsByAdmin(
    const std::string& admin) const {
    std::vector<HistoryRecord> result;
    for (const auto& record : records) {
        if (record.adminUsername == admin) {
            result.push_back(record);
        }
    }
    std::sort(result.begin(), result.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return result;
}

std::vector<HistoryRecord> ApplicationHistory::searchRecords(
    const std::string& keyword) const {
    std::vector<HistoryRecord> result;
    std::string lowerKeyword = keyword;
    std::transform(lowerKeyword.begin(), lowerKeyword.end(),
        lowerKeyword.begin(), ::tolower);

    for (const auto& record : records) {
        std::string studentLower = record.studentUsername;
        std::transform(studentLower.begin(), studentLower.end(),
            studentLower.begin(), ::tolower);

        std::string scholarshipLower = record.scholarshipType;
        std::transform(scholarshipLower.begin(), scholarshipLower.end(),
            scholarshipLower.begin(), ::tolower);

        std::string adminLower = record.adminUsername;
        std::transform(adminLower.begin(), adminLower.end(),
            adminLower.begin(), ::tolower);

        if (studentLower.find(lowerKeyword) != std::string::npos ||
            scholarshipLower.find(lowerKeyword) != std::string::npos ||
            adminLower.find(lowerKeyword) != std::string::npos ||
            std::to_string(record.applicationId).find(keyword) != std::string::npos) {
            result.push_back(record);
        }
    }

    std::sort(result.begin(), result.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return result;
}

int ApplicationHistory::getActionCount(HistoryAction action) const {
    int count = 0;
    for (const auto& record : records) {
        if (record.action == action) {
            count++;
        }
    }
    return count;
}

std::string HistoryRecord::actionToString() const {
    switch (action) {
    case HistoryAction::CREATED: return "СОЗДАНА";
    case HistoryAction::APPROVED: return "ОДОБРЕНА";
    case HistoryAction::REJECTED: return "ОТКЛОНЕНА";
    case HistoryAction::DELETED: return "УДАЛЕНА";
    case HistoryAction::MODIFIED: return "ИЗМЕНЕНА";
    default: return "НЕИЗВЕСТНО";
    }
}

std::string HistoryRecord::toString() const {
    std::stringstream ss;
    char timeStr[20];
    struct tm timeinfo;
    localtime_s(&timeinfo, &timestamp);
    strftime(timeStr, sizeof(timeStr), "%d.%m.%Y %H:%M:%S", &timeinfo);

    ss << "ID: " << applicationId
        << " | Студент: " << studentUsername
        << " | Стипендия: " << scholarshipType
        << " | Действие: " << actionToString()
        << " | Администратор: " << (adminUsername.empty() ? "система" : adminUsername)
        << " | Время: " << timeStr;

    if (!comment.empty()) {
        ss << " | Комментарий: " << comment;
    }

    return ss.str();
}

void ApplicationHistory::clearHistory() {
    records.clear();
    saveToFile();
}
std::vector<HistoryRecord> ApplicationHistory::getRecordsSince(std::time_t since) const {
    std::vector<HistoryRecord> result;
    for (const auto& record : records) {
        if (record.timestamp >= since) {
            result.push_back(record);
        }
    }
    std::sort(result.begin(), result.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return result;
}

std::vector<HistoryRecord> ApplicationHistory::getRecordsBetween(std::time_t from, std::time_t to) const {
    std::vector<HistoryRecord> result;
    for (const auto& record : records) {
        if (record.timestamp >= from && record.timestamp <= to) {
            result.push_back(record);
        }
    }
    std::sort(result.begin(), result.end(),
        [](const HistoryRecord& a, const HistoryRecord& b) {
            return a.timestamp > b.timestamp;
        });
    return result;
}