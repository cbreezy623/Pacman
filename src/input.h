#pragma once

#include <SDL/SDL.h>
#include <stdbool.h>

#include "direction.h"

#define MAX_KEYS 323

//Call at the end of each round of keypresses. If this isn't called key_pressed() and key_released() will not work.
void keyevents_finished(void);

//Handles when a key is pressed down.
void handle_keydown(int keycode);

//Handles when a key is pressed up.
void handle_keyup(int keycode);

//Returns true if the current direction is currently pressed down.
//A direction can be bound to multiple keypressed, making this function input-method agnostic.
bool dir_key_held(Direction direction);

//If a directional key was pressed this tick, this function returns true
//and stores the pressed direction in the given pointer.
//If multiple directions were pressed at once, this function prioritises Up, Left, Down, Right.
//If no direction was pressed this tick, this function simply returns false. 
bool dir_pressed_now(Direction *direction);

//Tests if a particular key is pressed. Uses the SDLK keycode constants.
bool key_held(int key);

//Tests if the key was pressed this frame.
bool key_pressed(int key);

//Tests if the key was released this frame.
bool key_released(int key);
