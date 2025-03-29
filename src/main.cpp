#include "data.h"
#include <cstdint>
#include <vector>
#include <locale>
#include <memory>
#include <algorithm>

using u32 = uint32_t;
using i32 = int32_t;

using Appointments = std::vector<Appointment>;
using Entry = std::pair<User, std::shared_ptr<Appointments>>;
using Entries = std::vector<Entry>;

static const size_t MinimumUsernameLength = 5;
static const size_t MaximumUsernameLength = 20;

static const size_t MinimumPasswordLength = 7;
static const size_t MaximumPasswordLength = 50;

static const RGB ErrorColor = RGB(255, 0, 0);
static const RGB SelectedColor = RGB(245,212,66);
static const RGB UnselectedColor = RGB(112,109,96);

Entries doctors {
    {User(L"DrSmith", "#Password123", Type::GeneralPractice), {}},
    {User(L"DrJohnson", "@securE456", Type::Cardiology), {}},
    {User(L"DrWilliams", "^Doctor789", Type::Pediatrics), {}},
    {User(L"DrBrown", ")mediC2023", Type::Neurology), {}},
    {User(L"DrDavis", "(Health2024", Type::Orthopedics), {}}
};

Entries patients {
    {User(L"EmilyClark", "Se@ure123Pass", Type::Patient), {}},
    {User(L"JacobMiller", "P@ssw0rdSafe", Type::Patient), {}},
    {User(L"SophiaBrown", "H3alth#Care2023", Type::Patient), {}},
    {User(L"NoahWilson", "Patient$789Abcd", Type::Patient), {}},
    {User(L"OliviaJones", "M3dical!Records", Type::Patient), {}}
};

void initializeAppointments() {
    static const std::vector<std::vector<Date>> dates {
        {Date(15, 3, 2025), Date(22, 7, 2025), Date(10, 11, 2025) },
        {Date(5, 2, 2025), Date(18, 6, 2025), Date(29, 9, 2025)},
        {Date(12, 4, 2025), Date(7, 8, 2025), Date(3, 12, 2025)},
        {Date(25, 1, 2025), Date(14, 5, 2025), Date(19, 10, 2025)},
        {Date(8, 3, 2025), Date(17, 7, 2025), Date(30, 11, 2025)}
    };

    static const std::vector<std::vector<size_t>> patientIndexes {
        {0, 2, 4},
        {1, 3, 0},
        {2, 4, 1},
        {3, 0, 2},
        {4, 1, 3}
    };

    static const std::vector<std::vector<size_t>> doctorIndexes {
        {0, 2, 4},
        {1, 3, 0},
        {2, 4, 1},
        {3, 0, 2},
        {4, 1, 3}
    };

    const size_t sz = doctors.size();

    for(size_t i=0; i < sz; ++i) {
        doctors[i].second = std::make_shared<Appointments>();
        doctors[i].second->reserve(sz);

        for(size_t j=0; j < dates[i].size(); ++j)
            doctors[i].second->emplace_back(dates[i][j], doctors[i].first, patients[patientIndexes[i][j]].first);
    }


    for(size_t i=0; i < sz; ++i) {
        patients[i].second = std::make_shared<Appointments>();

        patients[i].second->reserve(sz);

        for(size_t j=0; j < dates[i].size(); ++j)
            patients[i].second->emplace_back(dates[i][j], doctors[doctorIndexes[i][j]].first, patients[i].first);
    }
}


std::shared_ptr<User> user;
std::shared_ptr<Appointments> appointments;

void makeAppointment() {
    // TODO
}

void modifyAppointment(std::shared_ptr<Appointment> appointment) {
    // TODO
}

std::shared_ptr<Entry> pickEntry(const bool isDoctor) {
    return nullptr; // TODO
}

void deleteAppointment(const u8 idx) {
    appointments->erase(appointments->begin() + idx);
}

void mainServiceMenu(const bool isDoctor) {
    u8 idx = 0;

    while(true) {
        clearScreen();
        const size_t appointmentsSize = appointments->size();

        std::wcout << (isDoctor ? L"Doctor" : L"Patient") << " Actions\n\n";

        if(appointmentsSize == 0) {
            std::wcout << getCol(SelectedColor) << "No appointments made yet, " << (isDoctor ? L"" : L"press n to make one or") << L"press q to quit" << L'\n' << getCol();
            const char c = getChar();

            if(!isDoctor && c == 'n') makeAppointment();
            if(c == 'q') return;

            continue;
        }

        for(size_t i=0; i < appointmentsSize; ++i) {
            const Appointment& appointment = (*appointments)[i];

            std::wcout << getCol(idx == i ? SelectedColor : UnselectedColor);

            std::wcout << i << L") " << (isDoctor ? L"Patient:" : L"Doctor: ") << (isDoctor ? appointment.patient.name : appointment.doctor.name)
                       << L"\nDate: " << appointment.date.str() 
                       << L"\n\n" << getCol();
        }

        const char c = getChar();

        if(std::isdigit(c)) {
            const u8 digit = c - '0';

            if(digit < 1 || digit >= appointmentsSize) {
                clearScreen();
                std::wcout << getCol(ErrorColor) << L"Error: Digit input must be between 1-" << appointmentsSize << L'\n' << getCol();
                getCharV();
            }
            break;
        }
    
        switch(c) {
            case 'w': case 'a': idx = idx == 0 ? appointments->size() - 1 : idx - 1; break;
            case 's': case 'd': idx = idx == appointments->size()-1 ? 0 : idx + 1; break;
            
            case 'n':
            if(!isDoctor) makeAppointment(); 
            break;

            case 'b':

            break;
    
            case 'q': return;

            case 'y':
            deleteAppointment(idx);
            if(idx == appointmentsSize-1) --idx;
            break;
            
            default:

            break;
        }
    }
}

void mainPatientMenu() {

}

const std::pair<std::shared_ptr<User>, std::shared_ptr<Appointments>> isValidName(const std::wstring& name) {
    const auto doctor = std::find_if(doctors.begin(), doctors.end(), [&name](const Entry& doctor) { 
        return doctor.first.name == name; 
    });
    
    const auto patient = std::find_if(patients.begin(), patients.end(), [&name](const Entry& patient) { 
        return patient.first.name == name; 
    });

    return doctor == doctors.end() ? 
           (patient == patients.end() ? std::make_pair(nullptr, nullptr)
           : std::make_pair(std::make_shared<User>(patient->first), patient->second)) 
           : std::make_pair(std::make_shared<User>(doctor->first), doctor->second);
}

void execPatientMenu(const bool hasAccount) {
    std::wstring name;
    
    while(true) {
        clearScreen();
        std::wcout << (hasAccount ? L"Log In" : L"Register") << L"\n\nEnter a username (" 
                   << MinimumUsernameLength << L'-' << MaximumUsernameLength 
                   << L" characters): ";
                  
        std::string temp;
        std::cin >> temp;
        name = stw(temp);
        clearInputBuffer();

        if(const size_t sz=name.length(); !(sz >= MinimumUsernameLength && sz <= MaximumUsernameLength)) {
            std::wcout << getCol(ErrorColor) << L"\nInvalid username length " << getCol() << L"(Must be between "
                       << MinimumUsernameLength << L'-' << MaximumUsernameLength 
                       << L" characters)" << getCol();
            getCharV();
        } else if(!hasAccount && std::find_if(patients.begin(), patients.end(), [&name](const Entry& patient) { 
            return patient.first.name == name; 
        }) != patients.end()) {
            std::wcout << getCol(ErrorColor) << L"\nUsername already exists\n" << getCol();
            getCharV();
        } if(hasAccount) {
            const auto result = isValidName(name);
            user = result.first;
            appointments = result.second;

            if(user == nullptr) {
                std::wcout << getCol(ErrorColor) << L"\nThere is nobody with that username" << getCol();
                getCharV();
            } else break;
        } else break;
    }

    std::string password;

    while(true) {
        clearScreen();
        std::wcout << (hasAccount ? L"Log In" : L"Register") << L"\nUsername: " << name
                   << (hasAccount ? user->type==Type::Patient ? L"\nPatient" : L"\nDoctor" : L"")      
                   << L"\n\nEnter a password (" 
                   << MinimumPasswordLength << L'-' << MaximumPasswordLength 
                   << L" characters): ";

        std::cin >> password;
        clearInputBuffer();

        if(const size_t sz=password.length(); !(sz >= MinimumPasswordLength && sz <= MaximumPasswordLength)) {
            std::wcout << getCol(ErrorColor) << L"\nInvalid password length " << getCol() << L"(Must be between "
                       << MinimumPasswordLength << L'-' << MaximumPasswordLength 
                       << L" characters)" << getCol();
            getCharV();
        } else if(std::find_if(password.begin(), password.end(), [](const char c) { 
            return std::islower(c);
        }) == password.end()) {
            std::wcout << getCol(ErrorColor) << L"\nPassword must contain at least one lowercase letter" << getCol();
            getCharV();
        } else if(std::find_if(password.begin(), password.end(), [](const char c) { 
            return std::isupper(c);
        }) == password.end()) {
            std::wcout << getCol(ErrorColor) << L"\nPassword must contain at least one uppercase letter" << getCol();
            getCharV();
        } else if(std::find_if(password.begin(), password.end(), [](const char c) { 
            return std::isdigit(c);
        }) == password.end()) {
            std::wcout << getCol(ErrorColor) << L"\nPassword must contain at least one digit" << getCol();
            getCharV();
        } else if(std::find_if(password.begin(), password.end(), [](const char c) { 
            return !isalnum(c) && !std::isspace(c);
        }) == password.end()) {
            std::wcout << getCol(ErrorColor) << L"\nPassword must contain at least one symbol" << getCol();
            getCharV();
        } if(hasAccount && password != user->password) {
            std::wcout << getCol(ErrorColor) << L"\nInvalid password for user " << user->name << getCol();
            getCharV();
        } else break;
    }

    if(!hasAccount) {
        std::shared_ptr<Appointments> appointments_ptr = std::make_shared<Appointments>();
        patients.emplace_back(User(name, password), appointments_ptr);
        user = std::make_shared<User>(patients.back().first);
        appointments = appointments_ptr;
    }

    mainServiceMenu(user->type != Type::Patient);
}

i32 main() {
    initializeAppointments();

    #ifndef _WIN32
    initTerminalStates();
    std::locale::global (std::locale(""));
	#endif

    u8 idx = 0;
    bool running = true;
    
    while(running) {
        clearScreen();

        std::wcout << L"Do you have an existing account?";
        std::wcout << getCol(idx==0?SelectedColor:UnselectedColor) << L"\n1) Yes\n"
                   << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) No\n"
                   << getCol();

        const char c = getChar();

        if(std::isdigit(c)) {
            const u8 digit = c - '0';

            if(digit < 1 || digit > 2) {
                clearScreen();
                std::wcout << getCol(ErrorColor) << L"Error: Digit input must be between 1-2\n" << getCol();
                getCharV();
            } else execPatientMenu(digit == 1);

            break;
        } 
    
        switch(c) {
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

    cleanup(0);

    std::wcout << getCol(RGB(0, 255, 0)) << L"\n\nGoodbye!" << getCol() << std::endl;
    
#ifndef _WIN32
    std::system("reset");
#endif

    return 0;
}