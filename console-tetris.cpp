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


int rotate(int px, int py, int rotation) {
	int pi = 0;			        //index

	switch (rotation % 4) {
	case 0: //0 degrees
		pi = py * 4 + px;		//0 1 2 3
								//4 5 6 7
								//8 9 10 11
								//12 13 14 15
		break;
	case 1://90 degrees
		pi = 12 + py - (px * 4);
		break;
	case 2://180 degrees
		pi = 15 - (py * 4) - px;
		break;
	case 3://270 degrees;
		pi = pi = 3 - py + (px * 4);
		break;
	
	}
	return pi;
}

bool doesPieceFit(int pieceX, int pieceY, int rotation, int tetrominoIndex) {
	for (int x{ 0 }; x < 4; ++x) {
		for (int y{ 0 }; y < 4; ++y) {
			int pieceIndex = rotate(x, y, rotation);

			int fieldIndex = (pieceY + y)* fieldWidth + (pieceX + x);

			if ((pieceX + x) >= 0 && (pieceX + x) < fieldWidth) {
				if ((pieceY + y) >= 0 && (pieceY + y) < fieldHeight) {
					if (tetromino[tetrominoIndex][pieceIndex] == L'x' && field[fieldIndex] != 0) {
						return false;
					}
				}
			}
			
		}
	}
	return true;
}

int main()
{
	//create the different tetrominoes in 4x4 strings; L in before string for wstring literal, each row is appended separately for readability
	tetromino[0].append(L"..x.");
	tetromino[0].append(L"..x.");
	tetromino[0].append(L"..x.");
	tetromino[0].append(L"..x.");

	tetromino[1].append(L"....");
	tetromino[1].append(L".xx.");
	tetromino[1].append(L".xx.");
	tetromino[1].append(L"....");

	tetromino[2].append(L"....");
	tetromino[2].append(L"..xx");
	tetromino[2].append(L".xx.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"....");
	tetromino[3].append(L"xx..");
	tetromino[3].append(L".xx.");
	tetromino[3].append(L"....");

	tetromino[4].append(L".x..");
	tetromino[4].append(L".x..");
	tetromino[4].append(L".xx.");
	tetromino[4].append(L"....");

	tetromino[5].append(L"..x.");
	tetromino[5].append(L"..x.");
	tetromino[5].append(L".xx.");
	tetromino[5].append(L"....");

	tetromino[6].append(L"....");
	tetromino[6].append(L".xxx");
	tetromino[6].append(L"..x.");
	tetromino[6].append(L"....");

	field = new unsigned char[fieldHeight * fieldWidth]; //intialize the playing field cells as a one dimensional array, which functionally simulates a 2D plane
	for (int x = 0; x < fieldWidth; x++) {				 //fill the cells of the field
		for (int y = 0; y < fieldHeight; y++) {			 //y*w + x is used to simulate a plane off of the 1D array
			field[y * fieldWidth + x] = (x == 0 || x == fieldWidth - 1 ||  y == fieldHeight-1) ? 9 : 0; //9 is border, 0 is empty space
		}
	}

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int screenWidth;		//actual console buffer width
	int screenHeight;		//actual console buffer height
	wchar_t* screen = nullptr;
	//used to check if a change has been made to the buffer size (window resizing)
	int screenWidthFlag{0}; 
	int screenHeightFlag{0};
	
	int currentPiece{ 3 };				//index of the current tetromino
	int piecePosX{ fieldWidth/2 };		//upper left index(x) of the current piece 
	int piecePosY{ 0 };					//upper left index(y) of the current piece
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
					screen[y * screenWidth + x] = L" ABCDEFG=#"[field[y * fieldWidth + x]];	//9(#) for borders, 0 for usable field
				}
			}
		}


		//enter pieces into screen array (i.e. draw current piece on the screen)
		if (doesPieceFit(piecePosX, piecePosY, pieceRotation, currentPiece)) {
			for (int x = 0; x < 4; x++) {				 //fill the cells of the field
				for (int y = 0; y < 4; y++) {			 //y*w + x is used to simulate a plane off of the 1D array
					if (tetromino[currentPiece][rotate(x, y, pieceRotation)] != '.') {
						screen[(piecePosY + y) * screenWidth + (piecePosX + x)] = currentPiece+65;
					}
				}
			}
		}
		
		//player input
		if ((GetAsyncKeyState(0x25)) && doesPieceFit(piecePosX - 1, piecePosY, pieceRotation, currentPiece)) {
			--piecePosX;
		}
		else if ((GetAsyncKeyState(0x27)) && doesPieceFit(piecePosX + 1, piecePosY, pieceRotation, currentPiece)) {
			++piecePosX;
		}
	

		if ((GetAsyncKeyState(0x20)) && doesPieceFit(piecePosX, piecePosY, pieceRotation+1, currentPiece)) {
			//this if statement needs to be nested, so that the else statement is not dependant on rotateKeyHold, that way else executes only when the spacebar is not being held down
			if (!rotateKeyHold) {
				pieceRotation = ((pieceRotation + 1) % 4);
				rotateKeyHold = true;
			}
		}
		else {
			rotateKeyHold = false;
		}

		

		//display screen buffer
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);

		//check if piece can be moved downwards
		if (doesPieceFit(piecePosX, piecePosY + 1, pieceRotation, currentPiece)) {
			if(moveDown)
				++piecePosY;
		}//if it can't, then keep it in that position
		else {
			for (int x{0}; x < 4; x++) {
				for (int y{0}; y < 4; y++) {
					if (tetromino[currentPiece][rotate(x, y, pieceRotation)] != '.') {
						field[(piecePosY + y) * fieldWidth + x + piecePosX] = currentPiece+1;
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
						field[i * fieldWidth + x] = 8;
					}
				}
				//repeating code, need to relocate original code and make into function
				for (int x = 0; x < fieldWidth; x++) {
					for (int y = 0; y < fieldHeight; y++) {
						if (screen != nullptr) {
							screen[y * screenWidth + x] = L" ABCDEFG=#"[field[y * fieldWidth + x]];	//9(#) for borders, 0 for usable field
						}
					}
				}
				WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
				std::this_thread::sleep_for(500ms);

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
			
			currentPiece = rand() % 7;
			piecePosX = fieldWidth / 2;
			piecePosY = 0;

		}
		
	} 

	return 0;	
}

