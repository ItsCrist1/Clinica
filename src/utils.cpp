#include "utils.h"
#include <iostream>

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

RGB::RGB(u8 r, u8 g, u8 b) : r(r), g(g), b(b) {}
RGB::RGB(u8 c) : r(c), g(c), b(c) {}

std::wstring getCol(const RGB rgb) {
    std::wstringstream wss;
    wss << L"\033[38;2;" << static_cast<int>(rgb.r) << L';' << static_cast<int>(rgb.g) << L';' << static_cast<int>(rgb.b) << L'm';
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