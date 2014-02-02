#pragma once

//28 across, 36 down, 16 pixels each square.
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

//A simple wrapper around SDL_GetTicks().
//This allows us to pause the game though, by subtracting our pause time from SDL_GetTicks().
unsigned int ticks_game(void);

//Returns SDL_GetTicks().
//This allows us to know the actual time the game has been running for.
unsigned int ticks_startup(void);

//Returns the number of frames that have happened of actual game-time.
unsigned int frames_game(void);
//Returns the number of frames that have happened since the program starting.
unsigned int frames_startup(void);

//Copies -1, 0, 1 to the given x and y ints depending on what direction is given.
void dir_xy(Direction dir, int *x, int *y);

//Same behaviour as dir_xy() except this emulates the bug for ghosts where
//AI based on up direction is both Up *and* Left.
void dir_xy_buggy(Direction dir, int *x, int *y);

//Returns the opposite direction to the one given.
Direction dir_opposite(Direction dir);