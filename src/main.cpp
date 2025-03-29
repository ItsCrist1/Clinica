#include "data.h"
#include <cstdint>
#include <vector>
#include <locale>
#include <memory>
#include <algorithm>

using u32 = uint32_t;
using i32 = int32_t;

static const size_t MinimumUsernameLength = 5;
static const size_t MaximumUsernameLength = 15;

static const size_t MinimumPasswordLength = 7;
static const size_t MaximumPasswordLength = 50;

static const RGB ErrorColor = RGB(255, 0, 0);
static const RGB SelectedColor = RGB(245,212,66);
static const RGB UnselectedColor = RGB(112,109,96);

static const std::vector<User> doctors {
    User("DrSmith", "#Password123", Type::GeneralPractice),
    User("DrJohnson", "@securE456", Type::Cardiology),
    User("DrWilliams", "^Doctor789", Type::Pediatrics),
    User("DrBrown", ")mediC2023", Type::Neurology),
    User("DrDavis", "(Health2024", Type::Orthopedics)
};

std::vector<User> patients {
    User("EmilyClark", "Se@ure123Pass", Type::Patient),
    User("JacobMiller", "P@ssw0rdSafe", Type::Patient),
    User("SophiaBrown", "H3alth#Care2023", Type::Patient),
    User("NoahWilson", "Patient$789Abcd", Type::Patient),
    User("OliviaJones", "M3dical!Records", Type::Patient)
};

std::shared_ptr<User> user;

std::shared_ptr<User> isValidName(const std::string& name) {
    const auto doctor = std::find_if(doctors.begin(), doctors.end(), [&name](const User& user) { 
        return user.name == name; 
    });
    
    const auto patient = std::find_if(patients.begin(), patients.end(), [&name](const User& user) { 
        return user.name == name; 
    });

    return doctor == doctors.end() ? (patient == patients.end() ? nullptr : std::make_shared<User>(*patient)) : std::make_shared<User>(*doctor);
}

void execPatientMenu(const bool hasAccount) {
    std::string name;
    
    while(true) {
        clearScreen();
        std::wcout << L"Enter a username (" 
                   << MinimumUsernameLength << L'-' << MaximumUsernameLength 
                   << L" characters): ";
                  
        std::cin >> name;
        clearInputBuffer();

        if(const size_t sz=name.length(); !(sz >= MinimumUsernameLength && sz <= MaximumUsernameLength)) {
            std::wcout << getCol(ErrorColor) << L"\nInvalid username length " << getCol() << L"(Must be between "
                       << MinimumUsernameLength << L'-' << MaximumUsernameLength 
                       << L" characters)" << getCol();
            getCharV();
        } else if(!hasAccount && std::find_if(patients.begin(), patients.end(), [&name](const User& user) { 
            return user.name == name; 
        }) != patients.end()) {
            std::wcout << getCol(ErrorColor) << L"\nUsername already exists\n" << getCol();
            getCharV();
        } if(hasAccount) {
            user = isValidName(name);

            if(user == nullptr) {
                std::wcout << getCol(ErrorColor) << L"\nThere is nobody with that username" << getCol();
                getCharV();
            } else break;
        } else break;
    }

    std::string password;

    while(true) {
        clearScreen();
        std::wcout << L"Enter a password (" 
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
            std::wcout << getCol(ErrorColor) << L"\nInvalid password for user " << stw(user->name) << getCol();
            getCharV();
        } else break;
    }

    if(!hasAccount) {
        patients.emplace_back(name, password);
        user = std::make_shared<User>(patients[patients.size()-1]);
    }

    
}

void mainPatientMenu() {
    u8 idx = 0;
    
    while(true) {
        clearScreen();

        std::wcout << L"Do you have an existing account?";
        std::wcout << getCol(idx==0?SelectedColor:UnselectedColor) << L"\n1) Yes\n"
                   << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) No"
                   << getCol();

        const char c = getChar();

        if(std::isdigit(c)) {
            const u8 digit = c - '0';

            if(digit < 1 || digit > 2) {
                clearScreen();
                std::wcout << getCol(ErrorColor) << L"Error: Digit input must be between 1-2\n" << getCol();
                getCharV();
            } else execPatientMenu(digit == 1);

            return;
        } 
    
        switch(c) {
            case 'w': case 's': 
            case 'a': case 'd': 
            idx = idx == 0 ? 1 : 0; break;
    
            case 'q': return;
            
            default: execPatientMenu(idx == 0); return;
        }
    }
}

i32 main() {
    #ifndef _WIN32
    initTerminalStates();
    std::locale::global (std::locale(""));
	#endif

    mainPatientMenu();

    std::wcout << getCol(RGB(0, 255, 0)) << L"\n\nGoodbye!" << getCol() << std::endl;
    return 0;
}