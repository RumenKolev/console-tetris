#include <iostream>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include "global_functions.h"
#include <string>
using namespace std;

wstring tetromino[7];		    //holds the different types of tetrominoes, wstring for extended char set (because of Windows.h)

//field is segmented in cells
const int fieldWidth = 12;
const int fieldHeight = 18;
unsigned char* field = nullptr;
vector<WORD> pieceColors{ 0, //black for empty space
							   BACKGROUND_BLUE | BACKGROUND_GREEN  ,	//cyan
							   BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,	//yellow?
							   BACKGROUND_GREEN | BACKGROUND_INTENSITY,
							   BACKGROUND_RED | BACKGROUND_INTENSITY,
							   BACKGROUND_RED | BACKGROUND_GREEN, //ORANGE?
							   BACKGROUND_BLUE | BACKGROUND_INTENSITY,
							   BACKGROUND_RED | BACKGROUND_BLUE, //purple
							   BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY,//white for borders
							   BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED //white for lines
};//hold the colors corresponding to the tetrominoes, currentTetromino to be used for indexing

int main()
{
	cout << "Welcome to Console-Tetris! Press enter to start the game...";
	cin.get();	//waits for input to start game
	system("CLS");

	//the different tetrominoes in 4x4(=16 char long) strings; L in before string for wstring literal, each row can be appended separately for readability
	tetromino[0].append(L"..x...x...x...x.");
	tetromino[1].append(L".....xx..xx.....");
	tetromino[2].append(L"......xx.xx.....");
	tetromino[3].append(L"....xx...xx.....");
	tetromino[4].append(L".x...x...xx.....");
	tetromino[5].append(L"..x...x..xx.....");
	tetromino[6].append(L".....xxx..x.....");

	//intialize the playing field cells as a one dimensional array, which functionally simulates a 2D plane
	field = new unsigned char[fieldHeight * fieldWidth];
	for (int x = 0; x < fieldWidth; x++) {
		for (int y = 0; y < fieldHeight; y++) {			 //y*w + x is used to simulate a plane off of the 1D array
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 || y == fieldHeight - 1) ? 8 : 0; //8 is border, 0 is empty space
		}
	}

	//initialize screen buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int screenWidth{ 0 };
	int screenHeight{ 0 };
	CHAR_INFO* screen = nullptr;
	SMALL_RECT rectangle = { 0 ,0,static_cast<short>(screenWidth),static_cast<short>(screenHeight) };
	//initializations of buffer scroller variables
	SMALL_RECT movedRect = { 0,0, static_cast<short>(screenWidth), static_cast<short>(screenHeight) };
	COORD newLocation = { static_cast<short>(screenWidth / 2), 0 };
	CHAR_INFO clipCharInfo;

	//used to check if a change has been made to the buffer size (window resizing)
	int screenWidthFlag{ 0 };
	int screenHeightFlag{ 0 };


	//variables holding information about the current piece
	srand(time(0));
	int currentTetromino{ rand() % 7 };
	int piecePosX{ fieldWidth / 2 };		//upper left x-Coordinate of the current piece 
	int piecePosY{ 0 };
	int pieceRotation{ 0 };				//the current rotation of the piece


	bool rotateKeyHold{ false };		//is the spacebar (rotation key) held down?
	bool moveDown{ false };
	int tickSpeed{ 8 };
	int tickCounter{ 0 };

	int currentScore{ 0 };
	string scoreString{ "Score: " };
	//game loop
	bool gameOver = false;
	while (!gameOver) {
		this_thread::sleep_for(50ms);			//game clock tick, reason for graphical glitches while resizing
		tickCounter = (tickCounter + 1) % (tickSpeed + 1);
		moveDown = (tickSpeed == tickCounter);

		//get the buffer info to check if any window resizing has been done
		GetConsoleScreenBufferInfo(hConsole, &csbi);

		//use the current(i.e. resized) buffer sizes for the screen array
		screenWidth = csbi.dwSize.X;			//Console screen width (x) (columns)
		screenHeight = csbi.dwSize.Y;			//Console screen height (y) (rows)


		//check for resizes of the window and initialize the screen array 
		if (screenWidthFlag != screenWidth || screenHeightFlag != screenHeight) {
			if (screen != nullptr) {			//deallocate memory in case of resizing;
				delete[] screen;
			}
			//create screen buffer, which is a 2D array of characters with attributes (later also simulating a 2D plane)
			screen = new CHAR_INFO[screenWidth * screenHeight];
			for (int i = 0; i < screenWidth * screenHeight; i++) { //fill screen buffer with empty characters
				screen[i].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
				screen[i].Char.UnicodeChar = L' ';

			}
			movedRect.Right = screenWidth;
			movedRect.Bottom = screenHeight;
			newLocation.X = screenWidth / 2;

			clipCharInfo.Attributes = 0;
			//scrolls the screen buffer, so that all artefacts on the left of the field are cleared
			ScrollConsoleScreenBuffer(hConsole, &movedRect, NULL, newLocation, &clipCharInfo);
			rectangle = { static_cast<short>(screenWidth / 2 - fieldWidth / 2) ,0,static_cast<short>(screenWidth),static_cast<short>(screenHeight) };
			screenWidthFlag = screenWidth;
			screenHeightFlag = screenHeight;
		}





		//enter pieces into screen array (i.e. draw current piece on the screen)
		if (doesPieceFit(piecePosX, piecePosY, pieceRotation, currentTetromino)) {
			for (int x = 0; x < 4; x++) {				 //fill the cells of the field
				for (int y = 0; y < 4; y++) {			 //y*w + x is used to simulate a plane off of the 1D array
					if (tetromino[currentTetromino][rotate(x, y, pieceRotation)] != '.') {
						screen[(piecePosY + y) * screenWidth + (piecePosX + x)].Char.UnicodeChar = ' ';
						screen[(piecePosY + y) * screenWidth + (piecePosX + x)].Attributes = pieceColors[currentTetromino + 1];
					}
				}
			}
		}

		//player input
		if ((GetAsyncKeyState(0x25)) && doesPieceFit(piecePosX - 1, piecePosY, pieceRotation, currentTetromino)) {
			--piecePosX;
		}
		if ((GetAsyncKeyState(0x27)) && doesPieceFit(piecePosX + 1, piecePosY, pieceRotation, currentTetromino)) {
			++piecePosX;
		}


		if ((GetAsyncKeyState(0x20)) && doesPieceFit(piecePosX, piecePosY, pieceRotation + 1, currentTetromino)) {
			//this if statement needs to be nested, so that the else statement is not dependant on rotateKeyHold, that way else executes only when the spacebar is not being held down
			if (!rotateKeyHold) {
				pieceRotation = ((pieceRotation + 1) % 4);
				rotateKeyHold = true;
			}
		}
		else {
			rotateKeyHold = false;
		}


		if ((GetAsyncKeyState(0x28)) && doesPieceFit(piecePosX, piecePosY + 1, pieceRotation, currentTetromino)) {
			piecePosY++;
		}


		vector<int> lineIndices;
		//display screen buffer
		WriteConsoleOutput(hConsole, screen, { static_cast<short>(screenWidth) , static_cast<short>(screenHeight) }, { 0,0 }, & rectangle);
		//check if piece can be moved downwards
		if (doesPieceFit(piecePosX, piecePosY + 1, pieceRotation, currentTetromino)) {
			if (moveDown)
				++piecePosY;
		}//if it can't, then keep it in that position
		else {
			for (int x{ 0 }; x < 4; x++) {
				for (int y{ 0 }; y < 4; y++) {
					if (tetromino[currentTetromino][rotate(x, y, pieceRotation)] != '.') {
						field[(piecePosY + y) * fieldWidth + x + piecePosX] = currentTetromino + 1;
					}
				}
			}
			//check for lines 
			bool lineExists{ true };		//only true if a whole line exists on the current y-coordinate

			for (int y = piecePosY; y < fieldHeight - 1; ++y) {
				lineExists = true;
				for (int x{ 1 }; x < fieldWidth - 1; ++x) {
					lineExists &= (field[y * fieldWidth + x] != 0);
				}
				if (lineExists) {
					lineIndices.push_back(y);
				}
			}

			//display ==== on lines
			if (!lineIndices.empty()) {
				for (auto& i : lineIndices) {
					for (int x{ 1 }; x < fieldWidth - 1; ++x) {
						field[i * fieldWidth + x] = 9;
					}
				}
				switch (lineIndices.size()) {
				case 1:
					currentScore += 40;
					break;
				case 2:
					currentScore += 100;
					break;
				case 3:
					currentScore += 300;
					break;
				case 4:
					currentScore += 1200;
					break;
				}
			}

			currentTetromino = rand() % 7;
			piecePosX = fieldWidth / 2;
			piecePosY = 0;
			pieceRotation = 0;
			if (!doesPieceFit(piecePosX, piecePosY, pieceRotation, currentTetromino)) {
				gameOver = true;
			}
		}


		//draws field
		//enter a character into the buffer at the position, that corresponds to the position of the cell on the play-field
		//this updates screen (i.e. the buffer, i.e. the animations on screen)
		for (int x = 0; x < fieldWidth; x++) {
			for (int y = 0; y < fieldHeight; y++) {
				if (screen != nullptr) {
					//screen[y * screenWidth + x].Char.UnicodeChar = L" OOOOOOO#="[field[y * fieldWidth + x]];	//0:  usable field, 8: # border, 9: = line, 1-7: O piece 
					screen[y * screenWidth + x].Attributes = pieceColors[field[y * fieldWidth + x]];
				}
			}
		}

		//display score just under the field (using pointer iteration)
		for (int i{ 0 }; i < (scoreString + to_string(currentScore)).length(); i++) {
			(screen + screenWidth * (fieldHeight)+i)->Char.UnicodeChar = (scoreString + to_string(currentScore))[i];
		}


		if (!lineIndices.empty()) {
			//remove lines
			for (auto& i : lineIndices) {
				for (int x{ 1 }; x < fieldWidth - 1; ++x) {
					field[i * fieldWidth + x] = 0;
					for (int y = i; y > 0; --y) {
						field[(y)*fieldWidth + x] = field[(y - 1) * fieldWidth + x];
					}
				}

			}
			WriteConsoleOutput(hConsole, screen, { static_cast<short>(screenWidth) , static_cast<short>(screenHeight) }, { 0,0 }, & rectangle);
			this_thread::sleep_for(500ms);
			lineIndices.clear();
		}
	}
	CloseHandle(hConsole);
	cout << "Game Over\n" << "Your score is: " << currentScore << '\n' << "Press enter to close the program." << endl;
	cin.get();	//waits for input to close game over menu
	return 0;
}

