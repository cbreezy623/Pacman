#pragma once

#include "main.h"
#include "pacman.h"

typedef enum 
{
	Blinky,		// red ghost
	Inky,		// cyan
	Pinky,		// pink
	Clyde		// orange
} GhostType;

typedef enum
{
	Chase,
	Scatter,
	Frightened
} MovementMode;

typedef struct 
{
	int x;
	int y;
	int xTileOffset;
	int yTileOffset;
	int targetX;
	int targetY;
	GhostType ghostType;
	MovementMode movementMode;
} Ghost;

void execute_ghost_logic(Ghost *targetGhost, GhostType type, Ghost *redGhost, Pacman *pacman);

void send_to_home(Ghost *ghost, GhostType type);

void execute_red_logic(Ghost *redGhost, Pacman *pacman);
void execute_pink_logic(Ghost *pinkGhost, Pacman *pacman);
void execute_orange_logic(Ghost *orangeGhost, Pacman *pacman);
void execute_blue_logic(Ghost *blueGhost, Ghost *redGhost, Pacman *pacman);
