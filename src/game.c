#include "game.h"

#include "animation.h"
#include "board.h"
#include "input.h"
#include "main.h"
#include "pacman.h"
#include "pellet.h"
#include "physics.h"
#include "renderer.h"
#include "sound.h"
#include "text.h"
#include "window.h"

static void process_player(PacmanGame *game);
static void process_fruit(PacmanGame *game);
static void process_ghosts(PacmanGame *game);
static void process_pellets(PacmanGame *game);
static bool check_pacghost_collision(PacmanGame *game);			//return true if pacman collided with any ghosts
static void enter_state(PacmanGame *game, GameState state);
static bool resolve_telesquare(PhysicsBody *body);		//wraps the body around if they've gone tele square

void game_tick(PacmanGame *game)
{
	unsigned dt = ticks_game() - game->ticksSinceModeChange;

	switch (game->gameState)
	{
		case GameBeginState:
			// plays the sound, has the "player 1" image, has the "READY!" thing

			break;
		case LevelBeginState:
			// similar to game begin mode except no sound, no "Player 1", and slightly shorter duration

			break;
		case GamePlayState:
			// everyone can move and this is the standard 'play' game mode
			process_player(game);
			process_ghosts(game);

			process_fruit(game);
			process_pellets(game);


			if (game->pacman.score > game->highscore) game->highscore = game->pacman.score;

			break;
		case WinState:
			//pacman eats last pellet, immediately becomes full circle
			//everything stays still for a second or so
			//monsters + pen gate disappear
			//level flashes between normal color and white 4 times
			//screen turns dark (pacman still appears to be there for a second before disappearing)
			//full normal map appears again
			//pellets + ghosts + pacman appear again
			//go to start level mode

			if (dt > 2000)
			{
				
				
			}

			break;

		case DeathState:
			// pacman has been eaten by a ghost and everything stops moving
			// he then does death animation and game starts again

			//everything stops for 1ish second

			//ghosts disappear
			//death animation starts
			//empty screen for half a second


			break;

		case GameoverState:
			// pacman has lost all his lives
			//it displays "game over" briefly, then goes back to main menu
			break;
	}

	//
	// State Transitions - refer to gameflow for descriptions
	//

	bool allPelletsEaten = game->pelletHolder.numLeft == 0;
	bool collidedWithGhost = check_pacghost_collision(game);
	int lives = game->pacman.livesLeft;

	switch (game->gameState)
	{
		case GameBeginState:
			if (dt > 2200) enter_state(game, LevelBeginState);
			
			break;
		case LevelBeginState:
			if (dt > 1800) enter_state(game, GamePlayState);

			break;
		case GamePlayState:
			if (key_held(SDLK_0)) enter_state(game, DeathState);

			else if (allPelletsEaten) enter_state(game, WinState);
			else if (collidedWithGhost) enter_state(game, DeathState); 
			
			break;
		case WinState:
			//if (transitionLevel) //do transition here
			if (dt > 4000) enter_state(game, LevelBeginState);

			break;
		case DeathState:
			if (dt > 4000)
			{
				if (lives == 0) enter_state(game, GameoverState);
				else enter_state(game, LevelBeginState);
			}

			break;
		case GameoverState:
			if (dt > 2000) 
			{
				//TODO: go back to main menu

			} 
			break;
	}

}

void game_render(PacmanGame *game)
{
	unsigned dt = ticks_game() - game->ticksSinceModeChange;

	//common stuff that is rendered in every mode:
	// 1up + score, highscore, base board, lives, small pellets, fruit indicators
	draw_common_oneup(true, game->pacman.score);
	draw_common_highscore(game->highscore);
	
	draw_pacman_lives(game->pacman.livesLeft);
	
	draw_small_pellets(&game->pelletHolder);
	draw_fruit_indicators(game->currentLevel);

	//in gameover state big pellets don't render
	//in gamebegin + levelbegin big pellets don't flash
	//in all other states they flash at normal rate

	switch (game->gameState)
	{
		case GameBeginState:
			draw_game_playerone_start();
			draw_game_ready();

			draw_large_pellets(&game->pelletHolder, false);
			draw_board(&game->board);
			break;
		case LevelBeginState:
			draw_game_ready();

			//we also draw pacman and ghosts (they are idle currently though)
			draw_pacman_static(&game->pacman);
			for (int i = 0; i < 4; i++) draw_ghost(&game->ghosts[i]);

			draw_large_pellets(&game->pelletHolder, false);
			draw_board(&game->board);
			break;
		case GamePlayState:
			draw_large_pellets(&game->pelletHolder, true);
			draw_board(&game->board);


			if (game->gameFruit1.fruitMode == Displaying) draw_fruit_game(game->currentLevel);
			if (game->gameFruit2.fruitMode == Displaying) draw_fruit_game(game->currentLevel);

			if (game->gameFruit1.eaten && ticks_game() - game->gameFruit1.eatenAt < 2000) draw_fruit_pts(&game->gameFruit1);
			if (game->gameFruit2.eaten && ticks_game() - game->gameFruit2.eatenAt < 2000) draw_fruit_pts(&game->gameFruit2);
			
			draw_pacman(&game->pacman);

			for (int i = 0; i < 4; i++) draw_ghost(&game->ghosts[i]);
			break;
		case WinState:
			draw_pacman_static(&game->pacman);

			if (dt < 2000)
			{
				for (int i = 0; i < 4; i++) draw_ghost(&game->ghosts[i]);
				draw_board(&game->board);
			}
			else
			{
				//stop rendering the pen, and do the flash animation
				draw_board_flash(&game->board);
			}

			break;
		case DeathState:
			//draw everything the same for 1ish second
			if (dt < 1000)
			{
				//draw everything normally

				//TODO: this actually draws the last frame pacman was on when he died
				draw_pacman_static(&game->pacman);

				for (int i = 0; i < 4; i++) draw_ghost(&game->ghosts[i]);
			}
			else
			{
				//draw the death animation
				draw_pacman_death(&game->pacman, dt - 1000);
			}


			draw_large_pellets(&game->pelletHolder, true);
			draw_board(&game->board);
			break;
		case GameoverState:
			draw_game_gameover();
			draw_board(&game->board);
			draw_credits(num_credits());
			break;
	}
}

static void enter_state(PacmanGame *game, GameState state)
{	
	//process leaving a state
	switch (game->gameState)
	{
		case GameBeginState:
			game->pacman.livesLeft--;

			break;

		case WinState:
			game->currentLevel++;
			game->gameState = LevelBeginState;

			break;
		case DeathState:
			// Player died and is starting a new game, subtract a life
			if (state == LevelBeginState)
			{	
				game->pacman.livesLeft--;
			}
		default: ; //do nothing
	}

	//process entering a state
	switch (state)
	{
		case GameBeginState:
			play_sound(LevelStartSound);
			
			break;
		case LevelBeginState:
			level_init(game);
			break;
		case GamePlayState:
			break;
		case WinState:

			break;
		case DeathState:
			break;
		case GameoverState:
			break;
	}

	game->ticksSinceModeChange = ticks_game();
	game->gameState = state;
}

//checks if it's valid that pacman could move in this direction at this point in time
bool can_move(Pacman *pacman, Board *board, Direction dir)
{
	//easy edge cases, tile has to be parallal with a direction to move it
	if ((dir == Up   || dir == Down ) && pacman->body.xOffset != 0) return false;
	if ((dir == Left || dir == Right) && pacman->body.yOffset != 0) return false;

	//if pacman wants to move on an axis and he is already partially on that axis (not 0)
	//it is always a valid move
	if ((dir == Left || dir == Right) && pacman->body.xOffset != 0) return true;
	if ((dir == Up   || dir == Down ) && pacman->body.yOffset != 0) return true;

	//pacman is at 0/0 and moving in the requested direction depends on if there is a valid tile there
	int x = 0;
	int y = 0;

	dir_xy(dir, &x, &y);

	int newX = pacman->body.x + x;
	int newY = pacman->body.y + y;

	return is_valid_square(board, newX, newY) || is_tele_square(newX, newY);
}

static void process_player(PacmanGame *game)
{
	Pacman *pacman = &game->pacman;
	Board *board = &game->board;
	
	Direction newDir;

	int x = 0;
	int y = 0;

	//first do pacmans missed frames
	if (pacman->missedFrames != 0)
	{
		pacman->missedFrames--;
		return;
	}

	if 		(dir_key_held(Left)) 	{ x = -1; newDir = Left;  }
	else if (dir_key_held(Right))	{ x =  1; newDir = Right; }
	else if (dir_key_held(Up))		{ y = -1; newDir = Up; 	  }
	else if (dir_key_held(Down))	{ y =  1; newDir = Down;  }

	bool dirPressed = x != 0 || y != 0;

	//the game remembers what the last direction the player pressed was
	//if pacman gets into a situation in which he is stuck, the currently facing direction becomes the 
	//last pressed direction (weird behaviour, but let's roll with it)
	if (dirPressed && pacman->movementType != Stuck)
	{
		// TODO, fix this up
		pacman->lastAttemptedMoveDirection = newDir;
	}

	//pacman will move in whatever direction he is currently in, until he hits a wall
	//if the play is holding a button down at the same point in time in which pacman can move in a new direction
	//he moves that way, and that new direction becomes the currently moved direction
	//printf("pacman: (%d, %d)  (%d, %d) \n", pacman->x, pacman->y, pacman->xTileOffset, pacman->yTileOffset);
	
	//if user has a dir pressed, try move pacman in the direction the user has selected
	if (dirPressed && can_move(pacman, board, newDir))
	{
		//printf("valid move\n");
		//just replace pacmans current direction
		pacman->body.dir = newDir;
		pacman->movementType = Unstuck;
	}
	else if (can_move(pacman, board, pacman->body.dir))
	{
		pacman->movementType = Unstuck;		
	}
	else
	{
		pacman->movementType = Stuck;
		//user hasn't moved in a valid direction or pacman can't move in his default direction
		return;
	}

	move(&pacman->body);
	resolve_telesquare(&pacman->body);
}

static void process_ghosts(PacmanGame *game)
{
	for (int i = 0; i < 4; i++)
	{
		Ghost *g = &game->ghosts[i];

		if (g->movementMode == InPen)
		{
			//ghosts bob up and down - move in direction. If they hit a square, change direction
			bool moved = move(&g->body);

			if (moved && (g->body.y == 13 || g->body.y == 15))
			{
				g->body.dir = dir_opposite(g->body.dir);
			}

			continue;
		}

		if (g->movementMode == LeavingPen)
		{
			//ghost is in center of tile
			//move em to the center of the pen (in x axis)
			//then more em up out the gate
			//when they are out of the gate, set them to be in normal chase mode then set them off on their way

			continue;
		}

		//all other modes can move normally (I think,)

		if (g->body.xOffset == 0 && g->body.yOffset == 0)
		{
			//they've hit the center of a tile, so change their current direction to the new direction
			g->body.dir = g->transDirection;
			g->transDirection = g->nextDirection;
		}
		
		bool changedTile;

		changedTile = move(&g->body);
		resolve_telesquare(&g->body);

		if (changedTile)
		{
			//if they are in a new tile, rerun their target update logic
			execute_ghost_logic(g, g->ghostType, &game->ghosts[0], &game->pacman);

			g->nextDirection = next_direction(g, &game->board);
		}
	}
}

static void process_fruit(PacmanGame *game)
{
	int pelletsEaten = game->pelletHolder.totalNum - game->pelletHolder.numLeft;

	GameFruit *f1 = &game->gameFruit1;
	GameFruit *f2 = &game->gameFruit2;

	int curLvl = game->currentLevel;

	if (pelletsEaten >= 70 && f1->fruitMode == NotDisplaying)
	{
		f1->fruitMode = Displaying;
		regen_fruit(f1, curLvl);
	}
	else if (pelletsEaten == 170 && f2->fruitMode == NotDisplaying)
	{
		f2->fruitMode = Displaying;
		regen_fruit(f2, curLvl);
	}

	unsigned int f1dt = ticks_game() - f1->startedAt;
	unsigned int f2dt = ticks_game() - f2->startedAt;

	Pacman *pac = &game->pacman;

	if (f1->fruitMode == Displaying)
	{
		//printf("f1 x=%d, y=%d\n", f1->x, f1->y);
		//printf("pac x=%d, y=%d\n", pac->x, pac->y);
		if (f1dt > f1->displayTime) f1->fruitMode = Displayed;
	}

	if (f2->fruitMode == Displaying)
	{
		if (f2dt > f2->displayTime) f2->fruitMode = Displayed;
	}

	//check for collisions

	if (f1->fruitMode == Displaying && collides_obj(&pac->body, f1->x, f1->y))
	{
		f1->fruitMode = Displayed;
		f1->eaten = true;
		f1->eatenAt = ticks_game();
		pac->score += fruit_points(f1->fruit);
	}

	if (f2->fruitMode == Displaying && collides_obj(&pac->body, f2->x, f2->y))
	{
		f2->fruitMode = Displayed;
		f2->eaten = true;
		f2->eatenAt = ticks_game();
		pac->score += fruit_points(f2->fruit);
	}
}

static void process_pellets(PacmanGame *game)
{
	//if pacman and pellet collide
	//give pacman that many points
	//set pellet to not be active
	//decrease num of alive pellets
	PelletHolder *holder = &game->pelletHolder;

	for (int i = 0; i < holder->totalNum; i++)
	{
		Pellet *p = &holder->pellets[i];

		//skip if we've eaten this one already
		if (p->eaten) continue;

		if (collides_obj(&game->pacman.body, p->x, p->y))
		{
			holder->numLeft--;
			
			p->eaten = true;
			game->pacman.score += pellet_points(p);

			//play eat sound

			//eating a small pellet makes pacman not move for 1 frame
			//eating a large pellet makes pacman not move for 3 frames
			game->pacman.missedFrames = pellet_nop_frames(p);

			//can only ever eat 1 pellet in a frame, so return
			return;
		}
	}

	//maybe next time, poor pacman
}

static bool check_pacghost_collision(PacmanGame *game)
{
	for (int i = 0; i < 4; i++)
	{
		Ghost *g = &game->ghosts[i];

		if (collides(&game->pacman.body, &g->body)) return true;
	}

	return false;
}

void gamestart_init(PacmanGame *game)
{
	level_init(game);

	//we need to reset all fruit
	//fuit_init();
	game->highscore = 0;	//TODO maybe load this in from a file..?
	game->currentLevel = 1;

	//invalidate the state so it doesn't effect the enter_state function
	game->gameState = -1;
	enter_state(game, GameBeginState);
}

void level_init(PacmanGame *game)
{
	//reset pacmans position
	pacman_level_init(&game->pacman);

	//reset pellets
	pellets_init(&game->pelletHolder);

	//reset ghosts
	ghosts_init(game->ghosts);

	//reset fruit
	reset_fruit(&game->gameFruit1);
	reset_fruit(&game->gameFruit2);
}


//TODO: make this method based on a state, not a conditional
//or make the menu system the same. Just make it consistant
bool is_game_over(PacmanGame *game)
{
	unsigned dt = ticks_game() - game->ticksSinceModeChange;

	return dt > 2000 && game->gameState == GameoverState;
}

int int_length(int x)
{
    if (x >= 1000000000) return 10;
    if (x >= 100000000)  return 9;
    if (x >= 10000000) 	 return 8;
    if (x >= 1000000) 	 return 7;
    if (x >= 100000) 	 return 6;
    if (x >= 10000) 	 return 5;
    if (x >= 1000) 		 return 4;
    if (x >= 100) 		 return 3;
    if (x >= 10) 		 return 2;
    return 1;
}


static bool resolve_telesquare(PhysicsBody *body)
{
	//TODO: chuck this back in the board class somehow

	if (body->y != 14) return false;
	
	if (body->x == -1) { body->x = 27; return true; }
	if (body->x == 28) { body->x =  0; return true; }

	return false;
}