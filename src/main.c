#include "main.h"

#include <stdbool.h>

#include <SDL/SDL.h>
#include <SDL/SDL_framerate.h>
#include <SDL/SDL_image.h>

#include "board.h"
#include "boardloader.h"
#include "game.h"
#include "input.h"
#include "intermission.h"
#include "imageloader.h"
#include "menu.h"
#include "sound.h"
#include "text.h"
#include "window.h"

//Initializes all resources.
static void resource_init(void);

//Initialize all the internal entities needed for the game at startup.
static void game_init(void);

//Called when a game is about to begin (player hits enter from main menu).
static void startgame_init(void);

//Frees all resources.
static void clean_up(void);

//Performs a loop, updating and rendering at 60hz.
static void main_loop(void);

//Sleeps until 60hz is reached.
static void internal_fps_sleep(void);

//Defers to appropriate tick, based on current state.
static void internal_tick(void);

//Defers to appropriate render, based on current state.
static void internal_render(void);

//Processess and deals with all SDL events.
static void process_events(void);

//Performs specific actions on various keypresses. Used for testing.
static void key_down_hacks(int keycode);

static ProgramState state;
static MenuSystem menuSystem;
static PacmanGame pacmanGame;

static FPSmanager fpsManager;

static bool gameRunning = true;
static int numCredits = 0;

static int numTicks = 0;

int main(void)
{
    resource_init();
    game_init();

    main_loop();

    clean_up();

    return 0;
}

static void main_loop(void)
{
    while (gameRunning && !key_held(SDLK_ESCAPE))
    {
        process_events();

        internal_tick();
        internal_render();

        internal_fps_sleep();
    }
}

static void internal_fps_sleep(void)
{
    numTicks++;
    SDL_framerateDelay(&fpsManager);
}

static void internal_tick(void)
{
    switch (state)
    {
        case Menu:
            menu_tick(&menuSystem);
            
            if (menuSystem.action == GoToGame)
            {
                state = Game;
                startgame_init();
            }
            
            break;
        case Game:
            game_tick(&pacmanGame);
            
            if (is_game_over(&pacmanGame))
            {
                menu_init(&menuSystem);
                state = Menu;
            }

            break;
        case Intermission:
            intermission_tick();
            break;
    }
}

static void internal_render(void)
{
    clear_screen(0, 0, 0, 0);
    
    switch (state)
    {
        case Menu:
            menu_render(&menuSystem);
            break;
        case Game:
            game_render(&pacmanGame);
            break;
        case Intermission:
            intermission_render();
            break;
    }

    flip_screen();
}

static void game_init(void)
{
    //Load the board here. We only need to do it once
    load_board(&pacmanGame.board, &pacmanGame.pelletHolder, "maps/encodedboard");

    //set to be in menu
    state = Menu;

    //init the framerate manager
    SDL_setFramerate(&fpsManager, key_held(SDLK_SPACE) ? 200 : 60);

    menu_init(&menuSystem);
}

static void startgame_init(void)
{
    gamestart_init(&pacmanGame);
}

static void resource_init(void)
{
    init_window(SCREEN_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT);
    load_images();
    load_sounds();
    load_text();

    //TODO: ensure all the resources loaded properly with some nice function calls
}

static void clean_up(void)
{
    dispose_window();
    dispose_images();
    dispose_sounds();
    dispose_text();

    SDL_Quit();
}

static void process_events(void)
{
    static SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                gameRunning = false;

                break;
            case SDL_KEYDOWN:
                handle_keydown(event.key.keysym.sym);
                key_down_hacks(event.key.keysym.sym);

                break;
            case SDL_KEYUP:
                handle_keyup(event.key.keysym.sym);

                break;
        }
    }

    keyevents_finished();
}

static void key_down_hacks(int keycode)
{
    if (keycode == SDLK_RETURN) pacmanGame.currentLevel++;
    if (keycode == SDLK_BACKSPACE) menuSystem.ticksSinceModeChange = SDL_GetTicks();

    static bool rateSwitch = false;

    //TODO: remove this hack and try make it work with the physics body
    if (keycode == SDLK_SPACE) SDL_setFramerate(&fpsManager, (rateSwitch = !rateSwitch) ? 200 : 60);

    //TODO: move logic into the tick method of the menu
    if (state == Menu && keycode == SDLK_5 && numCredits < 99) 
    {
        numCredits++;
    }
}

int num_credits(void)
{
    return numCredits;
}

unsigned int ticks_game(void)
{
    return SDL_GetTicks();
}

unsigned int ticks_startup()
{
    return SDL_GetTicks();
}

unsigned int frames_game(void)
{
    return numTicks;
}

unsigned int frames_startup(void)
{
    return numTicks;
}

void dir_xy(Direction dir, int *x, int *y)
{
    switch (dir)
    {
        case Up:    { *x =  0; *y = -1; break; } 
        case Down:  { *x =  0; *y =  1; break; }
        case Left:  { *x = -1; *y =  0; break; }
        case Right: { *x =  1; *y =  0; break; }
    }
}

void dir_xy_buggy(Direction dir, int *x, int *y)
{
    switch (dir)
    {
        case Up:    { *x = -1; *y = -1; break; } 
        case Down:  { *x =  0; *y =  1; break; }
        case Left:  { *x = -1; *y =  0; break; }
        case Right: { *x =  1; *y =  0; break; }
    }
}

Direction dir_opposite(Direction dir)
{
    if      (dir == Left)  { return Right; }
    else if (dir == Right) { return Left; }
    else if (dir == Up)    { return Down; }
    else                   { return Up; }
}