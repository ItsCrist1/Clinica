#pragma once
#include "utils.h"
#include <string>
#include <memory>

struct Date {
    u8 day, month;
    u32 year;

    Date(const u8, const u8, const u32);
    Date();
    std::wstring str() const;

    Date& operator=(const Date&);
    bool operator==(const Date&);

    static const Date Default;
};

struct User {
    std::wstring name;
    std::string password;
    Type type;

    User(const std::wstring&, const std::string&, const Type type = Type::Patient);
    User(const User& other) = default;
};


struct Appointment {
    Date date;
    u32 patientIdx, doctorIdx;

    Appointment(const Date, const u32, const u32);
    Appointment();
};
