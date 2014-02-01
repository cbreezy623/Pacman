#include <stdio.h>
#include <stdlib.h>

#include "board.h"

bool is_valid_square(Board *board, int x, int y)
{
	if (x < 0 || y < 0) return false;
	if (x >= BOARD_LENGTH || y >= BOARD_HEIGHT) return false;

	return board->boardSquares[x][y].walkable;
}

bool is_tele_square(int x, int y)
{
	if (x == -1 && y == 14) return true;
	else if (x == 28 && y == 14) return true;
	else return false;
}
