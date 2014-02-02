#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "ghost.h"

//TODO: remove the default causes from all of the switch statements
//or make them do something legitimate

void ghosts_init(Ghost ghosts[4])
{
	ghosts[0].ghostType = Blinky;
	ghosts[1].ghostType = Inky;
	ghosts[2].ghostType = Pinky;
	ghosts[3].ghostType = Clyde;

	for (int i = 0; i < 4; i++)
	{
		reset_ghost(&ghosts[i], ghosts[i].ghostType);
	}
}

void reset_ghost(Ghost *ghost, GhostType type)
{
	int x, y;
	int ox, oy;
	Direction dir;
	MovementMode mode;

	switch (type)
	{
		case Blinky:	{ x = 14; y = 11; ox = -8; oy =  0; mode = Chase; dir = Left; break; }
		case Inky:		{ x = 12; y = 14; ox = -8; oy =  0; mode = InPen; dir = Up;   break; }
		case Clyde:		{ x = 16; y = 14; ox = -8; oy =  0; mode = InPen; dir = Up;   break; }
		case Pinky:		{ x = 14; y = 14; ox = -8; oy =  0; mode = InPen; dir = Down; break; }

		//testing
		//case Inky:	{ x = 14; y = 11; ox = -8; oy =  0; mode = Chase; dir = Left; break; }
		//case Clyde:	{ x = 14; y = 11; ox = -8; oy =  0; mode = Chase; dir = Left; break; }
		//case Pinky:	{ x = 14; y = 11; ox = -8; oy =  0; mode = Chase; dir = Left; break; }

		default: printf("error ghost\naborting\n"); exit(1);
	}

	ghost->x = x;
	ghost->y = y;
	ghost->xTileOffset = ox;
	ghost->yTileOffset = oy;
	ghost->targetX = 0;
	ghost->targetY = 0;
	ghost->movementMode = mode;
	ghost->direction = dir;
	ghost->transDirection = Left;
	ghost->nextDirection = Left;
}


void send_to_home(Ghost *ghost, GhostType type)
{
	int targetX;
	int targetY;

	switch (type)
	{
		case Blinky: 	{ targetX =  4; targetY =   4; break; }
		case Inky:		{ targetX = 34; targetY =   4; break; }
		case Clyde:		{ targetX =  4; targetY =  34; break; }
		case Pinky:		{ targetX = 34; targetY =  34; break; }
		default: printf("error ghost\naborting\n"); exit(1);
	}
	
	ghost->targetX = targetX;
	ghost->targetY = targetY;
}

typedef struct 
{
	Direction dir;
	int distance;
} Targetcalc;

typedef struct 
{
	int x;
	int y;
} Boardoffset;

Direction next_direction(Ghost *ghost, Board *board)
{
	//ghost has just entered a new tile
	//we look ahead to their next tile
	//then we look at all the surrounding valid tiles
	//we get the distance from each of those to the ghosts target tile
	//we then select the one with the smallest distance
	//if there is a tie, we break it in order Up, Left, Down, Right

	Targetcalc targets[4] = {  {Up, INT_MAX}, {Left, INT_MAX}, {Down, INT_MAX}, {Right, INT_MAX} };

	Boardoffset offsets[4] = { {0, -1},       {-1, 0},         {0, 1},          {1, 0} };

	int x, y;

	if 		(ghost->nextDirection == Left) 	{ x = -1; y =  0; }
	else if (ghost->nextDirection == Right) { x =  1; y =  0; }
	else if (ghost->nextDirection == Up) 	{ x =  0; y = -1; }
	else 									{ x =  0; y =  1; }

	//calculate the distances between the squares (or if it is even valid)
	for (int i = 0; i < 4; i++)
	{
		int testX = ghost->x + x + offsets[i].x;
		int testY = ghost->y + y + offsets[i].y;

		//allow for when ghost is going through teleporter
		if (testX == 0) testX = 26;
		if (testX == 27) testX = 1;

		//make sure the square is a valid walkable square
		if (!(is_valid_square(board, testX, testY) || is_tele_square(testX, testY))) continue;

		//a further condition is that ghosts cannot enter certain squares moving upwards
		if (targets[i].dir == Up && is_ghost_noup_square(testX, testY)) continue;

		int dx = testX - ghost->targetX;
		int dy = testY - ghost->targetY;

		targets[i].distance = (int) sqrt(dx * dx + dy * dy);
	}

	//a ghost can never turn around, so exclude the opposite direction to what they are facing
	Direction reverseDir;

	if 		(ghost->nextDirection == Left) 	{ reverseDir = Right; }
	else if (ghost->nextDirection == Right) { reverseDir = Left; }
	else if (ghost->nextDirection == Up) 	{ reverseDir = Down; }
	else 									{ reverseDir = Up; }

	Targetcalc shortest = {Right, INT_MAX};

	//iterate backwards so that we get the tie-breaking logic for free
	for (int i = 3; i >= 0; i--)
	{
		if (targets[i].dir == reverseDir) continue;

		if (targets[i].distance <= shortest.distance)
		{
			shortest.distance = targets[i].distance;
			shortest.dir = targets[i].dir;
		}
	}

	return shortest.dir;
}

void execute_ghost_logic(Ghost *targetGhost, GhostType type, Ghost *redGhost, Pacman *pacman)
{
	if (targetGhost->movementMode == Scatter)
	{
		send_to_home(targetGhost, type);
	}

	switch (type)
	{
		case Blinky:	execute_red_logic(targetGhost, pacman); 			break;
		case Inky:		execute_blue_logic(targetGhost, redGhost, pacman);	break;
		case Clyde:		execute_orange_logic(targetGhost, pacman);			break;
		case Pinky:		execute_pink_logic(targetGhost, pacman);			break;
	}
}

void execute_red_logic(Ghost *redGhost, Pacman *pacman)
{
	// Red's AI is to set his target position to pacmans
	redGhost->targetX = pacman->body.x;
	redGhost->targetY = pacman->body.y;
}

void execute_pink_logic(Ghost *pinkGhost, Pacman *pacman)
{
	// Pinks's AI is to set his target position to pacmans, plus a few more in the distance
	
	int targetOffsetX;
	int targetOffsetY;
	
	switch (pacman->body.dir)
	{
		case Up:	{ targetOffsetX =  4; targetOffsetY = -4; } break;	//4 up AND 4 left, as per bug in original game
		case Down:	{ targetOffsetX =  0; targetOffsetY =  4; } break;
		case Left:	{ targetOffsetX = -4; targetOffsetY =  0; } break;
		case Right:	{ targetOffsetX =  4; targetOffsetY =  0; } break;
		default: printf("error direction\naborting\n"); exit(1);
	}
	
	pinkGhost->targetX = pacman->body.x + targetOffsetX;
	pinkGhost->targetY = pacman->body.y + targetOffsetY;
}

void execute_orange_logic(Ghost *orangeGhost, Pacman *pacman)
{
	// Orange's logic is in two parts:
	// If Pacmans distance is more than 5 squares away, his target is pacman
	// If Pacman is within 5 squares, his target is his home
	
	int dx = orangeGhost->x - pacman->body.x;
	int dy = orangeGhost->y - pacman->body.y;
	
	int distance = sqrt(dx * dx + dy * dy);

	if (distance >= 8)
	{
		execute_red_logic(orangeGhost, pacman);
	}
	else
	{
		send_to_home(orangeGhost, orangeGhost->ghostType);
	}
}

void execute_blue_logic(Ghost *blueGhost, Ghost *redGhost, Pacman *pacman)
{
	int targetOffsetX;
	int targetOffsetY;
	
	switch (pacman->body.dir)
	{
		case Up:	{ targetOffsetX =  2; targetOffsetY = -2; } break;	//2 up AND 2 left, as per bug in original game
		case Down:	{ targetOffsetX =  0; targetOffsetY =  2; } break;
		case Left:	{ targetOffsetX = -2; targetOffsetY =  0; } break;
		case Right:	{ targetOffsetX =  2; targetOffsetY =  0; } break;
		default: printf("error direction\naborting\n"); exit(1);
	}
	
	int tx = pacman->body.x + targetOffsetX;
	int ty = pacman->body.y + targetOffsetY;
	
	int rx = redGhost->x;
	int ry = redGhost->y;
	
	int targetX = 2 * tx - rx;
	int targetY = 2 * ty - ry;
	
	blueGhost->targetX = targetX;
	blueGhost->targetY = targetY;
}
