#include "physics.h"

bool pacman_pellet_collides(Pacman *pacman, Pellet *pellet)
{
	//TODO: make this a little more accurate
	//collision occurs if they are on same center tile

	//we'll be mean...
	return  pacman->xTileOffset == 0 && pacman->yTileOffset == 0 && 
			pellet->x == pacman->x && pellet->y == pacman->y;
}

bool pacman_fruit_collides(Pacman *pacman, GameFruit *fruit)
{
	//TODO: make this a little more accurate
	return 	//pacman->xTileOffset == 0 && pacman->yTileOffset == 0 &&
			fruit->x == pacman->x && fruit->y == pacman->y;
}

bool pacman_ghost_collides(Pacman *pacman, Ghost *ghost)
{
	//TODO: make this more accurate
	return pacman->x == ghost->x && pacman->y == ghost->y; 	
	//pacman->xTileOffset == ghost->xTileOffset && pacman->yTileOffset == ghost->yTileOffset &&
}
