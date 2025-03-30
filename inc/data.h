#pragma once
#include "utils.h"
#include <string>
#include <memory>

struct Date {
    u8 day, month;
    u32 year;

    Date(const u8, const u8, const u32);
    std::wstring str() const;

    Date& operator=(const Date&);
};

struct User {
    std::wstring name;
    std::string password;
    Type type;

    User(const std::wstring&, const std::string&, const Type type = Type::Patient);
};


struct Appointment {
    Date date;
    std::shared_ptr<User> patient, doctor;

    Appointment(const Date, std::shared_ptr<User>, std::shared_ptr<User>);
    Appointment();
};
