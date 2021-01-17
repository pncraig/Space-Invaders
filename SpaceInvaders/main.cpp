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
int nBlockSize = 14;
const int nNumberOfBlocks = 7;
int nBlockSpacing = 4;
wstring blocks[nNumberOfBlocks];

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
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";
	blockAsset.shape += L"##############";

	for (int i = 0; i < nNumberOfBlocks; i++) 
		blocks[i] = blockAsset.shape;

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
		for (int i = 0; i < (signed)vcPlayerBullets.size(); i++) {
			// Skip bullets that can't hit the blocks
			if (vcPlayerBullets[i].Y < nFirstBlockY)
				continue;

			// Start with 0 because we increment the variable at the start of the loop
			int currentBlock = -1;
			bool collided = false;
			for (int l = nFirstBlockX; l < (nBlockSize + nBlockSpacing) * nNumberOfBlocks; l += nBlockSpacing + nBlockSize) {
				currentBlock++;
				for (int x = l; x < l + nBlockSize; x++) {
					for (int y = nFirstBlockY; y < nFirstBlockY + nBlockSize; y++) {
						// Skip if the bullet isn't colliding with the current tile
						if (!(vcPlayerBullets[i].X == x && vcPlayerBullets[i].Y == y)) {
							continue;
						} else {
							switch (blocks[currentBlock][(y - nFirstBlockY) * nBlockSize + (x - l)]) {
								case ' ':
									break;
								case '#':
									blocks[currentBlock][(y - nFirstBlockY) * nBlockSize + (x - l)] = L' ';
									vcPlayerBullets.erase(vcPlayerBullets.begin() + i);
									// We want to stop running checks with this specific vector element
									collided = true;
									break;
							}
						}
						// Break all the loops except the outermost loop
						if (collided)
							break;
					}
					if (collided)
						break;
				}
				if (collided)
					break;
			}
		}


		/* Display ------------------------------------------------------------------------------*/

		// Display blocks
		int currentBlock = -1;
		for (int i = nFirstBlockX; i < (nBlockSize + nBlockSpacing) * nNumberOfBlocks; i += nBlockSpacing + nBlockSize) {
			currentBlock++;
			for (int x = i; x < i + nBlockSize; x++) {
				for (int y = nFirstBlockY; y < nFirstBlockY + nBlockSize; y++) {
					wstring thisBlock = blocks[currentBlock];
					switch (thisBlock[(y - nFirstBlockY) * nBlockSize + (x - i)]) {
						case '#':
							screen[y * nScreenWidth + x] = L'\u2588';
							break;
						case ' ':
							screen[y * nScreenWidth + x] = L' ';
							break;
					}
				}
			}
		}

		// Display bullets
		for (int i = 0; i < (signed)vcPlayerBullets.size(); i++) {
			screen[vcPlayerBullets[i].Y * nScreenWidth + vcPlayerBullets[i].X] = L'|';
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

		wprintf_s(screen, 196, L'%s', blocks[0]);

		screen[nScreenWidth * nScreenHeight] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

		iterations++;
	}

	return 0;
}