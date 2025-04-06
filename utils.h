#pragma once

#include <cstdint>
#include <utility>
#include <iostream>
#include <limits>
#include <sstream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using u32 = uint32_t;
using i32 = int32_t;
using u8 = uint8_t;

#ifndef _WIN32
void initTerminalStates();
void setTerminalState(const struct termios& s);
void cleanup(i32);
#endif

enum class Type {
    GeneralPractice, Cardiology, Dermatology,
    Neurology, Pediatrics, Orthopedics,
    Gynecology, InternalMedicine, Surgery,
    Patient
};

std::wstring getTypeWstr(const Type);

std::wstring stw(const std::string&);

void clearScreen();

template <typename T>
T readBF(std::ifstream&);
template <typename T>
void writeBF(std::ofstream&, T);

void writeStr(std::ofstream&, const std::string&);
std::string readStr(std::ifstream&);

void writeWstr(std::ofstream&, const std::wstring&);
std::wstring readWstr(std::ifstream&);


struct RGB {
    const u8 r,g,b;

    RGB(u8 r, u8 g, u8 b);
    RGB(u8 c=0);
};

std::wstring getCol(const RGB);
std::wstring getCol();
void flushInputBuffer();
void clearInputBuffer();
char getChar();
void getCharV();