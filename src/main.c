#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <SDL/SDL_framerate.h>

#include "main.h"
#include "pacman.h"

#include "input.h"

#include "boardloader.h"

#include "game.h"
#include "menu.h"
#include "intermission.h"

#include "board.h"
#include "window.h"
#include "imageloader.h"
#include "sound.h"
#include "text.h"

//initializes all resources
static void resource_init(void);

//initialize all the internal entities needed for the game at startup
static void game_init(void);

//called when a game is about to begin (player hits enter from main menu)
static void startgame_init(void);

//frees all resources
static void clean_up(void);

//performs a loop, updating and rendering at 60hz
static void main_loop(void);

static void internal_fps_sleep(void);
static void internal_sleep_sleep(void);

//defers to appropriate tick, based on current state
static void internal_tick(void);

//defers to appropriate render, based on current state
static void internal_render(void);

//processess and deals with all SDL events
static void process_events(void);

static ProgramState state;
static MenuSystem menuSystem;
static PacmanGame pacmanGame;

static bool gameRunning = true;
static int numCredits = 0; 

int main(void)
{
    resource_init();
    game_init();

    main_loop();

    clean_up();

    return 0;
}

//TODO move this somewhere
static FPSmanager fpsManager;

static void main_loop(void)
{
    SDL_setFramerate(&fpsManager, key_held(SDLK_SPACE) ? 200 : 60);

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
    static bool rateSwitch = false;

    if (key_pressed(SDLK_SPACE)) SDL_setFramerate(&fpsManager, (rateSwitch = !rateSwitch) ? 200 : 60);
    SDL_framerateDelay(&fpsManager);
}

static void internal_sleep_sleep(void)
{
    unsigned int timeNow = SDL_GetTicks();
    static unsigned int numTicks = 0;

    unsigned int newTimeToWait = timeNow + 16 + ((numTicks % 3 == 0) ? 1 : 0);

    //SDL_Delay promises only at best 10ms granularity
    //the game loop has 16.666 ms to complete, so as long as we complete our 
    //processing + rendering in 6.666 ms, we can be sure we will retain 60 fps.
    //We should take far less, and SDL_Delay should be able to give us more than 10ms
    //Tests had it at at 1-2-3 ms, with 1ms the most common
    //SDL_Delay(10);

    while (SDL_GetTicks() < (newTimeToWait));
    timeNow = SDL_GetTicks();

    numTicks++;

    //printf("fps: %.2f\n", numTicks / ((timeNow - timeStart) / 1000.0f));
}

static void internal_tick(void)
{
    switch (state)
    {
        case Menu:
            menu_tick(&menuSystem);
            
            if (menuSystem.state == GoToGame)
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

    menu_init(&menuSystem);
}

static void startgame_init(void)
{
    //reinitialize the gamestate
    gamestart_init(&pacmanGame);
}

static void resource_init(void)
{
    //load window, images and sound
    init_window(SCREEN_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT);
    load_images();
    load_sounds();
    load_text();

    if (!loaded_succesfully())
    {
        printf("Images were not loaded properly.\n");
        printf("Aborting.\n");
        exit(1);
    }
}

static void clean_up(void)
{
    dispose_window();
    dispose_images();
    dispose_sounds();
    dispose_text();

    //Quit SDL
    SDL_Quit();
}

static void key_down_hacks(int keycode);

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
    //TODO: remove these hacks used for testing
    if (keycode == SDLK_RETURN) pacmanGame.currentLevel++;
    if (keycode == SDLK_BACKSPACE) menuSystem.ticksSinceModeChange = SDL_GetTicks();

    //TODO: create method to check if key pressed/ released this frame
    //and move logic into the tick method
    if (state == Menu && keycode == SDLK_5 && numCredits < 99) 
    {
        numCredits++;
        printf("add credit\n");
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