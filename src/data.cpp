#include "data.h"

std::wstring getTypeWstr(const Type type) {
    switch (type) {
        case Type::GeneralPractice: return L"GeneralPractice";
        case Type::Cardiology: return L"Cardiology";
        case Type::Dermatology: return L"Dermatology";
        case Type::Neurology: return L"Neurology";
        case Type::Pediatrics: return L"Pediatrics";
        case Type::Orthopedics: return L"Orthopedics";
        case Type::Gynecology: return L"Gynecology";
        case Type::InternalMedicine: return L"InternalMedicine";
        case Type::Surgery: return L"Surgery";
        case Type::Patient: return L"Patient";
        
        default: return L"Unknown";
    }
}

Date::Date(const u8 day, const u8 month, const u32 year) : day(day), month(month), year(year) {}
std::wstring Date::str() const {
    std::wstringstream wss;
    wss << (day < 10 ? L"0" : L"") << day << L'.' << (month < 10 ? L"0" : L"") << month <<  L'.' << year;
    return wss.str();
}

Date& Date::operator=(const Date& other) {
    day = other.day;
    month = other.month;
    year = other.year;
    return *this;
}

User::User(const std::wstring& name, const std::string& password, const Type type) 
: name(name), password(password), type(type) {}

Appointment::Appointment(const Date date, std::shared_ptr<User> doctor, std::shared_ptr<User> patient)
: date(date), doctor(doctor), patient(patient) {}