#include "input.h"

//does a check to ensure the given keycode is within the bounds of valid keys
//stops the program with an error if it is out of bounds
static void check_keycode(int keycode);

static bool keysHeld[MAX_KEYS] = {false};

static unsigned int keysPressed[MAX_KEYS] = {0};
static unsigned int keysReleased[MAX_KEYS] = {0};

static unsigned int curKeyFrame = 1;

void keyevents_finished(void)
{
    curKeyFrame++;
}

void handle_keydown(int keycode)
{
    check_keycode(keycode);

    if (!keysHeld[keycode]) keysPressed[keycode] = curKeyFrame;

    keysHeld[keycode] = true;
}

void handle_keyup(int keycode)
{
    check_keycode(keycode);

    if (keysHeld[keycode]) keysReleased[keycode] = curKeyFrame;

    keysHeld[keycode] = false;
}

bool dir_key_held(Direction direction)
{
    switch (direction)
    {
        case Up:    return keysHeld[SDLK_UP]    || keysHeld[SDLK_w];
        case Down:  return keysHeld[SDLK_DOWN]  || keysHeld[SDLK_s];
        case Left:  return keysHeld[SDLK_LEFT]  || keysHeld[SDLK_a];
        case Right: return keysHeld[SDLK_RIGHT] || keysHeld[SDLK_d];
    }

    printf("should never reach here\n");
    exit(1);
}

bool dir_pressed_now(Direction *dir)
{
    if (keysHeld[SDLK_UP]    || keysHeld[SDLK_w]) { *dir = Up;    return true; }
    if (keysHeld[SDLK_LEFT]  || keysHeld[SDLK_a]) { *dir = Left;  return true; }
    if (keysHeld[SDLK_DOWN]  || keysHeld[SDLK_s]) { *dir = Down;  return true; }
    if (keysHeld[SDLK_RIGHT] || keysHeld[SDLK_d]) { *dir = Right; return true; }

    return false;
}

bool key_held(int keycode)
{
    check_keycode(keycode);

    return keysHeld[keycode];
}

bool key_pressed(int keycode)
{
    check_keycode(keycode);

    return keysPressed[keycode] == (curKeyFrame - 1);
}

bool key_released(int keycode)
{
    check_keycode(keycode);

    return keysReleased[keycode] == (curKeyFrame - 1);
}

static void check_keycode(int keycode)
{
    if (keycode >= MAX_KEYS)
    {
        printf("Keycode %d is out of range.\n", keycode);
        printf("Aborting\n");
        exit(1);
    }
}
