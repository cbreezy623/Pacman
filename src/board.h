#pragma once

#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#define BOARD_LENGTH 28
#define BOARD_HEIGHT 31

typedef struct 
{
	int x, y;
	bool walkable;
	SDL_Surface *image;
} BoardSquare;

typedef struct 
{
	BoardSquare boardSquares[BOARD_LENGTH][BOARD_HEIGHT];
} Board;

bool is_valid_square(Board *board, int x, int y);

//TODO: decide if this uses the board or not
bool is_tele_square(int x, int y);
