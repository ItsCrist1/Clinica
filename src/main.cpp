#include "clinic.h"
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

static const std::string SaveFile = "data.dat";

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

	Clinic clinic (SaveFile);

	clinic.MainMenu();
    
    std::wcout << getCol(RGB{0,255,0}) << L"\n\nAll data saved successfully\nGoodbye!" << getCol() << std::endl;
    
#ifndef _WIN32
    cleanup(0);
    std::system("reset");
#endif
    
    return 0;
}
