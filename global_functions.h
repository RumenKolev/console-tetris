#pragma once
#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H
#include <string>

//forward declared variables
extern std::wstring tetromino[7];
extern const int fieldWidth;
extern const int fieldHeight;
extern unsigned char* field;

int rotate(int px, int py, int rotation);
bool doesPieceFit(int pieceX, int pieceY, int rotation, int tetrominoIndex);
#endif // GLOBAL_FUNCTIONS_H
