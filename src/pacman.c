#include "pacman.h"

void pacman_init(Pacman *pacman)
{
	pacman_location_init(pacman);

	pacman->score = 0;
	pacman->livesLeft = 3;
	pacman->direction = Left;

	pacman->missedFrames = 0;
}

void pacman_level_init(Pacman *pacman)
{
	pacman_location_init(pacman);
}

void pacman_location_init(Pacman *pacman)
{
	pacman->x = 14;
	pacman->y = 23;

    pacman->xTileOffset = -8;
    pacman->yTileOffset = 0;

    pacman->movementType = Unstuck;
    pacman->direction = Left;
    pacman->lastAttemptedMoveDirection = Left;
}
