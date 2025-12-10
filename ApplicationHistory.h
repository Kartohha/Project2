#ifndef APPLICATIONHISTORY_H
#define APPLICATIONHISTORY_H

#include <string>
#include <vector>
#include <ctime>

enum class HistoryAction {
    CREATED,      // Заявка создана
    APPROVED,     // Заявка одобрена
    REJECTED,     // Заявка отклонена
    DELETED,      // Заявка удалена
    MODIFIED      // Заявка изменена
};

// Структура вне класса
struct HistoryRecord {
    int applicationId;
    std::string studentUsername;
    std::string scholarshipType;
    HistoryAction action;
    std::string adminUsername;
    std::string comment;
    std::time_t timestamp;

    std::string toString() const;
    std::string actionToString() const;
};

class ApplicationHistory {
private:
    std::vector<HistoryRecord> records;
    std::string historyFile;

    void loadFromFile();
    void saveToFile() const;

public:
    ApplicationHistory(const std::string& filename = "history.txt");

    // Добавление записи
    void addRecord(int appId, const std::string& student,
        const std::string& scholarship, HistoryAction action,
        const std::string& admin = "", const std::string& comment = "");

    // Получение истории
    std::vector<HistoryRecord> getAllRecords() const;
    std::vector<HistoryRecord> getRecordsByStudent(const std::string& username) const;
    std::vector<HistoryRecord> getRecordsByAction(HistoryAction action) const;
    std::vector<HistoryRecord> getRecordsByAdmin(const std::string& admin) const;

    // Фильтрация по дате
    std::vector<HistoryRecord> getRecordsSince(std::time_t since) const;
    std::vector<HistoryRecord> getRecordsBetween(std::time_t from, std::time_t to) const;

    // Поиск записи
    std::vector<HistoryRecord> searchRecords(const std::string& keyword) const;

    // Статистика
    int getTotalCount() const { return static_cast<int>(records.size()); }
    int getActionCount(HistoryAction action) const;

    // Очистка истории
    void clearHistory();
};

#endif // APPLICATIONHISTORY_H