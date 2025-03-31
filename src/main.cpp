#include "data.h"
#include <cstdint>
#include <vector>
#include <algorithm>

#ifndef _WIN32
#include <locale>
#else
#include <windows.h>
#include <corecrt.h>
#include <fcntl.h>
#include <io.h>
#endif

using u32 = uint32_t;
using i32 = int32_t;

using Appointments = std::vector<Appointment>;
using Entry = std::pair<User, std::shared_ptr<Appointments>>;
using Entries = std::vector<Entry>;

static const std::string SaveFile = "data.dat";

static const u32 CurrentYear = 2025;

static const u32 MinimumUsernameLength = 5;
static const u32 MaximumUsernameLength = 20;

static const u32 MinimumPasswordLength = 7;
static const u32 MaximumPasswordLength = 50;

static const RGB ErrorColor = {255, 0, 0};
static const RGB SelectedColor = {245, 212, 66};
static const RGB UnselectedColor = {112, 109, 96};

static const Entries DefaultDoctors {
    {User(L"DrSmith", "#Password123", Type::GeneralPractice), {}},
    {User(L"DrJohnson", "@securE456", Type::Cardiology), {}},
    {User(L"DrWilliams", "^Doctor789", Type::Pediatrics), {}},
    {User(L"DrBrown", ")mediC2023", Type::Neurology), {}},
    {User(L"DrDavis", "(Health2024", Type::Orthopedics), {}}
};

static const Entries DefaultPatients {
    {User(L"EmilyClark", "Se@ure123Pass", Type::Patient), {}},
    {User(L"JacobMiller", "P@ssw0rdSafe", Type::Patient), {}},
    {User(L"SophiaBrown", "H3alth#Care2023", Type::Patient), {}},
    {User(L"NoahWilson", "Patient$789Abcd", Type::Patient), {}},
    {User(L"OliviaJones", "M3dical!Records", Type::Patient), {}}
};

std::shared_ptr<User> user;
std::shared_ptr<Appointments> appointments;

Entries patients, doctors;

void initializeAppointments() {
    static const std::vector<std::vector<Date>> dates {
        {Date(15, 3, CurrentYear), Date(22, 7, CurrentYear), Date(10, 11, CurrentYear) },
        {Date(5, 2, CurrentYear), Date(18, 6, CurrentYear), Date(29, 9, CurrentYear)},
        {Date(12, 4, CurrentYear), Date(7, 8, CurrentYear), Date(3, 12, CurrentYear)},
        {Date(25, 1, CurrentYear), Date(14, 5, CurrentYear), Date(19, 10, CurrentYear)},
        {Date(8, 3, CurrentYear), Date(17, 7, CurrentYear), Date(30, 11, CurrentYear)}
    };

    static const std::vector<std::vector<u32>> patientIndexes {
        {0, 2, 4}, {1, 3, 0}, {2, 4, 1}, {3, 0, 2}, {4, 1, 3}
    };

    static const std::vector<std::vector<u32>> doctorIndexes {
        {0, 2, 4}, {1, 3, 0}, {2, 4, 1}, {3, 0, 2}, {4, 1, 3}
    };

    const u32 sz = doctors.size();

    for(u32 i=0; i < sz; ++i) {
        doctors[i].second = std::make_shared<Appointments>();
        doctors[i].second->reserve(sz);

        for(u32 j=0; j < dates[i].size(); ++j)
            doctors[i].second->emplace_back(dates[i][j], std::make_shared<User>(doctors[i].first), std::make_shared<User>(patients[patientIndexes[i][j]].first));
    }


    for(u32 i=0; i < sz; ++i) {
        patients[i].second = std::make_shared<Appointments>();

        patients[i].second->reserve(sz);

        for(u32 j=0; j < dates[i].size(); ++j)
            patients[i].second->emplace_back(dates[i][j], std::make_shared<User>(doctors[doctorIndexes[i][j]].first), std::make_shared<User>(patients[i].first));
    }
}

void initializeData() {
    patients = DefaultPatients;
    doctors = DefaultDoctors;
    initializeAppointments();
}

void saveDate(std::ofstream& os, const Date& date) {
    writeBF<u8>(os, date.day);
    writeBF<u8>(os, date.month);
    writeBF<u32>(os, date.year);
}

void savePatient(std::ofstream& os, const User& user) {
    writeWstr(os, user.name);
    writeStr(os, user.password);
}

void saveDoctor(std::ofstream& os, const User& user) {
    writeWstr(os, user.name);
    writeStr(os, user.password);
    writeBF<Type>(os, user.type);
}

Date loadDate(std::ifstream& is) {
    return Date (readBF<u8>(is), readBF<u8>(is), readBF<u32>(is));
}

std::shared_ptr<User> loadDoctor(std::ifstream& is) {
    return std::make_shared<User> (User(readWstr(is), readStr(is), readBF<Type>(is)));
}

std::shared_ptr<User> loadPatient(std::ifstream& is) {
    return std::make_shared<User> (User(readWstr(is), readStr(is)));
}

std::shared_ptr<Appointments> loadAppointments(std::ifstream& is, const bool isDoctor, std::shared_ptr<User> user) {
    const u32 sz = readBF<u32>(is);
    std::shared_ptr<Appointments> appointments = std::make_shared<Appointments>();
    appointments->reserve(sz);

    for(u32 i=0; i < sz; ++i) {
        const Date date = loadDate(is);
        
        appointments->emplace_back(date, isDoctor ? user : loadDoctor(is), !isDoctor ? user : loadPatient(is));
    } return appointments;
}

void saveData() {
    std::ofstream os (SaveFile, std::ios::binary);
    writeBF<u32>(os, doctors.size());

    for(const Entry& entry : doctors) {
        saveDoctor(os, entry.first);

        writeBF<u32>(os, entry.second ? entry.second->size() : 0);
        if(entry.second) {
            for (const Appointment& appointment : *entry.second)
                saveDate(os, appointment.date),
                savePatient(os, *appointment.patient);
        }
    }

    writeBF<u32>(os, patients.size());

    for(const Entry& entry : patients) {
        savePatient(os, entry.first);

        writeBF<u32>(os, entry.second ? entry.second->size() : 0);
        if(entry.second) {
            for (const Appointment& appointment : *entry.second)
                saveDate(os, appointment.date),
                saveDoctor(os, *appointment.doctor);
        }
    }

    os.close();
}

void loadData() {
    std::ifstream is (SaveFile, std::ios::binary);
    u32 sz = readBF<u32>(is);
    
    doctors.clear();
    doctors.reserve(sz);
    
    for(u32 i=0; i < sz; ++i) {
        std::shared_ptr<User> doctor = loadDoctor(is);
        doctors.emplace_back(*doctor, loadAppointments(is, true, doctor));
    }
    
    sz = readBF<u32>(is);
    
    patients.clear();
    patients.reserve(sz);
    
    for(u32 i=0; i < sz; ++i) {
        std::shared_ptr<User> patient = loadPatient(is);
        patients.emplace_back(*patient, loadAppointments(is, false, patient));
    }
    
    is.close();
}

void modifyDate(Date& date) {
    u8 idx = 0;
    
    while(true) {
        clearScreen();
        
        std::wcout << L"Select which part to modify: "
        << getCol(idx==0?SelectedColor:UnselectedColor) << (date.day < 10 ? L"0" : L"") << date.day << getCol() << L'.'
        << getCol(idx==1?SelectedColor:UnselectedColor) << (date.month < 10 ? L"0" : L"") << date.month << getCol() << L'.'
        << getCol(idx==2?SelectedColor:UnselectedColor) << date.year << getCol();
        
        const char c = getChar();
        
        if(std::isdigit(c)) {
            const u8 digit = c - '0';
            
            if(digit < 1 || digit > 3) {
                clearScreen();
                std::wcout << getCol(ErrorColor) << L"Error: Digit input must be between 1-3\n" << getCol();
                getCharV();
                continue;
            }
            
            idx = digit - 1;
        }
        
        switch(c) {
            case 'w': case 'a': idx = idx == 0 ? 2 : idx - 1; break;
            case 's': case 'd': idx = idx == 2 ? 0 : idx + 1; break;
            
            case 'q': return;
            
            default:
            std::wcout << L"\n\nEnter a new " << (idx==0 ? L"day" : idx==1 ? L"month" : L"year") << L" (between "
                       << (idx==0 ? L"1-31" : idx==1 ? L"1-12" : std::to_wstring(CurrentYear) + L"-2030") << L"): ";
            
            u32 input;
            std::cin >> input;
            clearInputBuffer();
            
            switch(idx) {
                case 0:
                if(!(input > 0 && input <= 31)) {
                    std::wcout << getCol(ErrorColor) << L"Invalid day input, it must be between 1 and 31" << getCol();
                    getCharV();
                    continue;
                }
                
                date.day = input;
                break;
                
                case 1:
                if(!(input > 0 && input <= 12)) {
                    std::wcout << getCol(ErrorColor) << L"Invalid month input, it must be between 1 and 12" << getCol();
                    getCharV();
                    continue;
                }
                
                date.month = input;
                break;
                
                case 2:
                if(!(input >= CurrentYear && input <= 2030)) {
                    std::wcout << getCol(ErrorColor) << L"Invalid day input, it must be between " << CurrentYear << L" and 2030" << getCol();
                    getCharV();
                    continue;
                }
                
                date.year = input;
                break;
            }
            break;
        }
    }
}

std::shared_ptr<User> pickUser(const bool isDoctor) {
    u8 idx = 0;
    
    while(true) {
        clearScreen();
        
        const u32 sz = !isDoctor ? doctors.size() : patients.size();
        
        for(u32 i=0; i < sz; ++i) {
            const User user = !isDoctor ? doctors[i].first : patients[i].first;
            
            std::wcout << getCol(idx == i ? SelectedColor : UnselectedColor)
            << i+1 << L") " << user.name
            << (!isDoctor ? L": " + getTypeWstr(user.type) : L"")
            << L'\n' << getCol();
        }
        
        const char c = getChar();
        
        if(std::isdigit(c)) {
            const u8 digit = c - '0';
            
            if(digit < 1 || digit > sz) {
                clearScreen();
                std::wcout << getCol(ErrorColor) << L"Error: Digit input must be between 1-" << sz << L'\n' << getCol();
                getCharV();
                continue;
            }
            
            idx = digit - 1;
            continue;
        }
        
        switch(c) {
            case 'w': case 'a': idx = idx == 0 ? static_cast<u8>(sz - 1) : idx - 1; break;
            case 's': case 'd': idx = idx == sz-1 ? 0 : idx + 1; break;
            
            case 'q': return nullptr;
            
            default:
            return std::make_shared<User>((!isDoctor ? doctors[idx] : patients[idx]).first);
            break;
        }
    }
}

void createAppointment() {
    Date date (0,0, CurrentYear);
    modifyDate(date);

    appointments->emplace_back(date, pickUser(false), user);
}

void deleteAppointment(const u8 idx) {
    appointments->erase(appointments->begin() + idx);
}

void mainServiceMenu(const bool isDoctor) {
    u8 idx = 0;
    
    while(true) {
        clearScreen();
        const u32 sz = appointments->size();

        std::wcout << (isDoctor ? L"Doctor" : L"Patient") << " Actions\n\n";

        if(sz == 0) {
            std::wcout << getCol(SelectedColor) << "No appointments made yet, " << (isDoctor ? L"" : L"press n to make one or ") << L"press q to quit" << L'\n' << getCol();
            const char c = getChar();

            if(!isDoctor && c == 'n') createAppointment();
            if(c == 'q') return;

            continue;
        }

        for(u32 i=0; i < sz; ++i) {
            const Appointment& appointment = (*appointments)[i];

            std::wcout << getCol(idx == i ? SelectedColor : UnselectedColor)
                       << i+1 << L") " << (isDoctor ? L"Patient:" : L"Doctor: ") << (isDoctor ? appointment.patient->name : appointment.doctor->name)
                       << L"\nDate: " << appointment.date.str() 
                       << (!isDoctor ? L"\nSpecialization: " + getTypeWstr(appointment.doctor->type) : L"")
                       << L"\n\n" << getCol();
        }

        const char c = getChar();

        if(std::isdigit(c)) {
            const u8 digit = c - '0';

            if(digit < 1 || digit > sz) {
                clearScreen();
                std::wcout << getCol(ErrorColor) << L"Error: Digit input must be between 1-" << sz << L'\n' << getCol();
                getCharV();
                continue;
            }
            
            idx = digit - 1;
        }
    
        switch(c) {
            case 'w': case 'a': idx = idx == 0 ? static_cast<u8>(sz - 1) : idx - 1; break;
            case 's': case 'd': idx = idx == sz-1 ? 0 : idx + 1; break;
            
            case 'n':
            if(!isDoctor) createAppointment();
            else std::wcout << getCol(ErrorColor) << L"As a doctor, you cannot create new appointment" << getCol();
            getCharV();
            break;

            case 'b':
            modifyDate(appointments->at(idx).date);
            break;

            case 'v':
            if (std::shared_ptr<User> user = pickUser(isDoctor)) {
                if(isDoctor) appointments->at(idx).patient = user;
                else appointments->at(idx).doctor = user;
            }
            break;
    
            case 'q': return;

            case 'y':
            deleteAppointment(idx);
            if(idx == sz-1) --idx;
            break;
            
            default: break;
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

        if(const u32 sz=name.length(); !(sz >= MinimumUsernameLength && sz <= MaximumUsernameLength)) {
            std::wcout << getCol(ErrorColor) << L"\nInvalid username length " << getCol() << L"(Must be between "
                       << MinimumUsernameLength << L'-' << MaximumUsernameLength 
                       << L" characters)" << getCol();
            getCharV();
        } else if(!hasAccount && std::find_if(patients.begin(), patients.end(), [&name](const Entry& patient) { 
            return patient.first.name == name; 
        }) != patients.end()) {
            std::wcout << getCol(ErrorColor) << L"\nUsername already exists\n" << getCol();
            getCharV();
        } else if(hasAccount) {
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

        if(const u32 sz=password.length(); !(sz >= MinimumPasswordLength && sz <= MaximumPasswordLength)) {
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
        } else if(hasAccount && password != user->password) {
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
    #ifndef _WIN32
    initTerminalStates();
    std::locale::global (std::locale(""));
	#else
    if(!_setmode(_fileno(stdout), _O_U16TEXT)) {
        std::wcerr << L"Unable to set UTF-16 to the terminal, some symbols or colors may not render correctly, do you wish to continue? [y/N] ";
        if(std::tolower(getChar()) != 'y') return 1;
    }
    #endif

    if(fs::is_regular_file(SaveFile)) loadData();
    else initializeData(), saveData();

    u8 idx = 0;
    bool running = true;

    
    while(running) {
        clearScreen();

        std::wcout << L"--- Clinic System ---\n\n"
                   << L"Do you have an existing account?"
                   << getCol(idx==0?SelectedColor:UnselectedColor) << L"\n1) Yes\n"
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

    saveData();
    
    std::wcout << getCol(RGB{0, 255, 0}) << L"\n\nAll data saved successfully\nGoodbye!" << getCol() << std::endl;
    
#ifndef _WIN32
    cleanup(0);
    std::system("reset");
#endif
    
    return 0;
}
