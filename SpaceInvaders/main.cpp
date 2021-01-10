/*
* Created 1/9/2021 at 6:03 PM
*/

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <stdlib.h>
using namespace std;

int nScreenWidth = 130;
int nScreenHeight = 80;

int main() {
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Set the size of the screen to the acceptable parameters to prevent strange glitches
	SMALL_RECT srWindowInfo = { 0, 0, nScreenWidth - 1, nScreenHeight - 1 };
	SetConsoleWindowInfo(hConsole, TRUE, &srWindowInfo);
	SetConsoleScreenBufferSize(hConsole, { (short)nScreenWidth, (short)nScreenHeight });

	CONSOLE_FONT_INFOEX cfiConsoleFont;
	cfiConsoleFont.cbSize = sizeof(cfiConsoleFont);
	cfiConsoleFont.dwFontSize = { 2, 2 };
	cfiConsoleFont.nFont = 0;
	cfiConsoleFont.FontFamily = FF_DONTCARE;
	cfiConsoleFont.FontWeight = 400;
	wcscpy_s(cfiConsoleFont.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(hConsole, FALSE, &cfiConsoleFont);

	while (1) {
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
			screen[i] = ' ';

		screen[nScreenWidth * nScreenHeight];
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}