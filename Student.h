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
    bool hasScientificWorks;     // НАЛИЧИЕ НАУЧНЫХ РАБОТ 
    int conferencesCount;        // УЧАСТИЕ В КОНФЕРЕНЦИЯХ
    bool isActiveInCommunity;    // ОБЩЕСТВЕННАЯ АКТИВНОСТЬ

    StudyForm studyForm;
    int course;
    std::string group;
    std::string faculty;
    std::string specialty;
    std::string scholarshipType;
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

    std::string getFio() const { return fio; }
    double getAverageGrade() const { return averageGrade; }
    void setAverageGrade(double g) { averageGrade = g; }
    bool getHasScholarship() const { return hasScholarship; }
    void setHasScholarship(bool s) { hasScholarship = s; }
    bool getHasSocialBenefits() const { return hasSocialBenefits; }
    void setHasSocialBenefits(bool b) { hasSocialBenefits = b; }
    bool getHasScientificWorks() const { return hasScientificWorks; }
    void setHasScientificWorks(bool has) { hasScientificWorks = has; }
    int getConferencesCount() const { return conferencesCount; }
    void setConferencesCount(int count) { conferencesCount = count; }
    bool getIsActiveInCommunity() const { return isActiveInCommunity; }
    void setIsActiveInCommunity(bool active) { isActiveInCommunity = active; }

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

    std::string getSocialBenefitsComment() const { return socialBenefitsComment; }
    void setSocialBenefitsComment(const std::string& comment) { socialBenefitsComment = comment; }

    std::string getScientificWorksComment() const { return scientificWorksComment; }
    void setScientificWorksComment(const std::string& comment) { scientificWorksComment = comment; }

    std::string getConferencesComment() const { return conferencesComment; }
    void setConferencesComment(const std::string& comment) { conferencesComment = comment; }

    std::string getCommunityActivityComment() const { return communityActivityComment; }
    void setCommunityActivityComment(const std::string& comment) { communityActivityComment = comment; }
    void setFio(const std::string& f) { fio = f; }

    std::string getStudyFormString() const {
        return (studyForm == StudyForm::Budget) ? "Бюджет" : "Платно";
    }
    bool hasEnoughConferences(int requiredCount) const {
        return conferencesCount >= requiredCount;
    }
    std::string getFullInfo() const;       // Полная информация
    std::string getBasicInfo() const;      // Базовая информация
    std::string getAcademicInfo() const;   // Академическая информация
    std::string getCommentsInfo() const;   // Информация о комментариях
};

#endif