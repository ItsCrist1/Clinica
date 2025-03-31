#include "utils.h"
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#undef RGB
#else
#include <termio.h>
#include <csignal>
#include <unistd.h>

struct termios oldt, newt;

void initTerminalStates() {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    std::signal(SIGINT, cleanup);
    std::signal(SIGTERM, cleanup);
    std::signal(SIGSEGV, cleanup);
}

void setTerminalState(const struct termios& s) {
    tcsetattr(STDIN_FILENO, TCSANOW, &s);
}

void cleanup(i32 sig) {
    setTerminalState(oldt);
    std::exit(sig);
}

#endif

std::wstring getTypeWstr(const Type type) {
    switch (type) {
        case Type::GeneralPractice: return L"GeneralPractice";
        case Type::Cardiology: return L"Cardiology";
        case Type::Dermatology: return L"Dermatology";
        case Type::Neurology: return L"Neurology";
        case Type::Pediatrics: return L"Pediatrics";
        case Type::Orthopedics: return L"Orthopedics";
        case Type::Gynecology: return L"Gynecology";
        case Type::InternalMedicine: return L"InternalMedicine";
        case Type::Surgery: return L"Surgery";
        case Type::Patient: return L"Patient";
        
        default: return L"Unknown";
    }
}

std::wstring stw(const std::string& s) {
    return {s.begin(), s.end()};
}

void clearScreen() {
    #ifdef _WIN32
    std::system("cls");
    #else
    std::wcout << L"\033[2J\033[H";
    #endif
}

template u8 readBF<u8>(std::ifstream& is);
template wchar_t readBF<wchar_t>(std::ifstream& is);
template u32 readBF<u32>(std::ifstream& is);
template Type readBF<Type>(std::ifstream& is);

template <typename T>
T readBF(std::ifstream& is) {
    T n;
    is.read(reinterpret_cast<char*>(&n), sizeof(T));
    return n;
}

template void writeBF<u8>(std::ofstream& is, u8 n);
template void writeBF<wchar_t>(std::ofstream& is, wchar_t n);
template void writeBF<u32>(std::ofstream& is, u32 n);
template void writeBF<Type>(std::ofstream& is, Type n);

template <typename T>
void writeBF(std::ofstream& os, T n) {
    os.write(reinterpret_cast<char*>(&n), sizeof(T));
}

void writeStr(std::ofstream& os, const std::string& str) {
    u32 size = str.size();
    
    std::ofstream ts ("test.txt", std::ios::app);
    ts << size << '\n';
    ts.close();

    writeBF<u32>(os, size);
    for(const char c : str) writeBF<u8>(os, c);
}

std::string readStr(std::ifstream& is) {
    u32 size = readBF<u32>(is);

    std::ofstream ts ("test_2.txt", std::ios::app);
    ts << size << '\n';
    ts.close();

    std::string str (size, ' ');
    for(char& c : str) c = readBF<u8>(is);
    return str;
}

void writeWstr(std::ofstream& os, const std::wstring& wstr) {
    u32 size = wstr.size();
    
    writeBF<u32>(os, size);
    for(const wchar_t wc : wstr) writeBF<wchar_t>(os, wc);
}

std::wstring readWstr(std::ifstream& is) {
    u32 size = readBF<u32>(is);

    std::wstring wstr(size, L' ');
    for(wchar_t& wc : wstr) wc = readBF<wchar_t>(is);
    return wstr;
}

RGB::RGB(u8 r, u8 g, u8 b) : r(r), g(g), b(b) {}
RGB::RGB(u8 c) : r(c), g(c), b(c) {}

std::wstring getCol(const RGB rgb) {
    std::wstringstream wss;
    wss << L"\033[38;2;" << static_cast<u32>(rgb.r) << L';' << static_cast<u32>(rgb.g) << L';' << static_cast<u32>(rgb.b) << L'm';
    return wss.str();
}

std::wstring getCol() {
    return L"\033[0m";
}

void flushInputBuffer() {
    #ifdef _WIN32
    while(_kbhit()) _getch(); 
    #else
    tcflush(STDIN_FILENO, TCIFLUSH);
    #endif
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

char getChar() {
    #ifdef _WIN32
    int c = _getch();
    if (c == 224 || c == 0) {
        switch(_getch()) {
            case 72: return 'w';
            case 80: return 's';
            case 75: return 'a';
            case 77: return 'd';
            
            default: return 0;
        }
    } return c == 13 ? ' ' : (char)c;
    #else
    setTerminalState(newt);
    char c = getchar();
    setTerminalState(oldt);
    if(c == '\033') {
        getchar();
        switch(getchar()) {
            case 'A': return 'w';
            case 'B': return 's';
            case 'C': return 'd';
            case 'D': return 'a';
        }
    } return c == '\n' ? ' ' : c;
    #endif
}

void getCharV() {
    std::wcout << L"\nPress any key to continue...";
    getChar();
}