#include "data.h"

Date::Date(const u8 day, const u8 month, const u32 year) : day(day), month(month), year(year) {}
std::wstring Date::str() const {
    std::wstringstream wss;
    wss << (day < 10 ? L"0" : L"") << day << L'.' << (month < 10 ? L"0" : L"") << month <<  L'.' << year;
    return wss.str();
}

User::User(const std::string& name, const std::string& password, const Type type) 
: name(name), password(password), type(type) {}

Appointment::Appointment(const Date date, const User& doctor, const User& patient)
: date(date), doctor(doctor), patient(patient) {}