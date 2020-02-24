#include "global_functions.h"

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

			int fieldIndex = (pieceY + y) * fieldWidth + (pieceX + x);

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

