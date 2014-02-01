#pragma once

#include "fruit.h"
#include "ghost.h"
#include "pacman.h"
#include "pellet.h"

//Returns true if pacman is colliding with the given pellet.
bool pacman_pellet_collides(Pacman *pacman, Pellet *pellet);

//Returns true if pacman is colliding with the given fruit.
bool pacman_fruit_collides(Pacman *pacman, GameFruit *gameFruit);

//Returns true if pacman is colliding with the given ghost.
bool pacman_ghost_collides(Pacman *pacman, Ghost *ghost);