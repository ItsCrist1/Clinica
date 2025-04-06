#include "serializer.h"

void Serializer::saveDate(std::ofstream& os, const Date& date) const {
    writeBF<u8>(os, date.day);
    writeBF<u8>(os, date.month);
    writeBF<u32>(os, date.year);
}

void Serializer::savePatient(std::ofstream& os, const User& user) const {
    writeWstr(os, user.name);
    writeStr(os, user.password);
}

void Serializer::saveDoctor(std::ofstream& os, const User& user) const {
    writeWstr(os, user.name);
    writeStr(os, user.password);
    writeBF<Type>(os, user.type);
}

Date Serializer::loadDate(std::ifstream& is) const {
    const u8 day = readBF<u8>(is);
    const u8 month = readBF<u8>(is);
    const u32 year = readBF<u32>(is);

    return Date(day, month, year);
}

User Serializer::loadDoctor(std::ifstream& is) const {
    const std::wstring name = readWstr(is);
    const std::string password = readStr(is);
    const Type type = readBF<Type>(is);

    return User(name, password, type);
}

User Serializer::loadPatient(std::ifstream& is) const {
    const std::wstring name = readWstr(is);
    const std::string password = readStr(is);

    return User(name, password);
}

void Serializer::loadAppointments(std::ifstream& is, std::vector<std::shared_ptr<Appointment>>& appointments) const {
    const u32 sz = readBF<u32>(is);
    appointments.reserve(sz);

    for (u32 i = 0; i < sz; ++i) {
        const Date date = loadDate(is);
        const u32 doctor = readBF<u32>(is);
        const u32 patient = readBF<u32>(is);

        appointments.push_back(std::make_shared<Appointment>(date, doctor, patient));
    }
}

Serializer::Serializer(const std::string& SaveFile) : SaveFile(SaveFile) {}

void Serializer::SaveData(const std::vector<User>& doctors, const std::vector<User>& patients, const std::vector<std::shared_ptr<Appointment>>& appointments) const {
    std::ofstream os(SaveFile, std::ios::binary);

    writeBF<u32>(os, doctors.size());
    for (const User& doctor : doctors) saveDoctor(os, doctor);

    writeBF<u32>(os, patients.size());
    for (const User& patient : patients) savePatient(os, patient);

    writeBF<u32>(os, appointments.size());
    for (std::shared_ptr<Appointment> appointment : appointments)
        saveDate(os, appointment->date),
        writeBF<u32>(os, appointment->doctorIdx),
        writeBF<u32>(os, appointment->patientIdx);

    os.close();
}

void Serializer::LoadData(std::vector<User>& doctors, std::vector<User>& patients, std::vector<std::shared_ptr<Appointment>>& appointments) const {
    std::ifstream is(SaveFile, std::ios::binary);

    u32 sz = readBF<u32>(is);
    doctors.reserve(sz);

    for (u32 i = 0; i < sz; ++i) doctors.push_back(loadDoctor(is));

    sz = readBF<u32>(is);
    patients.reserve(sz);

    for (u32 i = 0; i < sz; ++i) patients.emplace_back(loadPatient(is));

    loadAppointments(is, appointments);
    is.close();
}