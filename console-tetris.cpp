#include <iostream>
#include <vector>
#include <Windows.h>
#include <stdio.h>
#include <thread>
using namespace std;

wstring tetromino[7];		    //holds the different types of tetrominoes, wstring for extended char set (because of Windows.h)

//field segmented in cells
int fieldWidth = 12;			//12 cells wide
int fieldHeight = 18;			//18 cells tall
unsigned char *field = nullptr;	//field array


int rotate(int px, int py, int r) {
	int pi = 0;			        //index

	switch (r % 4) {
	case 0: //0 degrees
		pi = py * 4 + px;		//0 1 2 3
								//4 5 6 7
								//8 9 10 11
								//12 13 14 15
	case 1://90 degrees
		pi = 12 + py - (px * 4);
	case 2://180 degrees
		pi = 15 - (py * 4) - px;
	case 3://270 degrees;
		pi = pi = 3 - py + (px * 4);
	
	}
	return pi;
}

int main()
{
   

	//create the different tetrominoes in 4x4 strings; L in before string for wstring literal, each row is appended separately for readability
	tetromino[0].append(L"..x.");
	tetromino[0].append(L"..x.");
	tetromino[0].append(L"..x.");
	tetromino[0].append(L"..x.");

	tetromino[1].append(L"....");
	tetromino[1].append(L"..xx");
	tetromino[1].append(L"..xx");
	tetromino[1].append(L"....");

	tetromino[2].append(L"....");
	tetromino[2].append(L"..xx");
	tetromino[2].append(L".xx.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L"xx..");
	tetromino[3].append(L".xx.");
	tetromino[3].append(L"....");

	tetromino[4].append(L"....");
	tetromino[4].append(L"x...");
	tetromino[4].append(L"x...");
	tetromino[4].append(L"xx..");

	tetromino[5].append(L"....");
	tetromino[5].append(L"...x");
	tetromino[5].append(L"...x");
	tetromino[5].append(L"..xx");

	tetromino[6].append(L"....");
	tetromino[6].append(L".xxx");
	tetromino[6].append(L"..x.");
	tetromino[6].append(L"....");

	field = new unsigned char[fieldHeight * fieldWidth]; //intialize the playing field cells as a one dimensional array, which functionally simulates a 2D plane
	for (int x = 0; x < fieldWidth; x++) {				 //fill the cells of the field
		for (int y = 0; y < fieldHeight; y++) {			 //y*w + x is used to simulate a plane off of the 1D array
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 ||  y == fieldHeight-1) ? 9 : 0; //9 is border
		}
	}

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int screenWidth;		//actual console buffer width
	int screenHeight;		//actual console buffer height
	//used to check if a change has been made to the buffer size (window resizing)
	int screenWidthFlag{0}; 
	int screenHeightFlag{0};
	wchar_t* screen = nullptr;
	
	


	//game loop
	bool gameOver = false;
	while (!gameOver) {

		this_thread::sleep_for(50ms);			//game clock tick, reason for graphical glitches while resizing

		//get the buffer info to check if any window resizing has been done
		GetConsoleScreenBufferInfo(hConsole, &csbi);


		//use the current buffer sizes for our screen array
		screenWidth = csbi.dwSize.X;			//Console screen width (x) (columns)
		screenHeight = csbi.dwSize.Y;			//Console screen height (y) (rows)

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
					screen[y * screenWidth + x] = L" ABCDEFG=#"[field[y * fieldWidth + x]];	//9(#) for borders, 0 for usable field, etc
				}
			}
		}
		//display screen buffer

		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
	} 

	return 0;	
}

