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

static const std::string SaveFile = "data.dat";

static const u32 CurrentYear = 2025;

static const u32 MinimumUsernameLength = 5;
static const u32 MaximumUsernameLength = 20;

static const u32 MinimumPasswordLength = 7;
static const u32 MaximumPasswordLength = 50;

static const RGB ErrorColor = {255, 0, 0};
static const RGB SelectedColor = {245, 212, 66};
static const RGB UnselectedColor = {112, 109, 96};

static const std::vector<User> DefaultDoctors {
    User(L"DrSmith", "#Password123", Type::GeneralPractice),
    User(L"DrJohnson", "@securE456", Type::Cardiology),
    User(L"DrWilliams", "^Doctor789", Type::Pediatrics),
    User(L"DrBrown", ")mediC2023", Type::Neurology),
    User(L"DrDavis", "(Health2024", Type::Orthopedics)
};

static const std::vector<User> DefaultPatients {
    User(L"EmilyClark", "Se@ure123Pass"),
    User(L"JacobMiller", "P@ssw0rdSafe"),
    User(L"SophiaBrown", "H3alth#Care2023"),
    User(L"NoahWilson", "Patient$789Abcd"),
    User(L"OliviaJones", "M3dical!Records")
};

static std::vector<std::shared_ptr<Appointment>> DefaultAppointments {
    std::make_shared<Appointment>(Date(12, 1, 2025), std::make_shared<User>(DefaultDoctors[2]), std::make_shared<User>(DefaultPatients[1])),
    std::make_shared<Appointment>(Date(23, 3, 2025), std::make_shared<User>(DefaultDoctors[0]), std::make_shared<User>(DefaultPatients[3])),
    std::make_shared<Appointment>(Date(15, 7, 2025), std::make_shared<User>(DefaultDoctors[4]), std::make_shared<User>(DefaultPatients[0])),
    std::make_shared<Appointment>(Date(8, 12, 2025), std::make_shared<User>(DefaultDoctors[1]), std::make_shared<User>(DefaultPatients[4])),
    std::make_shared<Appointment>(Date(19, 2, 2025), std::make_shared<User>(DefaultDoctors[3]), std::make_shared<User>(DefaultPatients[2]))
};

std::shared_ptr<User> CurrentUser;
std::vector<std::shared_ptr<Appointment>> CurrentAppointments;

std::vector<User> doctors, patients;
std::vector<std::shared_ptr<Appointment>> appointments;

void fetchAppointments(const bool isDoctor) {
    CurrentAppointments.reserve(appointments.size());

	for (std::shared_ptr<Appointment> appointment : appointments) {
        if ((isDoctor ? appointment->doctor->name : appointment->patient->name) != CurrentUser->name) continue;

        CurrentAppointments.push_back(appointment);
	}
}

void initializeData() {
    patients = DefaultPatients;
    doctors = DefaultDoctors;
	appointments = DefaultAppointments;
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
    const u8 day = readBF<u8>(is);
    const u8 month = readBF<u8>(is);
    const u32 year = readBF<u32>(is);

    return Date (day, month, year);
}

User loadDoctor(std::ifstream& is) {
    const std::wstring name = readWstr(is);
    const std::string password = readStr(is);
    const Type type = readBF<Type>(is);
    
    return User (name, password, type);
}

User loadPatient(std::ifstream& is) {
    const std::wstring name = readWstr(is);
    const std::string password = readStr(is);

    return User (name, password);
}

void loadAppointments(std::ifstream& is) {
    const u32 sz = readBF<u32>(is);
    appointments.reserve(sz);

    for(u32 i=0; i < sz; ++i) {
        const Date date = loadDate(is);
		const std::shared_ptr<User> doctor = std::make_shared<User>(loadDoctor(is));
		const std::shared_ptr<User> patient = std::make_shared<User>(loadPatient(is));
        
		appointments.push_back(std::make_shared<Appointment>(date, doctor, patient));
    }
}

void saveData() {
    std::ofstream os (SaveFile, std::ios::binary);

	writeBF<u32>(os, appointments.size());
	for (std::shared_ptr<Appointment> appointment : appointments)
		saveDate(os, appointment->date),
		saveDoctor(os, *appointment->doctor),
		savePatient(os, *appointment->  patient);

    writeBF<u32>(os, doctors.size());
    for(const User& doctor : doctors) saveDoctor(os, doctor);

    writeBF<u32>(os, patients.size());
    for(const User& patient : patients) savePatient(os, patient);

    os.close();
}

void loadData() {
    std::ifstream is (SaveFile, std::ios::binary);
	loadAppointments(is);

    u32 sz = readBF<u32>(is);
    doctors.reserve(sz);
    
    for(u32 i=0; i < sz; ++i) doctors.push_back(loadDoctor(is));
    
    sz = readBF<u32>(is);
    patients.reserve(sz);
    
    for(u32 i=0; i < sz; ++i) patients.emplace_back(loadPatient(is));
    
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
            } break;
        }
    }
}

std::shared_ptr<User> pickUser(const bool isDoctor) {
    u8 idx = 0;
    
    while(true) {
        clearScreen();
        
        const u32 sz = !isDoctor ? doctors.size() : patients.size();
        
        for(u32 i=0; i < sz; ++i) {
            const User user = !isDoctor ? doctors[i] : patients[i];
            
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
            return std::make_shared<User>(!isDoctor ? doctors[idx] : patients[idx]);
            break;
        }
    }
}

void createAppointment() {
    Date date(0, 0, CurrentYear);
    modifyDate(date);

    std::shared_ptr<Appointment> appointment = std::make_shared<Appointment>(date, pickUser(false), CurrentUser);

    appointments.push_back(appointment);
    CurrentAppointments.push_back(appointment);
}

void deleteAppointment(const u8 idx) {
    CurrentAppointments.erase(CurrentAppointments.begin() + idx);
}

void mainServiceMenu(const bool isDoctor) {
    u8 idx = 0;
    
    while(true) {
        clearScreen();
        const u32 sz = CurrentAppointments.size();

        std::wcout << (isDoctor ? L"Doctor" : L"Patient") << " Actions\n\n";

        if(sz == 0) {
            std::wcout << getCol(SelectedColor) << "No appointments made yet, " << (isDoctor ? L"" : L"press n to make one or ") << L"press q to quit" << L'\n' << getCol();
            const char c = getChar();

            if(!isDoctor && c == 'n') createAppointment();
            if(c == 'q') return;

            continue;
        }

        for(u32 i=0; i < sz; ++i) {
            std::shared_ptr<Appointment> appointment = CurrentAppointments[i];

            std::wcout << getCol(idx == i ? SelectedColor : UnselectedColor)
                       << i+1 << L") " << (isDoctor ? L"Patient:" : L"Doctor: ") << (isDoctor ? appointment->patient->name : appointment->doctor->name)
                       << L"\nDate: " << appointment->date.str() 
                       << (!isDoctor ? L"\nSpecialization: " + getTypeWstr(appointment->doctor->type) : L"")
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
            saveData();
            break;

            case 'b':
            modifyDate(CurrentAppointments[idx]->date);
            saveData();
            break;

            case 'v':
            if (std::shared_ptr<User> user = pickUser(isDoctor)) {
                if(isDoctor) CurrentAppointments[idx]->patient = user;
                else CurrentAppointments[idx]->doctor = user;
                
                saveData();
            }
            break;

            case 'g': saveData(); break;
    
            case 'q': return;

            case 'y':
            deleteAppointment(idx);
            if(idx == sz-1) --idx;
            break;
            
            default: break;
        }
    }
}

const std::shared_ptr<User> isValidName(const std::wstring& name) {
    const auto doctor = std::find_if(doctors.begin(), doctors.end(), [&name](const User& doctor) { 
        return doctor.name == name; 
    });
    
    const auto patient = std::find_if(patients.begin(), patients.end(), [&name](const User& patient) { 
        return patient.name == name;
    });

    return doctor == doctors.end() ? 
        (patient == patients.end() ? nullptr
        : std::make_shared<User>(*patient))
        : std::make_shared<User>(*doctor);
}

bool showPasswordError(const bool condition, const std::wstring& errorMessage) {
    if(!condition) return false;

    std::wcout << getCol(ErrorColor) << L'\n' << errorMessage << getCol();

    getCharV();
    return true;
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
        } else if(!hasAccount && std::find_if(patients.begin(), patients.end(), [&name](const User& patient) { 
            return patient.name == name; 
        }) != patients.end()) {
            std::wcout << getCol(ErrorColor) << L"\nUsername already exists\n" << getCol();
            getCharV();
        } else if(hasAccount) {
            CurrentUser = isValidName(name);

            if(CurrentUser == nullptr) {
                std::wcout << getCol(ErrorColor) << L"\nThere is nobody with that username" << getCol();
                getCharV();
            } else break;
        } else break;
    }

    std::string password;

    while(true) {
        clearScreen();
        std::wcout << (hasAccount ? L"Log In" : L"Register") << L"\nUsername: " << name
                   << (hasAccount ? CurrentUser->type==Type::Patient ? L"\nPatient" : L"\nDoctor" : L"")      
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
            continue;
        } else if(hasAccount && password != CurrentUser->password) {
            std::wcout << getCol(ErrorColor) << L"\nInvalid password for user " << CurrentUser->name << getCol();

            getCharV();
            continue;
        } 
        
        bool hasLower = false, hasUpper = false, hasDigit = false, hasSymbol = false;

        for(const char c : password)
            if(!hasLower && std::islower(c)) hasLower = true;
            else if(!hasUpper && std::isupper(c)) hasUpper = true;
            else if(!hasDigit && std::isdigit(c)) hasDigit = true;
            else if(!hasSymbol && !isalnum(c) && !std::isspace(c)) hasSymbol = true;

        if(showPasswordError(!hasLower, L"Password must contain at least one lowercase letter") ||
           showPasswordError(!hasUpper, L"Password must contain at least one uppercase letter") ||
           showPasswordError(!hasDigit, L"Password must contain at least one digit") ||
           showPasswordError(!hasSymbol, L"Password must contain at least one symbol"))
           continue;

        break;
    }

    if(!hasAccount) {
        patients.emplace_back(name, password);
        CurrentUser = std::make_shared<User>(patients.back());
		fetchAppointments(false);
	} else fetchAppointments(CurrentUser->type != Type::Patient);

    mainServiceMenu(CurrentUser->type != Type::Patient);
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
    else initializeData(), saveData(), getChar();

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
