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

struct asset {
	int width;
	int height;
	wstring shape;
};

// Environment variables
int nFirstBlockX = 4;
int nFirstBlockY = 50;
int nNumberOfBlocks = 7;
int nBlockSpacing = 4;

// Player variables
int nPlayerX = nScreenWidth / 2;
int nPlayerY = 73;
int nPlayerVel = 1;

int nFireRate = 10;
int iterations = 0;
int nBulletSpeed = 1;
vector<COORD> vcPlayerBullets;

int main() {
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Set the size of the screen to the acceptable parameters to prevent strange glitches
	SMALL_RECT srWindowInfo = { 0, 0, (short)nScreenWidth - 1, (short)nScreenHeight - 1 };
	SetConsoleWindowInfo(hConsole, TRUE, &srWindowInfo);
	SetConsoleScreenBufferSize(hConsole, { (short)nScreenWidth, (short)nScreenHeight });

	CONSOLE_FONT_INFOEX cfiConsoleFont;
	cfiConsoleFont.cbSize = sizeof(cfiConsoleFont);
	cfiConsoleFont.dwFontSize = { 7, 7 };
	cfiConsoleFont.nFont = 0;
	cfiConsoleFont.FontFamily = FF_DONTCARE;
	cfiConsoleFont.FontWeight = 400;
	wcscpy_s(cfiConsoleFont.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(hConsole, FALSE, &cfiConsoleFont);

	/* Assets -----------------------------------------------------------------------------------*/

	// Player asset
	asset playerAsset;
	playerAsset.width = 3;
	playerAsset.height = 2;
	playerAsset.shape += L".#.";
	playerAsset.shape += L"###";

	// Block asset
	asset blockAsset;
	blockAsset.width = 14;
	blockAsset.height = 14;
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";
	blockAsset.shape = L"##############";

	while (1) {
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
			screen[i] = ' ';
		this_thread::sleep_for(25ms);

		/* Player Input -------------------------------------------------------------------------*/

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			nPlayerX -= nPlayerVel;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			nPlayerX += nPlayerVel;
		if (GetAsyncKeyState((unsigned short)' ') & 0x8000) {
			if(iterations % nFireRate == 0)
				vcPlayerBullets.push_back({ (short)nPlayerX + 1, (short)nPlayerY });
		}

		/* Handle bullets -----------------------------------------------------------------------*/

		for (int i = 0; i < (signed)vcPlayerBullets.size();  i++) {
			if (vcPlayerBullets[i].Y <= 0) {
				vcPlayerBullets.erase(vcPlayerBullets.begin() + i);
				continue;
			}

			vcPlayerBullets.at(i).Y -= nBulletSpeed;
		}

		/* Handle bullet block collisions -------------------------------------------------------*/

		// Player bullet


		/* Display ------------------------------------------------------------------------------*/

		// Display bullets
		for (int i = 0; i < (signed)vcPlayerBullets.size(); i++) {
			screen[vcPlayerBullets[i].Y * nScreenWidth + vcPlayerBullets[i].X] = L'|';
		}

		// Display blocks
		for (int i = nFirstBlockX; i < (blockAsset.width + nBlockSpacing) * nNumberOfBlocks; i += nBlockSpacing + blockAsset.width) {
			for (int x = i; x < i + blockAsset.width; x++) {
				for (int y = nFirstBlockY; y < nFirstBlockY + blockAsset.height; y++) {
					screen[y * nScreenWidth + x] = L'\u2588';
				}
			}
		}

		// Diplay player
		for (int x = nPlayerX; x < nPlayerX + playerAsset.width; x++) {
			for (int y = nPlayerY; y < nPlayerY + playerAsset.height; y++) {
				switch (playerAsset.shape[(y - nPlayerY) * playerAsset.width + (x - nPlayerX)]) {
					case '#':
						screen[y * nScreenWidth + x] = L'\u2588';
						break;
					case ' ':
						screen[y * nScreenWidth + x] = L' ';
						break;
				}
			}
		}

		screen[nScreenWidth * nScreenHeight] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

		iterations++;
	}

	return 0;
}