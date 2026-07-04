#include "console.h"

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>

namespace {
HANDLE stdout_handle = INVALID_HANDLE_VALUE;
DWORD initial_mode = 0;
bool mode_changed = false;
}
#endif

void Console::setup()
{
#ifdef _WIN32
    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    // Redirected output is not a console and needs no mode change.
    if (stdout_handle == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(stdout_handle, &mode)) {
        return;
    }

    initial_mode = mode;
    mode_changed = SetConsoleMode(
        stdout_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#endif
}

void Console::restore()
{
    std::fputs("\x1b[0m", stdout);

#ifdef _WIN32
    if (mode_changed) {
        SetConsoleMode(stdout_handle, initial_mode);
    }
#endif
}
