#ifndef WIN_CONSOLEUTIL_H
#define WIN_CONSOLEUTIL_H

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <windows.h>

bool setupWindowsConsole();

void reopenWindowsStreams();

int getStdFD(const DWORD& nStdHandle);

#endif
