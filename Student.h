#ifndef STUDENT_H
#define STUDENT_H

#include "IUser.h"
#include <string>

enum class StudyForm { Budget, Paid };

class Student : public IUser {
    std::string fio;
    double averageGrade;
    bool hasScholarship;
    bool hasSocialBenefits;      // НАЛИЧИЕ ЛЬГОТ
    bool hasScientificWorks;     // НАЛИЧИЕ НАУЧНЫХ РАБОТ (вместо publicationsCount)
    int conferencesCount;        // УЧАСТИЕ В КОНФЕРЕНЦИЯХ
    bool isActiveInCommunity;    // ОБЩЕСТВЕННАЯ АКТИВНОСТЬ

    // Новые поля
    StudyForm studyForm;
    int course;
    std::string group;
    std::string faculty;
    std::string specialty;
    std::string scholarshipType;

    // Поля для комментариев (МОЖНО РЕДАКТИРОВАТЬ)
    std::string socialBenefitsComment;    // Комментарий к льготам
    std::string scientificWorksComment;   // Комментарий к научным работам
    std::string conferencesComment;       // Комментарий к участию в конференциях
    std::string communityActivityComment; // Комментарий к общественной деятельности

public:
    Student(const std::string& user, const std::string& hash,
        const std::string& fio, double avg, bool hasScholarship = false,
        bool hasSocialBenefits = false, bool hasScientificWorks = false,
        int conferences = 0, bool activeInCommunity = false,
        StudyForm form = StudyForm::Budget, int course = 1,
        const std::string& group = "", const std::string& faculty = "",
        const std::string& specialty = "", const std::string& scholarshipType = "",
        const std::string& socialBenefitsComment = "",
        const std::string& scientificWorksComment = "",
        const std::string& conferencesComment = "",
        const std::string& communityActivityComment = "");

    std::string getRole() const override { return "student"; }

    // Геттеры существующих полей
    std::string getFio() const { return fio; }
    double getAverageGrade() const { return averageGrade; }
    void setAverageGrade(double g) { averageGrade = g; }
    bool getHasScholarship() const { return hasScholarship; }
    void setHasScholarship(bool s) { hasScholarship = s; }

    // ЛЬГОТЫ - МОЖНО РЕДАКТИРОВАТЬ
    bool getHasSocialBenefits() const { return hasSocialBenefits; }
    void setHasSocialBenefits(bool b) { hasSocialBenefits = b; }

    // НАУЧНЫЕ РАБОТЫ - МОЖНО РЕДАКТИРОВАТЬ (вместо публикаций)
    bool getHasScientificWorks() const { return hasScientificWorks; }
    void setHasScientificWorks(bool has) { hasScientificWorks = has; }

    // КОНФЕРЕНЦИИ - МОЖНО РЕДАКТИРОВАТЬ
    int getConferencesCount() const { return conferencesCount; }
    void setConferencesCount(int count) { conferencesCount = count; }

    // ОБЩЕСТВЕННАЯ АКТИВНОСТЬ - МОЖНО РЕДАКТИРОВАТЬ
    bool getIsActiveInCommunity() const { return isActiveInCommunity; }
    void setIsActiveInCommunity(bool active) { isActiveInCommunity = active; }

    // Геттеры новых полей
    StudyForm getStudyForm() const { return studyForm; }
    void setStudyForm(StudyForm form) { studyForm = form; }
    int getCourse() const { return course; }
    void setCourse(int c) { course = c; }
    std::string getGroup() const { return group; }
    void setGroup(const std::string& g) { group = g; }
    std::string getFaculty() const { return faculty; }
    void setFaculty(const std::string& f) { faculty = f; }
    std::string getSpecialty() const { return specialty; }
    void setSpecialty(const std::string& s) { specialty = s; }
    std::string getScholarshipType() const { return scholarshipType; }
    void setScholarshipType(const std::string& type) { scholarshipType = type; }

    // Геттеры и сеттеры для комментариев - МОЖНО РЕДАКТИРОВАТЬ
    std::string getSocialBenefitsComment() const { return socialBenefitsComment; }
    void setSocialBenefitsComment(const std::string& comment) { socialBenefitsComment = comment; }

    std::string getScientificWorksComment() const { return scientificWorksComment; }
    void setScientificWorksComment(const std::string& comment) { scientificWorksComment = comment; }

    std::string getConferencesComment() const { return conferencesComment; }
    void setConferencesComment(const std::string& comment) { conferencesComment = comment; }

    std::string getCommunityActivityComment() const { return communityActivityComment; }
    void setCommunityActivityComment(const std::string& comment) { communityActivityComment = comment; }
    void setFio(const std::string& f) { fio = f; }
  

    // Вспомогательные методы
    std::string getStudyFormString() const {
        return (studyForm == StudyForm::Budget) ? "Бюджет" : "Платно";
    }
    bool hasEnoughConferences(int requiredCount) const {
        return conferencesCount >= requiredCount;
    }
    // Методы для получения информации
    std::string getFullInfo() const;       // Полная информация
    std::string getBasicInfo() const;      // Базовая информация
    std::string getAcademicInfo() const;   // Академическая информация
    std::string getCommentsInfo() const;   // Информация о комментариях
};

#endif // STUDENT_H