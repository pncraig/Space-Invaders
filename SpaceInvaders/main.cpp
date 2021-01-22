/*
* Created 1/9/2021 at 6:03 PM
*/

#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
#include <stdlib.h>
#include <ctime>
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
int nFirstBlockY = nScreenHeight - 50;
int nBlockSize = 20;
const int nNumberOfBlocks = 8;
int nBlockSpacing = 8;
wstring blocks[nNumberOfBlocks];

int nBlastRadius = 4;

// Player variables
int nPlayerX = nScreenWidth / 2;
int nPlayerY = nScreenHeight - 14;
int nPlayerVel = 1;
int nPlayerLives = 3;

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

int nAlienTickRate = 5;
int nAlienVel = 2;

int nTorpedoVel = 1;
int nAlienFireRate = 20;
vector<COORD> vAlienTorpedoes;

void collideProjectileWithBlock(vector<COORD>& projectileVector, int vectorIndex, int blastRadius);

int clamp(int, int, int);

int main() {
	srand((int)time(0));

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
	alien2Asset.width = 11;
	alien2Asset.height = 8;
	alien2Asset.shape += L"..#.....#..";
	alien2Asset.shape += L"...#...#...";
	alien2Asset.shape += L"..#######..";
	alien2Asset.shape += L".##.###.##.";
	alien2Asset.shape += L"###########";
	alien2Asset.shape += L"#.#######.#";
	alien2Asset.shape += L"#.#.....#.#";
	alien2Asset.shape += L"...##.##...";

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
	
	// Torpedo asset
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
		int currentAlien = 0;
		// Loop through every ROW (the 6 is the number of rows) and figure out what the y is
		for (int y = nFirstAlienY; y < nFirstAlienY + ((12 + nVerticalSpacing) * 6); y += nVerticalSpacing + 12) {
			// Keep the alien with a smaller width value in line with the other aliens
			int margin = 0;
			if (currentAlien % 3 == 2)
				margin = 2;
			column.push_back({ short(nFirstAlienX * i + margin), (short)y });

			currentAlien++;
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

		/* Handle alien movement ----------------------------------------------------------------*/

		if (iterations % nAlienTickRate == 0) {
			if (vColumns.back().back().X + alien3Asset.width > nScreenWidth - nColumnDistFromEdge || vColumns.front().back().X < nColumnDistFromEdge)
				nAlienVel *= -1;

			for (int i = 0; i < (signed)vColumns.size(); i++) {
				for (int l = 0; l < (signed)vColumns[i].size(); l++) {
					vColumns[i][l].X += nAlienVel;
				}
			}
		}

		/* Handle player bullets ----------------------------------------------------------------*/

		for (int i = 0; i < (signed)vcPlayerBullets.size();  i++) {
			if (vcPlayerBullets[i].Y <= 0) {
				vcPlayerBullets.erase(vcPlayerBullets.begin() + i);
				continue;
			}

			vcPlayerBullets.at(i).Y -= nBulletSpeed;
		}

		/* Handle alien torpedoes ---------------------------------------------------------------*/

		int randomColumnIndex = rand() % vColumns.size();
		COORD firingAlien = vColumns[randomColumnIndex].back();

		if (iterations % nAlienFireRate == 0)
			vAlienTorpedoes.push_back({ firingAlien.X + 5, firingAlien.Y + 5});

		for (int i = 0; i < (signed)vAlienTorpedoes.size(); i++) {
			if (vAlienTorpedoes[i].Y >= nScreenHeight) {
				vAlienTorpedoes.erase(vAlienTorpedoes.begin() + i);
				continue;
			}

			vAlienTorpedoes[i].Y += nTorpedoVel;
		}

		/* Handle bullet block collisions -------------------------------------------------------*/

		// Player bullet
		for (int i = 0; i < (signed)vcPlayerBullets.size(); i++) {
			// Skip bullets that can't hit the blocks
			if (vcPlayerBullets[i].Y < nFirstBlockY)
				continue;

			collideProjectileWithBlock(vcPlayerBullets, i, nBlastRadius);
		}

		// Alien torpedo
		for (int i = 0; i < (signed)vAlienTorpedoes.size(); i++) {
			if (vAlienTorpedoes[i].Y > nFirstBlockY + nBlockSize)
				continue;
			
			collideProjectileWithBlock(vAlienTorpedoes, i, nBlastRadius);
		}

		/* Player bullets against aliens --------------------------------------------------------*/

		bool hitAlien = false;
		for (int i = 0; i < (signed)vcPlayerBullets.size(); i++) {
			hitAlien = false;
			for (int l = 0; l < (signed)vColumns.size(); l++) {
				for (int j = 0; j < (signed)vColumns[l].size(); j++) {
					// Only one alien asset has a different size than the others
					int width = 12;
					if (vColumns[l][j].Y % 3 == 0)
						width = 8;
					// All alien assets are the same height
					int height = alien1Asset.height;

					if (vcPlayerBullets[i].X > vColumns[l][j].X && vcPlayerBullets[i].X < vColumns[l][j].X + width &&
						vcPlayerBullets[i].Y > vColumns[l][j].Y && vcPlayerBullets[i].Y < vColumns[l][j].Y + height) {

						vcPlayerBullets.erase(vcPlayerBullets.begin() + i);

						if (!vColumns[l].empty())
							vColumns[l].erase(vColumns[l].begin() + j);

						hitAlien = true;
						break;
					}
					if (hitAlien)
						break;
				}
				if (hitAlien)
					break;
			}
		}

		// Manage empty columns of aliens
		for (int i = 0; i < (signed)vColumns.size(); i++) {
			if (vColumns[i].empty())
				vColumns.erase(vColumns.begin() + i);
		}

		/* Alien Torpedoes Against Player -------------------------------------------------------*/

		for (int i = 0; i < (signed)vAlienTorpedoes.size(); i++) {
			if (vAlienTorpedoes[i].X > nPlayerX && vAlienTorpedoes[i].X < nPlayerX + playerAsset.width &&
				vAlienTorpedoes[i].Y > nPlayerY && vAlienTorpedoes[i].Y < nPlayerY + playerAsset.height) {
				nPlayerLives--;
				vAlienTorpedoes.erase(vAlienTorpedoes.begin() + i);
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

		// Display torpedoes
		for (int i = 0; i < (signed)vAlienTorpedoes.size(); i++)
			// I want to make the x and y coordinates the tip of the torpedoe
			displayAsset(vAlienTorpedoes[i].X - (int(torpedoAsset.width / 2)), vAlienTorpedoes[i].Y - torpedoAsset.height, torpedoAsset, screen);

		// Display aliens
		for (int i = 0; i < (signed)vColumns.size(); i++) {
			for (int l = 0; l < (signed)vColumns[i].size(); l++) {
				COORD currentAlien = vColumns[i][l];
				if (currentAlien.Y % 3 == 0)
					displayAsset(currentAlien.X, currentAlien.Y, alien1Asset, screen);
				else if (currentAlien.Y % 3 == 1)
					displayAsset(currentAlien.X, currentAlien.Y, alien2Asset, screen);
				else if (currentAlien.Y % 3 == 2)
					displayAsset(currentAlien.X, currentAlien.Y, alien3Asset, screen);
			}
		}

		// Display player lives
		for (int i = 1; i < nPlayerLives + 1; i++) {
			displayAsset(7 * i, nScreenHeight - 7, playerAsset, screen);
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
					break;
			}
		}
	}
}

void collideProjectileWithBlock(vector<COORD>& projectileVector, int vectorIndex, int blastRadius) {
	int currentBlock = -1;
	bool collidedWithBlock = false;
	// l is equal to the x coordinate of the current block
	for (int l = nFirstBlockX; l < (nBlockSize + nBlockSpacing) * nNumberOfBlocks; l += nBlockSpacing + nBlockSize) {
		currentBlock++;
		for (int x = l; x < l + nBlockSize; x++) {
			for (int y = nFirstBlockY; y < nFirstBlockY + nBlockSize; y++) {
				// Skip if the projectile isn't colliding
				if (!(projectileVector[vectorIndex].X == x && projectileVector[vectorIndex].Y == y))
					continue;
				else {
					collidedWithBlock = true;
					// Only run this if the block isn't an empty space
					if (blocks[currentBlock][(y - nFirstBlockY) * nBlockSize + (x - l)] == '#') {
						// Loops through a smaller area which contains the blast radius, sort of like a bounding box
						for (int blastX = x - blastRadius; blastX < x + blastRadius; blastX++) {
							for (int blastY = y - blastRadius; blastY < y + blastRadius; blastY++) {
								// Use the pythagorean theorem to calculate if the current character cell falls within the blast radius
								int a = x - blastX;
								int b = y - blastY;
								double dist = sqrt(a * a + b * b);
								if (dist > (double)blastRadius)
									continue;

								// Keeps the x from overflowing onto the other side
								int xPortion = blastX - l;
								if (xPortion < 0)
									xPortion = 0;
								if (xPortion >= nBlockSize)
									xPortion = nBlockSize - 1;

								// Only changes the block string if it is sure that it is in range
								int blockCharacterIndex = (blastY - nFirstBlockY) * nBlockSize + xPortion;
								if (!(blockCharacterIndex < 0 || blockCharacterIndex >= nBlockSize * nBlockSize)) {
									blocks[currentBlock][blockCharacterIndex] = '.';
								}
							}
						}
						projectileVector.erase(projectileVector.begin() + vectorIndex);
						return;
					}
				}
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

