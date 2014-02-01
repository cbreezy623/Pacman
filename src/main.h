#pragma once

#include <stdbool.h>

//28 across, 36 down, 16 pixels each square
#define SCREEN_TITLE "Tphakumahn"
#define SCREEN_WIDTH 448
#define SCREEN_HEIGHT 576

//Defines four directions.
typedef enum 
{
	Up,
	Down,
	Left,
	Right
} Direction;

//Defines the main states the program can be in.
typedef enum
{
	Menu,
	Game,
	Intermission
} ProgramState;

//Returns the number of credits the user currently has.
int num_credits(void);

//a simple wrapper around SDL_GetTicks()
//this allows us to pause the game though, by subtracting our pause time from SDL_GetTicks()
unsigned int ticks_game(void);

//Returns SDL_GetTicks().
//This allows us to know the actual time the game has been running for
unsigned int ticks_startup(void);
