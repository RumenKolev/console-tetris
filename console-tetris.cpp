#include <iostream>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <thread>
#include "global_functions.h"

using namespace std;

wstring tetromino[7];		    //holds the different types of tetrominoes, wstring for extended char set (because of Windows.h)

//field is segmented in cells
const int fieldWidth = 12;		
const int fieldHeight = 18;			
unsigned char *field = nullptr;	


int main()
{
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
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 ||  y == fieldHeight-1) ? 1 : 0; //1 is border, 0 is empty space
		}
	}

	//initialize screen buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int screenWidth;		
	int screenHeight;		
	wchar_t* screen = nullptr;
	//used to check if a change has been made to the buffer size (window resizing)
	int screenWidthFlag{0}; 
	int screenHeightFlag{0};
	

	//variables holding information about the current piece
	int currentTetromino{ 3 };				
	int piecePosX{ fieldWidth/2 };		//upper left x-Coordinate of the current piece 
	int piecePosY{ 0 };					
	int pieceRotation{ 0 };				//the current rotation of the piece


	bool rotateKeyHold{ false };		//is the spacebar (rotation key) held down?
	bool moveDown{ false };
	int tickSpeed{ 8 };
	int tickCounter{ 0 };

	//game loop
	bool gameOver = false;
	while (!gameOver) {
		this_thread::sleep_for(50ms);			//game clock tick, reason for graphical glitches while resizing
		tickCounter = (tickCounter+1)%(tickSpeed+1);
		moveDown = (tickSpeed == tickCounter);
		
		//get the buffer info to check if any window resizing has been done
		GetConsoleScreenBufferInfo(hConsole, &csbi);

		//use the current(i.e. resized) buffer sizes for the screen array
		screenWidth = csbi.dwSize.X;			//Console screen width (x) (columns)
		screenHeight = csbi.dwSize.Y;			//Console screen height (y) (rows)

		//check for resizes of the window and initialize the screen array 
		if (screenWidthFlag != screenWidth || screenHeightFlag != screenHeight) {
			if (screen != nullptr) {			//deallocate memory in case of resizing
				delete[] screen;
			}
			//create screen buffer, which is a 1D array of characters (later also simulating a 2D plane)
			screen= new wchar_t[screenWidth * screenHeight];
			for (int i = 0; i < screenWidth * screenHeight; i++) { //fill screen buffer with empty characters
				screen[i] = L' ';
			}
			screenWidthFlag = screenWidth;
			screenHeightFlag = screenHeight;
		}
		

		//draws field
		//enter a character into the buffer at the position, that corresponds to the position of the cell on the play-field
		//this updates screen (i.e. the buffer, i.e. the animations on screen)
		for (int x = 0; x < fieldWidth; x++) {
			for (int y = 0; y < fieldHeight; y++) {
				if (screen != nullptr) {
					screen[y * screenWidth + x] = L" #=O"[field[y * fieldWidth + x]];	//0-  usable field, 1-# border, 2-= line, 3-O piece 
				}
			}
		}


		//enter pieces into screen array (i.e. draw current piece on the screen)
		if (doesPieceFit(piecePosX, piecePosY, pieceRotation, currentTetromino)) {
			for (int x = 0; x < 4; x++) {				 //fill the cells of the field
				for (int y = 0; y < 4; y++) {			 //y*w + x is used to simulate a plane off of the 1D array
					if (tetromino[currentTetromino][rotate(x, y, pieceRotation)] != '.') {
						screen[(piecePosY + y) * screenWidth + (piecePosX + x)] = 'O';
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
	

		if ((GetAsyncKeyState(0x20)) && doesPieceFit(piecePosX, piecePosY, pieceRotation+1, currentTetromino)) {
			//this if statement needs to be nested, so that the else statement is not dependant on rotateKeyHold, that way else executes only when the spacebar is not being held down
			if (!rotateKeyHold) {
				pieceRotation = ((pieceRotation + 1) % 4);
				rotateKeyHold = true;
			}
		}
		else {
			rotateKeyHold = false;
		}

		
		if ((GetAsyncKeyState(0x28)) && doesPieceFit(piecePosX, piecePosY, pieceRotation, currentTetromino)) {
			++piecePosY;
		}
		
		

		//display screen buffer
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
		//check if piece can be moved downwards
		if (doesPieceFit(piecePosX, piecePosY + 1, pieceRotation, currentTetromino)) {
			if(moveDown)
				++piecePosY;
		}//if it can't, then keep it in that position
		else {
			for (int x{0}; x < 4; x++) {
				for (int y{0}; y < 4; y++) {
					if (tetromino[currentTetromino][rotate(x, y, pieceRotation)] != '.') {
						field[(piecePosY + y) * fieldWidth + x + piecePosX] = 3;
					}
				}
			}
			//check for lines 
			bool lineExists{ true };		//only true if a whole line exists on the current y-coordinate
			std::vector<int> lineIndices;	//contains the y-Coordinates of the lines that are to be removed
			for (int y = piecePosY; y < fieldHeight-1; ++y){
				lineExists = true;
				for (int x{ 1 }; x < fieldWidth-1; ++x){
					lineExists &= (field[y * fieldWidth + x] != 0);
				}
				if (lineExists) {
					lineIndices.push_back(y);
				}
			}
			
			//display ==== on lines
			if (lineIndices.size() > 0) {
				for (auto& i : lineIndices) {
					for (int x{ 1 }; x < fieldWidth - 1; ++x) {
						field[i * fieldWidth + x] = 2;
					}
				}
				//repeating code, need to relocate original code and make into function
				for (int x = 0; x < fieldWidth; x++) {
					for (int y = 0; y < fieldHeight; y++) {
						if (screen != nullptr) {
							screen[y * screenWidth + x] = L" #=O"[field[y * fieldWidth + x]];	//0-  usable field, 1-# border, 2-= line, 3-O piece 
						}
					}
				}
				WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
				this_thread::sleep_for(500ms);

				//remove lines
				for (auto& i : lineIndices) {
					for (int x{ 1 }; x < fieldWidth - 1; ++x) {
						field[i * fieldWidth + x] = 0;
						for (int y = i; y > 0; --y) {
							field[(y)*fieldWidth + x] = field[(y - 1) * fieldWidth + x];
						}
					}

				}
				lineIndices.clear();
			}
			
			currentTetromino = rand() % 7;
			piecePosX = fieldWidth / 2;
			piecePosY = 0;
			if (!doesPieceFit(piecePosX, piecePosY, pieceRotation, currentTetromino)) {
				gameOver = true;
			}
		}
		
	} 

	return 0;	
}

