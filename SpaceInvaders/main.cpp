/*
* Created 1/9/2021 at 6:03 PM
*/

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <stdlib.h>
using namespace std;

int nScreenWidth = 230;
int nScreenHeight = 200;

struct asset {
	int width;
	int height;
	wstring shape;
};

// This function isn't used for blocks due to the process being more unique
void displayAsset(int, int, asset&, wchar_t*);

// Environment variables
int nFirstBlockX = 8;
int nFirstBlockY = nScreenHeight - 40;
int nBlockSize = 20;
const int nNumberOfBlocks = 8;
int nBlockSpacing = 8;
wstring blocks[nNumberOfBlocks];

// Player variables
int nPlayerX = nScreenWidth / 2;
int nPlayerY = nScreenHeight - 7;
int nPlayerVel = 1;

int nFireRate = 10;
int iterations = 0;
int nBulletSpeed = 2;
vector<COORD> vcPlayerBullets;

// Alien variables
int nColumnDistFromEdge = 15;
int nVerticalSpacing = 5;
int nHorizontalSpacing = 8;
int nNumberOfColumns = (nScreenWidth - nColumnDistFromEdge / 2) / (12 + nHorizontalSpacing); // The twelve is the largest alien asset width
vector<vector<COORD>> vColumns;

int nFirstAlienX = nColumnDistFromEdge;
int nFirstAlienY = 20;

int nMovementTickRate = 10;
int nAlienVel;

int nTorpedoVel = 1;
int nAlienFireRate = 20;
vector<COORD> vcAlienTorpedoes;


int clamp(int, int, int);

int main() {
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Set the size of the screen to the acceptable parameters to prevent strange glitches (doesn't work right now)
	SMALL_RECT srWindowInfo = { 0, 0, (short)nScreenWidth - 1, (short)nScreenHeight - 1 };
	SetConsoleWindowInfo(hConsole, TRUE, &srWindowInfo);
	SetConsoleScreenBufferSize(hConsole, { (short)nScreenWidth, (short)nScreenHeight });

	CONSOLE_FONT_INFOEX cfiConsoleFont;
	cfiConsoleFont.cbSize = sizeof(cfiConsoleFont);
	cfiConsoleFont.dwFontSize = { 3, 3 };
	cfiConsoleFont.nFont = 0;
	cfiConsoleFont.FontFamily = FF_DONTCARE;
	cfiConsoleFont.FontWeight = 400;
	wcscpy_s(cfiConsoleFont.FaceName, L"Consolas");
	SetCurrentConsoleFontEx(hConsole, FALSE, &cfiConsoleFont);

	/* Assets -----------------------------------------------------------------------------------*/

	// Player asset
	asset playerAsset;
	playerAsset.width = 5;
	playerAsset.height = 4;
	playerAsset.shape += L"..#..";
	playerAsset.shape += L".###.";
	playerAsset.shape += L"#####";
	playerAsset.shape += L"#####";

	// Bullet Asset
	asset bulletAsset;
	bulletAsset.width = 1;
	bulletAsset.height = 2;
	bulletAsset.shape += L"#";
	bulletAsset.shape += L"#";

	// First alien asset
	asset alien1Asset;
	alien1Asset.width = 8;
	alien1Asset.height = 8;
	alien1Asset.shape += L"...##...";
	alien1Asset.shape += L"..####..";
	alien1Asset.shape += L".######.";
	alien1Asset.shape += L"##.##.##";
	alien1Asset.shape += L"########";
	alien1Asset.shape += L"..#..#..";
	alien1Asset.shape += L".#.##.#.";
	alien1Asset.shape += L"#.#..#.#";

	// Second alien asset
	asset alien2Asset;
	alien2Asset.width = 12;
	alien2Asset.height = 8;
	alien2Asset.shape += L"..#.....#...";
	alien2Asset.shape += L"...#...#....";
	alien2Asset.shape += L"..#######...";
	alien2Asset.shape += L".##.###.##..";
	alien2Asset.shape += L"###########.";
	alien2Asset.shape += L"#.#######.#.";
	alien2Asset.shape += L"#.#.....#.#.";
	alien2Asset.shape += L"...##.##....";

	// Third alien asset
	asset alien3Asset;
	alien3Asset.width = 12;
	alien3Asset.height = 8;
	alien3Asset.shape += L"....####....";
	alien3Asset.shape += L".##########.";
	alien3Asset.shape += L"############";
	alien3Asset.shape += L"###..##..###";
	alien3Asset.shape += L"############";
	alien3Asset.shape += L"...##..##...";
	alien3Asset.shape += L"..##.##.##..";
	alien3Asset.shape += L"##........##";
	
	asset torpedoAsset;
	torpedoAsset.width = 3;
	torpedoAsset.height = 4;
	torpedoAsset.shape += L"###";
	torpedoAsset.shape += L".#.";
	torpedoAsset.shape += L".#.";
	torpedoAsset.shape += L".#.";

	// Block asset (the width and height parameters are stored in the "nBlockSize" variable
	asset blockAsset;
	for (int x = 0; x < nBlockSize; x++) {
		for (int y = 0; y < nBlockSize; y++) {
			blockAsset.shape += L'#';
		}
	}

	// Copy the block asset so we can edit it
	for (int i = 0; i < nNumberOfBlocks; i++) 
		blocks[i] = blockAsset.shape;



	// Initialize the vColumns vector with vectors which represent the indiviual columns
	for (int i = 1; i <= nNumberOfColumns; i++) {
		vector<COORD> column;
		for (int y = nFirstAlienY; y < nFirstAlienY + ((12 + nVerticalSpacing) * 6); y += nVerticalSpacing + 12) {
			column.push_back({ short(nFirstAlienX * i), (short)y });
		}
		vColumns.push_back(column);
	}

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
				vcPlayerBullets.push_back({ (short)nPlayerX + (short)playerAsset.width / 2, (short)nPlayerY });
		}

		nPlayerX = clamp(nPlayerX, 0, nScreenWidth - playerAsset.width);

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
						if (!(vcPlayerBullets[i].X == x && (vcPlayerBullets[i].Y <= y && vcPlayerBullets[i].Y + 3 >= y))) {
							continue;
						} else {
							switch (blocks[currentBlock][(y - nFirstBlockY) * nBlockSize + (x - l)]) {
								case '.':
									break;
								case '#':
									for (int inc = -2; inc < 3; inc++) {
										if ((x - l) + inc >= 0 && (x - l) + inc < nBlockSize)
											blocks[currentBlock][(y - nFirstBlockY) * nBlockSize + (x - l + inc)] = L'.';

										if ((y - nFirstBlockY) + inc >= 0 && (y - nFirstBlockY) + inc < nBlockSize)
											blocks[currentBlock][(y - nFirstBlockY + inc) * nBlockSize + (x - l)] = L'.';

										if (inc > -2 && inc < 2) {
											if ((x - l) + inc >= 0 && (x - l) + inc < nBlockSize &&
												(y - nFirstBlockY) + inc >= 0 && (y - nFirstBlockY) + inc < nBlockSize) {
												blocks[currentBlock][(y - nFirstBlockY + inc) * nBlockSize + (x - l + inc)] = L'.';

												// Stop the bullet from destroying the other side of the block
												int incControl = 1;
												if (x - l - inc >= nBlockSize || x - l - inc <= 0)
													incControl = 0;
												blocks[currentBlock][(y - nFirstBlockY + inc) * nBlockSize + (x - l - (inc * incControl))] = L'.';
											}
										}
									}

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
						case '.':
							screen[y * nScreenWidth + x] = L' ';
							break;
					}
				}
			}
		}

		// Display bullets
		for (int i = 0; i < (signed)vcPlayerBullets.size(); i++) {
			displayAsset(vcPlayerBullets[i].X, vcPlayerBullets[i].Y, bulletAsset, screen);
		}

		for (int i = 0; i < (signed)vColumns.size(); i++) {
			for (int l = 0; l < (signed)vColumns[i].size(); l++) {
				COORD currentAlien = vColumns[i][l];
				displayAsset(currentAlien.X, currentAlien.Y, alien1Asset, screen);
			}
		}

		// Diplay player
		displayAsset(nPlayerX, nPlayerY, playerAsset, screen);

		screen[nScreenWidth * nScreenHeight] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);

		iterations++;
	}

	return 0;
}

void displayAsset(int assetX, int assetY, asset& assetToDraw, wchar_t* screen) {
	for (int x = assetX; x < assetX + assetToDraw.width; x++) {
		for (int y = assetY; y < assetY + assetToDraw.height; y++) {
			switch (assetToDraw.shape[(y - assetY) * assetToDraw.width + (x - assetX)]) {
				case '#':
					screen[y * nScreenWidth + x] = L'\u2588';
					break;
				case '.':
					screen[y * nScreenWidth + x] = L' ';
					break;
			}
		}
	}
}

int clamp(int x, int lo, int hi) {
	if (x > hi)
		return hi;
	else if (x < lo)
		return lo;

	return x;
}

