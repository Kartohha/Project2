#include "Student.h"
#include <sstream>
#include <iomanip>

Student::Student(const std::string& user, const std::string& hash,
    const std::string& fio_, double avg, bool hasScholarship,
    bool hasSocialBenefits, bool hasScientificWorks,
    int conferences, bool activeInCommunity,
    StudyForm form, int course,
    const std::string& group, const std::string& faculty,
    const std::string& specialty, const std::string& scholarshipType,
    const std::string& socialBenefitsComment,
    const std::string& scientificWorksComment,
    const std::string& conferencesComment,
    const std::string& communityActivityComment)
    : IUser(user, hash), fio(fio_), averageGrade(avg),
    hasScholarship(hasScholarship), hasSocialBenefits(hasSocialBenefits),
    hasScientificWorks(hasScientificWorks), conferencesCount(conferences),
    isActiveInCommunity(activeInCommunity), studyForm(form),
    course(course), group(group), faculty(faculty),
    specialty(specialty), scholarshipType(scholarshipType),
    socialBenefitsComment(socialBenefitsComment),
    scientificWorksComment(scientificWorksComment),
    conferencesComment(conferencesComment),
    communityActivityComment(communityActivityComment) {}

std::string Student::getFullInfo() const {
    std::stringstream ss;
    ss << "===== ПОЛНАЯ ИНФОРМАЦИЯ О СТУДЕНТЕ =====\n\n";
    ss << "ОСНОВНАЯ ИНФОРМАЦИЯ:\n";
    ss << "  ФИО: " << fio << "\n";
    ss << "  Логин: " << username << "\n";
    ss << "  Форма обучения: " << getStudyFormString() << "\n";
    ss << "  Курс: " << course << "\n";
    ss << "  Группа: " << group << "\n";
    ss << "  Факультет: " << faculty << "\n";
    ss << "  Специальность: " << specialty << "\n\n";

    ss << "АКАДЕМИЧЕСКАЯ ИНФОРМАЦИЯ:\n";
    ss << "  Средний балл: " << std::fixed << std::setprecision(2) << averageGrade << "/10.0\n";
    ss << "  Стипендия: " << (hasScholarship ? "Назначена" : "Не назначена") << "\n";
    if (hasScholarship && !scholarshipType.empty()) {
        ss << "  Тип стипендии: " << scholarshipType << "\n";
    }
    ss << "  Социальные льготы: " << (hasSocialBenefits ? "Да" : "Нет") << "\n";
    ss << "  Научные работы: " << (hasScientificWorks ? "Да" : "Нет") << "\n";
    ss << "  Участие в конференциях: " << conferencesCount << " раз(а)\n";
    ss << "  Общественная активность: " << (isActiveInCommunity ? "Да" : "Нет") << "\n\n";

    // Добавляем информацию о комментариях
    ss << "КОММЕНТАРИИ И ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ:\n";
    if (!socialBenefitsComment.empty()) {
        ss << "  Социальные льготы (комментарий): " << socialBenefitsComment << "\n";
    }
    if (!scientificWorksComment.empty()) {
        ss << "  Научные работы (комментарий): " << scientificWorksComment << "\n";
    }
    if (!conferencesComment.empty()) {
        ss << "  Участие в конференциях (комментарий): " << conferencesComment << "\n";
    }
    if (!communityActivityComment.empty()) {
        ss << "  Общественная активность (комментарий): " << communityActivityComment << "\n";
    }

    return ss.str();
}

std::string Student::getBasicInfo() const {
    std::stringstream ss;
    ss << "ФИО: " << fio << " | ";
    ss << "Логин: " << username << " | ";
    ss << "Курс: " << course << " | ";
    ss << "Группа: " << group << " | ";
    ss << "Факультет: " << faculty << " | ";
    ss << "Обучение: " << getStudyFormString() << " | ";
    ss << "Стипендия: " << (hasScholarship ? (scholarshipType.empty() ? "Да" : scholarshipType) : "Нет");
    return ss.str();
}

std::string Student::getAcademicInfo() const {
    std::stringstream ss;
    ss << "Средний балл: " << std::fixed << std::setprecision(2) << averageGrade << "/10.0\n";
    ss << "Специальность: " << specialty << "\n";
    ss << "Научные работы: " << (hasScientificWorks ? "Да" : "Нет") << "\n";
    ss << "Конференции: " << conferencesCount << "\n";
    ss << "Общественная активность: " << (isActiveInCommunity ? "Да" : "Нет") << "\n";
    ss << "Социальные льготы: " << (hasSocialBenefits ? "Да" : "Нет") << "\n";
    return ss.str();
}

std::string Student::getCommentsInfo() const {
    std::stringstream ss;
    ss << "КОММЕНТАРИИ:\n";

    if (!socialBenefitsComment.empty()) {
        ss << "  Социальные льготы: " << socialBenefitsComment << "\n";
    }
    else {
        ss << "  Социальные льготы: нет комментария\n";
    }

    if (!scientificWorksComment.empty()) {
        ss << "  Научные работы: " << scientificWorksComment << "\n";
    }
    else {
        ss << "  Научные работы: нет комментария\n";
    }

    if (!conferencesComment.empty()) {
        ss << "  Участие в конференциях: " << conferencesComment << "\n";
    }
    else {
        ss << "  Участие в конференциях: нет комментария\n";
    }

    if (!communityActivityComment.empty()) {
        ss << "  Общественная активность: " << communityActivityComment << "\n";
    }
    else {
        ss << "  Общественная активность: нет комментария\n";
    }

    return ss.str();
}