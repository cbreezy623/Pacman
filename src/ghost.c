#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "ghost.h"

//TODO: remove the default causes from all of the switch statements
//or make them do something legitimate

void send_to_home(Ghost *ghost, GhostType type)
{
	int targetX;
	int targetY;

	switch (type)
	{
		case Blinky: 	{ targetX =  4; targetY =  4;  break; }
		case Inky:		{ targetX = 34; targetY =  4;  break; }
		case Clyde:		{ targetX =  4; targetY =  34; break; }
		case Pinky:		{ targetX = 34; targetY =  34; break; }
		default: printf("error direction\naborting\n"); exit(1);
	}
	
	ghost->targetX = targetX;
	ghost->targetY = targetY;
}

void execute_ghost_logic(Ghost *targetGhost, GhostType type, Ghost *redGhost, Pacman *pacman)
{
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
	redGhost->targetX = pacman->x;
	redGhost->targetY = pacman->y;
}

void execute_pink_logic(Ghost *pinkGhost, Pacman *pacman)
{
	// Pinks's AI is to set his target position to pacmans, plus a few more in the distance
	
	int targetOffsetX;
	int targetOffsetY;
	
	switch (pacman->direction)
	{
		case Up:	{ targetOffsetX =  4; targetOffsetY = -4; } break;	//4 up AND 4 left, as per bug in original game
		case Down:	{ targetOffsetX =  0; targetOffsetY =  4; } break;
		case Left:	{ targetOffsetX = -4; targetOffsetY =  0; } break;
		case Right:	{ targetOffsetX =  4; targetOffsetY =  0; } break;
		default: printf("error direction\naborting\n"); exit(1);
	}
	
	pinkGhost->targetX = pacman->x + targetOffsetX;
	pinkGhost->targetY = pacman->y + targetOffsetY;
}

void execute_orange_logic(Ghost *orangeGhost, Pacman *pacman)
{
	// Orange's logic is in two parts:
	// If Pacmans distance is more than 5 squares away, his target is pacman
	// If Pacman is within 5 squares, his target is his home
	
	int dx = orangeGhost->x - pacman->x;
	int dy = orangeGhost->y - pacman->x;
	
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
	
	switch (pacman->direction)
	{
		case Up:	{ targetOffsetX =  2; targetOffsetY = -2; } break;	//2 up AND 2 left, as per bug in original game
		case Down:	{ targetOffsetX =  0; targetOffsetY =  2; } break;
		case Left:	{ targetOffsetX = -2; targetOffsetY =  0; } break;
		case Right:	{ targetOffsetX =  2; targetOffsetY =  0; } break;
		default: printf("error direction\naborting\n"); exit(1);
	}
	
	int tx = pacman->x + targetOffsetX;
	int ty = pacman->y + targetOffsetY;
	
	int rx = redGhost->x;
	int ry = redGhost->y;
	
	int targetX = 2 * tx - rx;
	int targetY = 2 * ty - ry;
	
	blueGhost->targetX = targetX;
	blueGhost->targetY = targetY;
}
