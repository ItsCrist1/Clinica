#include "clinic.h"
#include <algorithm>

void Clinic::saveData() const {
	serializer.SaveData(doctors, patients, appointments);
}

void Clinic::initializeData() {
    patients = DefaultPatients;
    doctors = DefaultDoctors;
    appointments = DefaultAppointments;
}

void Clinic::fetchAppointments(const bool isDoctor) {
    const u32 sz = appointments.size();
    CurrentAppointments.reserve(sz);

    for (u32 i=0; i < sz; ++i)
        if ((isDoctor ? doctors[appointments[i]->doctorIdx].name : patients[appointments[i]->patientIdx].name) == CurrentUser->name)
            CurrentAppointments.push_back(appointments[i]);
}

void Clinic::modifyDate(Date& date) const {
    u8 idx = 0;

    while (true) {
        clearScreen();

        std::wcout << L"Select which part to modify: "
            << (idx == 0 ? SelectedColor : UnselectedColor) << (date.day < 10 ? L"0" : L"") << date.day << getCol() << L'.'
            << (idx == 1 ? SelectedColor : UnselectedColor) << (date.month < 10 ? L"0" : L"") << date.month << getCol() << L'.'
            << (idx == 2 ? SelectedColor : UnselectedColor) << date.year << getCol();

        const char c = getChar();

        if (std::isdigit(c)) {
            const u8 digit = c - '0';

            if (digit < 1 || digit > 3) {
                clearScreen();
                std::wcout << ErrorColor << L"Error: Digit input must be between 1-3\n" << getCol();
                getCharV();
                continue;
            }

            idx = digit - 1;
        }

        switch (c) {
        case 'w': case 'a': idx = idx == 0 ? 2 : idx - 1; break;
        case 's': case 'd': idx = idx == 2 ? 0 : idx + 1; break;

        case 'q': return;

        default:
            std::wcout << L"\n\nEnter a new " << (idx == 0 ? L"day" : idx == 1 ? L"month" : L"year") << L" (between "
                << (idx == 0 ? L"1-31" : idx == 1 ? L"1-12" : std::to_wstring(CurrentYear) + L"-2030") << L"): ";

            u32 input;
            std::cin >> input;
            clearInputBuffer();

            switch (idx) {
                case 0:
                if (!(input > 0 && input <= 31)) {
                    std::wcout << ErrorColor << L"Invalid day input, it must be between 1 and 31" << getCol();
                    getCharV();
                    continue;
                }

                date.day = input;
                break;

                case 1:
                if (!(input > 0 && input <= 12)) {
                    std::wcout << ErrorColor << L"Invalid month input, it must be between 1 and 12" << getCol();
                    getCharV();
                    continue;
                }

                date.month = input;
                break;

                case 2:
                if (!(input >= CurrentYear && input <= 2030)) {
                    std::wcout << ErrorColor << L"Invalid day input, it must be between " << CurrentYear << L" and 2030" << getCol();
                    getCharV();
                    continue;
                }

                date.year = input;
                break;
            } break;
        }
    }
}

std::pair<std::shared_ptr<User>, u32> Clinic::pickUser(const bool isDoctor, const Date& date) const {
    u8 idx = 0;

    std::vector<User> freeDoctors;
    
    if(!isDoctor) {
        const u32 sz = doctors.size();

        std::vector<bool> freeDoctorFlags (sz, true);
        
        for(std::shared_ptr<Appointment> appointment : appointments)
            if(appointment->date == date) 
                freeDoctorFlags[appointment->doctorIdx] = false;
        
        freeDoctors.reserve(sz);
        for(u32 i=0; i < sz; ++i) {
            if(freeDoctorFlags[i])
                freeDoctors.push_back(doctors[i]);
        }
    }

    const u32 fdsz = freeDoctors.size();

    while (true) {
        clearScreen();

        const u32 sz = !isDoctor ? doctors.size() : patients.size();

        if(isDoctor)
            for (u32 i=0; i < sz; ++i)
                std::wcout << (idx==i ? SelectedColor : UnselectedColor)
                           << i + 1 << L") " << patients[i].name
                           << L'\n' << getCol();

        else
            for(u32 i=0; i < fdsz; ++i)
                std::wcout << (idx==i ? SelectedColor : UnselectedColor)
                           << i + 1 << L") " << freeDoctors[i].name
                           << L"\nSpecialization: " << getTypeWstr(freeDoctors[i].type)
                           << L'\n' << getCol();

        const char c = getChar();

        if (std::isdigit(c)) {
            const u8 digit = c - '0';

            if (digit < 1 || digit > (isDoctor ? sz : fdsz)) {
                clearScreen();
                std::wcout << ErrorColor << L"Error: Digit input must be between 1-" << sz << L'\n' << getCol();
                getCharV();
                continue;
            }

            idx = digit - 1;
            continue;
        }

        switch (c) {
            case 'w': case 'a': idx = idx == 0 ? static_cast<u8>((isDoctor ? sz : fdsz) - 1) : idx - 1; break;
            case 's': case 'd': idx = idx == (isDoctor ? sz : fdsz) - 1 ? 0 : idx + 1; break;

            case 'q': return std::make_pair(nullptr, 0);

            default:
            if (isDoctor)
                return std::make_pair(std::make_shared<User>(patients[idx]), idx);
            else
                return std::make_pair(std::make_shared<User>(freeDoctors[idx]), std::distance(doctors.begin(), 
                       std::find_if(doctors.begin(), doctors.end(), 
                       [&freeDoctors, idx](const User& doctor) { return doctor.name == freeDoctors[idx].name; })));
            break;
        }
    }
}

void Clinic::createAppointment() {
    Date date (0, 0, CurrentYear);
    modifyDate(date);

    std::shared_ptr<Appointment> appointment = std::make_shared<Appointment>(date, pickUser(false, date).second, CurrentIdx);

    appointments.push_back(appointment);
    CurrentAppointments.push_back(appointment);
}

void Clinic::deleteAppointment(const u8 idx) {
    if (const auto t = std::find(appointments.begin(), appointments.end(), CurrentAppointments[idx]);
        t != appointments.end()) appointments.erase(t);

    CurrentAppointments.erase(CurrentAppointments.begin() + idx);
}

void Clinic::mainServiceMenu(const bool isDoctor) {
    u8 idx = 0;

    while (true) {
        clearScreen();
        const u32 sz = CurrentAppointments.size();

        std::wcout << (isDoctor ? L"Doctor" : L"Patient") << " Actions\n\n";

        if (sz == 0) {
            std::wcout << SelectedColor << "No appointments made yet, " << (isDoctor ? L"" : L"press n to make one or ") << L"press q to quit" << L'\n' << getCol();
            const char c = getChar();

            if (!isDoctor && c == 'n') createAppointment();
            if (c == 'q') return;

            continue;
        }

        for (u32 i=0; i < sz; ++i) {
            std::shared_ptr<Appointment> appointment = CurrentAppointments[i];

            std::wcout << (idx == i ? SelectedColor : UnselectedColor)
                << i + 1 << L") " << (isDoctor ? L"Patient:" : L"Doctor: ") << (isDoctor ? patients[appointments[i]->patientIdx].name : doctors[appointments[i]->doctorIdx].name)
                << L"\nDate: " << appointment->date.str()
                << (!isDoctor ? L"\nSpecialization: " + getTypeWstr(doctors[appointments[i]->doctorIdx].type) : L"")
                << L"\n\n" << getCol();
        }

        const char c = getChar();

        if (std::isdigit(c)) {
            const u8 digit = c - '0';

            if (digit < 1 || digit > sz) {
                clearScreen();
                std::wcout << ErrorColor << L"Error: Digit input must be between 1-" << sz << L'\n' << getCol();
                getCharV();
                continue;
            }

            idx = digit - 1;
        }

        switch (c) {
            case 'w': case 'a': idx = idx == 0 ? static_cast<u8>(sz - 1) : idx - 1; break;
            case 's': case 'd': idx = idx == sz - 1 ? 0 : idx + 1; break;

            case 'n':
            if (!isDoctor) createAppointment();
            else std::wcout << ErrorColor << L"As a doctor, you cannot create new appointment" << getCol();
            getCharV();
            saveData();
            break;

            case 'b':
            modifyDate(CurrentAppointments[idx]->date);
            saveData();
            break;

            case 'v':
            if (std::shared_ptr<User> user = pickUser(isDoctor).first) {
                if (isDoctor) patients[CurrentAppointments[idx]->patientIdx] = *user;
                else doctors[CurrentAppointments[idx]->doctorIdx] = *user;

                saveData();
            }
            break;

            case 'g': 
			std::wcout << SelectedColor << L"Data saved successfully!\n" << getCol();
            getCharV();
            saveData(); 
            break;

            case 'q': return;

            case 'y':
            deleteAppointment(idx);
            if (idx == sz - 1) --idx;
            break;

            default: break;
        }
    }
}

std::pair<std::shared_ptr<User>, u32> Clinic::isValidName(const std::wstring& name) const {
    const auto doctor = std::find_if(doctors.begin(), doctors.end(), [&name](const User& doctor) {
        return doctor.name == name;
        });

    if (doctor != doctors.end())
        return std::make_pair(std::make_shared<User>(*doctor), std::distance(doctors.begin(), doctor));

    const auto patient = std::find_if(patients.begin(), patients.end(), [&name](const User& patient) {
        return patient.name == name;
        });

    if (patient != patients.end())
        return std::make_pair(std::make_shared<User>(*patient), std::distance(patients.begin(), patient));

    return std::make_pair(nullptr, 0);
}

bool Clinic::showPasswordError(const bool condition, const std::wstring& errorMessage) const {
    if (!condition) return false;

    std::wcout << ErrorColor << L'\n' << errorMessage << getCol();

    getCharV();
    return true;
}

void Clinic::execPatientMenu(const bool hasAccount) {
    std::wstring name;

    while (true) {
        clearScreen();
        std::wcout << (hasAccount ? L"Log In" : L"Register") << L"\n\nEnter a username ("
            << MinimumUsernameLength << L'-' << MaximumUsernameLength
            << L" characters): ";

        std::string temp;
        std::cin >> temp;
        name = stw(temp);
        clearInputBuffer();

        if (const u32 sz = name.length(); !(sz >= MinimumUsernameLength && sz <= MaximumUsernameLength)) {
            std::wcout << ErrorColor << L"\nInvalid username length " << getCol() << L"(Must be between "
                << MinimumUsernameLength << L'-' << MaximumUsernameLength
                << L" characters)" << getCol();
            getCharV();
        }
        else if (!hasAccount && std::find_if(patients.begin(), patients.end(), [&name](const User& patient) {
            return patient.name == name;
            }) != patients.end()) {
            std::wcout << ErrorColor << L"\nUsername already exists\n" << getCol();
            getCharV();
        }
        else if (hasAccount) {
            std::pair<std::shared_ptr<User>, u32> result = isValidName(name);

            if (result.first == nullptr) {
                std::wcout << ErrorColor << L"\nThere is nobody with that username" << getCol();
                getCharV();
                continue;
            }

            CurrentUser = result.first;
            CurrentIdx = result.second;
            break;
        }
        else break;
    }

    std::string password;

    while (true) {
        clearScreen();
        std::wcout << (hasAccount ? L"Log In" : L"Register") << L"\nUsername: " << name
            << (hasAccount ? CurrentUser->type == Type::Patient ? L"\nPatient" : L"\nDoctor" : L"")
            << L"\n\nEnter a password ("
            << MinimumPasswordLength << L'-' << MaximumPasswordLength
            << L" characters): ";

        std::cin >> password;
        clearInputBuffer();

        if (const u32 sz = password.length(); !(sz >= MinimumPasswordLength && sz <= MaximumPasswordLength)) {
            std::wcout << ErrorColor << L"\nInvalid password length " << getCol() << L"(Must be between "
                << MinimumPasswordLength << L'-' << MaximumPasswordLength
                << L" characters)" << getCol();

            getCharV();
            continue;
        }
        else if (hasAccount && password != CurrentUser->password) {
            std::wcout << ErrorColor << L"\nInvalid password for user " << CurrentUser->name << getCol();

            getCharV();
            continue;
        }

        bool hasLower = false, hasUpper = false, hasDigit = false, hasSymbol = false;

        for (const char c : password)
            if (!hasLower && std::islower(c)) hasLower = true;
            else if (!hasUpper && std::isupper(c)) hasUpper = true;
            else if (!hasDigit && std::isdigit(c)) hasDigit = true;
            else if (!hasSymbol && !isalnum(c) && !std::isspace(c)) hasSymbol = true;

        if (showPasswordError(!hasLower, L"Password must contain at least one lowercase letter") ||
            showPasswordError(!hasUpper, L"Password must contain at least one uppercase letter") ||
            showPasswordError(!hasDigit, L"Password must contain at least one digit") ||
            showPasswordError(!hasSymbol, L"Password must contain at least one symbol"))
            continue;

        break;
    }

    if (!hasAccount) {
        patients.emplace_back(name, password);
        CurrentUser = std::make_shared<User>(patients.back());
        CurrentIdx = patients.size() - 1;
        fetchAppointments(false);
    }
    else fetchAppointments(CurrentUser->type != Type::Patient);

    mainServiceMenu(CurrentUser->type != Type::Patient);
}

Clinic::Clinic(const std::string& saveFile) : serializer(saveFile) {
    if (fs::is_regular_file(saveFile)) serializer.LoadData(doctors, patients, appointments);
    else initializeData(), saveData();
}

void Clinic::MainMenu() {
    u8 idx = 0;
    bool running = true;

    while (running) {
        clearScreen();

        std::wcout << L"--- Clinic System ---\n\n"
            << L"Do you have an existing account?"
            << (idx == 0 ? SelectedColor : UnselectedColor) << L"\n1) Yes\n"
            << (idx == 1 ? SelectedColor : UnselectedColor) << L"2) No\n"
            << getCol();

        const char c = getChar();

        if (std::isdigit(c)) {
            const u8 digit = c - '0';

            if (digit < 1 || digit > 2) {
                clearScreen();
                std::wcout << ErrorColor << L"Error: Digit input must be between 1-2\n" << getCol();
                getCharV();
            }
            else execPatientMenu(digit == 1);

            break;
        }

        switch (c) {
        case 'w': case 's':
        case 'a': case 'd':
            idx = idx == 0 ? 1 : 0; break;

        case 'q': running = false; break;

        default:
            execPatientMenu(idx == 0);
            running = false;
            break;
        }
    }
}