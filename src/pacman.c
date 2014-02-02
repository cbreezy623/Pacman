#include "pacman.h"

void pacman_init(Pacman *pacman)
{
	pacman_location_init(pacman);

	pacman->score = 0;
	pacman->livesLeft = 3;

	pacman->missedFrames = 0;
}

void pacman_level_init(Pacman *pacman)
{
	pacman_location_init(pacman);
}

void pacman_location_init(Pacman *pacman)
{
	pacman->body = (PhysicsBody) {14, 23, -8, 0, Left, 1};

    pacman->movementType = Unstuck;
    pacman->lastAttemptedMoveDirection = Left;
}
