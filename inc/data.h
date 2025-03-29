#pragma once
#include "utils.h"
#include <string>

enum class Type {
    GeneralPractice, Cardiology, Dermatology,
    Neurology, Pediatrics, Orthopedics,
    Gynecology, InternalMedicine, Surgery,
    Patient
};

struct Date {
    const u8 day, month;
    const u32 year;

    Date(const u8, const u8, const u32);
    std::wstring str() const;
};

struct User {
    const std::string name;
    const std::string password;
    const Type type;

    User(const std::string&, const std::string&, const Type type = Type::Patient);
};


struct Appointment {
    const Date date;
    const User doctor, patient;

    Appointment(const Date, const User&, const User&);
};