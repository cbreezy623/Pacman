#pragma once

#include <stdbool.h>

#define NUM_FRUIT 8

typedef enum
{
	Cherry,
	Strawberry,
	Peach,
	Apple,
	Grapes,
	Galaxian,
	Bell,
	Key
} Fruit;

// Different modes a fruit can be in while in-game
// Either not-displayed (haven't eaten enough pellets yet)
// Displaying
// Displayed (either we timed-out or the player ate the fruit)
typedef enum
{
	NotDisplaying,
	Displaying,
	Displayed
} FruitMode;

typedef struct
{
	Fruit fruit;					// the fruit we are displaying
	FruitMode fruitMode;			// the mode the fruit is currently in
	unsigned int startedAt;			// the time we started displaying this fruit at (in SDL_Ticks())
	unsigned int displayTime;		// the random time between 9-10 this fruit displays for
	int x;
	int y;
	bool eaten;						// if the fruit was eaten while it displayed
	unsigned int eatenAt;			// the time it was eaten (used for displaying the points gained)
} GameFruit;

int fruit_points(Fruit fruit);

Fruit fruit_for_level(int level);

int rand_fruit_visible_time(void);

//resets fruit back to default state
void reset_fruit(GameFruit *gameFruit);

//assigns the appropriate fruit based of the level, and generates a visible-time for the fruit
void regen_fruit(GameFruit *gameFruit, int level);
