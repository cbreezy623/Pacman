#pragma once

#include <stdbool.h>

#include "main.h"

//Returns true if pacman is colliding with the given pellet.
//bool pacman_pellet_collides(Pacman *pacman, Pellet *pellet);

//Returns true if pacman is colliding with the given fruit.
//bool pacman_fruit_collides(Pacman *pacman, GameFruit *gameFruit);

//Returns true if pacman is colliding with the given ghost.
//bool pacman_ghost_collides(Pacman *pacman, Ghost *ghost);

//Defines a body which has a position
typedef struct
{
	int x;
	int y;
	int xOffset;
	int yOffset;
	Direction dir;
	int velocity;
} PhysicsBody;

//Determines if the bodies collide based soley on their square on the board.
bool collides(PhysicsBody *body1, PhysicsBody *body2);

//Determines if the bodies collide based on their square and position offset within the square.
bool collides_accurate(PhysicsBody *body1, PhysicsBody *body2);

bool collides_obj(PhysicsBody *body, int otherX, int otherY);

//Advances the body 1 step based on current direction and velocity.
//Returns true if the body changed square during the move, false otherwise
bool move(PhysicsBody *body);
