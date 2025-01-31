#include "game.h"

#include "animation.h"
#include "board.h"
#include "fps.h"
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

static bool check_pacghost_collision(PacmanGame *game);     //return true if pacman collided with any ghosts
static void enter_state(PacmanGame *game, GameState state); //transitions to/ from a state
static bool resolve_telesquare(PhysicsBody *body);          //wraps the body around if they've gone tele square

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
			game->frightened = false;
			game->currentlyFrightened = 0;
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

			//TODO: remove this hacks
			if (key_held(SDLK_k)) enter_state(game, DeathState);

			else if (allPelletsEaten) enter_state(game, WinState);
			else if (collidedWithGhost){
				switch(game->recentCollision){
					case NormalCollision:
						enter_state(game, DeathState);
						break;
					default:
						break;
				}
			}

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
	//printf("Frame Offset: %u\n", game->gameFramesOffset);

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

			if (game->frightened && frames_game() - game->eatenInfo.eatenFrame < 120) draw_ghost_pts(&game->eatenInfo);

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
			game->deathsThisLevel = 0;

			break;
		case WinState:
			game->currentLevel++;
			game->deathsThisLevel = 0;
			game->frightenedThisLevel = 0;
			game->gameFramesOffset = frames_game() + 1000; //0 at beginning of level
			game->gameState = LevelBeginState;
			level_init(game);
			break;
		case DeathState:
			// Player died and is starting a new game, subtract a life
			if (state == LevelBeginState)
			{
				game->pacman.livesLeft--;
				game->deathsThisLevel++;
				pacdeath_init(game);
			}
		default: ; //do nothing
	}

	//process entering a state
	switch (state)
	{
		case GameBeginState:
			play_sound(LevelStartSound);
			game->gameFramesOffset = frames_game() + 1000; //0 at beginning of level

			break;
		case LevelBeginState:
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
	if ((dir == Up   || dir == Down ) && !on_vert(&pacman->body)) return false;
	if ((dir == Left || dir == Right) && !on_horo(&pacman->body)) return false;

	//if pacman wants to move on an axis and he is already partially on that axis (not 0)
	//it is always a valid move
	if ((dir == Left || dir == Right) && !on_vert(&pacman->body)) return true;
	if ((dir == Up   || dir == Down ) && !on_horo(&pacman->body)) return true;

	//pacman is at 0/0 and moving in the requested direction depends on if there is a valid tile there
	int x = 0;
	int y = 0;

	dir_xy(dir, &x, &y);

	int newX = pacman->body.x + x;
	int newY = pacman->body.y + y;

	return is_valid_square(board, newX, newY) || is_tele_square(newX, newY);
}

static void update_player_speed(Pacman *pacman, PacmanGame *game){
	pacman->body.velocity = pacman_speed_normal(game->currentLevel);
	//printf("Pacman: %d\n", pacman->body.velocity);
}

static void process_player(PacmanGame *game)
{
	Pacman *pacman = &game->pacman;
	Board *board = &game->board;

	if (pacman->missedFrames != 0)
	{
		pacman->missedFrames--;
		return;
	}

	update_player_speed(pacman, game);

	Direction oldLastAttemptedDir = pacman->lastAttemptedMoveDirection;

	Direction newDir;

	bool dirPressed = dir_pressed_now(&newDir);

	if (dirPressed)
	{
		//user wants to move in a direction
		pacman->lastAttemptedMoveDirection = newDir;

		//if player holds opposite direction to current walking dir
		//we can always just switch current walking direction
		//since we're on parallel line
		if (newDir == dir_opposite(pacman->body.curDir))
		{
			pacman->body.curDir = newDir;
			pacman->body.nextDir = newDir;
		}

		//if pacman was stuck before just set his current direction as pressed
		if (pacman->movementType == Stuck)
		{
			pacman->body.curDir = newDir;
		}

		pacman->body.nextDir = newDir;
	}
	else if (pacman->movementType == Stuck)
	{
		//pacman is stuck and player didn't move - player should still be stuck.
		//don't do anything
		return;
	}
	else
	{
		//user doesn't want to change direction and pacman isn't stuck
		//pacman can move like normal

		//just set the next dir to current dir
		pacman->body.nextDir = pacman->body.curDir;
	}

	pacman->movementType = Unstuck;

	int curDirX = 0;
	int curDirY = 0;
	int nextDirX = 0;
	int nextDirY = 0;

	dir_xy(pacman->body.curDir, &curDirX, &curDirY);
	dir_xy(pacman->body.nextDir, &nextDirX, &nextDirY);

	int newCurX = pacman->body.x + curDirX;
	int newCurY = pacman->body.y + curDirY;
	int newNextX = pacman->body.x + nextDirX;
	int newNextY = pacman->body.y + nextDirY;

	bool canMoveCur =  is_valid_square(board, newCurX, newCurY) || is_tele_square(newCurX, newCurY);
	bool canMoveNext = is_valid_square(board, newNextX, newNextY) || is_tele_square(newNextX, newNextY);

	//if pacman is currently on a center tile and can't move in either direction
	//don't move him
	if (on_center(&pacman->body) && !canMoveCur && !canMoveNext)
	{
		pacman->movementType = Stuck;
		pacman->lastAttemptedMoveDirection = oldLastAttemptedDir;

		return;
	}

	move_pacman(&pacman->body, canMoveCur, canMoveNext);

	//if pacman is on the center, and he couldn't move either of  his last directions
	//he must be stuck now
	if (on_center(&pacman->body) && !canMoveCur && !canMoveNext)
	{
		pacman->movementType = Stuck;
		return;
	}

	resolve_telesquare(&pacman->body);
}

static void normal_ghost_movement(Ghost *g, PacmanGame *game){
	unsigned int currFrame = frames_game();
	unsigned int frameDiff = currFrame - game->gameFramesOffset;
	
	//fixes diff and offset if signed value f frameDiff expression is negative
	if(game->gameFramesOffset > currFrame){
		frameDiff = 0;
		game->gameFramesOffset = currFrame;
	}
	else{
		frameDiff -= 350 * game->deathsThisLevel;
	}

	unsigned int frameRate = 60;
	unsigned int frameRemoveFright = frameDiff -
		frameRate * game->frightenedThisLevel * fright_time(game->currentLevel);
	

	//these values should be updated to more accurately reflect the time ghosts
	//spend in the pen
	if(g->movementMode == InPen){
		switch(g->ghostType){
			case Blinky:
				g->movementMode = LeavingPen;
				return;
				break;
			case Inky:
				if(frameDiff < 7 * frameRate)
					return;
				else{
					g->movementMode = LeavingPen;
					return;
				}
				break;
			case Clyde:
				if(frameDiff < 15 * frameRate)
					return;
				else{
					g->movementMode = LeavingPen;
					return;
				}
				break;
			case Pinky:
				if(frameDiff < 2 * frameRate)
					return;
				else{
					g->movementMode = LeavingPen;
					return;
				}
				break;
			default:
				break;
		}
	}

	if(game->currentLevel == 1){
		if(frameRemoveFright <= 7 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 27 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 34 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 54 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 59 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 79 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 84 * frameRate){g->movementMode = Scatter;}
		else{g->movementMode = Chase;}
	}
	else if(game->currentLevel < 5){
		if(frameRemoveFright <= 7 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 27 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 34 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 54 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 59 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 1092 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 1092 * frameRate + 1){g->movementMode = Scatter;}
		else{g->movementMode = Chase;}
	}
	else{
		if(frameRemoveFright <= 5 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 20 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 25 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 45 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 50 * frameRate){g->movementMode = Scatter;}
		else if(frameRemoveFright <= 1087 * frameRate){g->movementMode = Chase;}
		else if(frameRemoveFright <= 1087 * frameRate + 1){g->movementMode = Scatter;}
		else{g->movementMode = Chase;}
	}
}

static void update_ghost_movement(Ghost *g, PacmanGame *game){
		normal_ghost_movement(g, game);
}

static void update_ghost_speed(Ghost *g, PacmanGame *game){
	if(g->movementMode == Chase || g->movementMode == Scatter){
		if((g->body.x < 4 || g->body.x > 24) && g->body.y == 14){
			g->body.velocity = ghost_speed_tunnel(game->currentLevel);
		}
		else{
			g->body.velocity = ghost_speed_normal(game->currentLevel);
		}
	}
	else if(g->movementMode == Eaten){
		g->body.velocity = 200;
	}
	else if(g->movementMode == InPen){
		g->body.velocity = 20;
	}
	else{
		g->body.velocity = ghost_speed_fright(game->currentLevel);
	}

	//printf("Ghost %d\n", g->body.velocity);
}

static void process_ghosts(PacmanGame *game)
{
	for (int i = 0; i < 4; i++)
	{
		Ghost *g = &game->ghosts[i];
		update_ghost_speed(g, game);

		//if(g->ghostType == Inky && g->movementMode == LeavingPen)
			//printf("%d %d %d %d %d\n", g->body.x, g->body.y, g->body.xOffset, g->body.yOffset, g->movementMode == LeavingPen);
		
		if(g->movementMode == Frightened || g->movementMode == Eaten){
			if(frames_game() >= 60 * fright_time(game->currentLevel) + game->frightenedStart){
				if(g->movementMode != Eaten)
					update_ghost_movement(g, game);
				
				game->frightened = false;
				game->currentlyFrightened = 0;
			}
		}
		else if(g->movementMode != LeavingPen){
			update_ghost_movement(g, game);
		}

		if (g->movementMode == InPen)
		{
			move_ghost(&g->body);
			if(g->body.y < 14){
				g->body.nextDir = Down;
				g->body.curDir = Down;
			}
			else if(g->body.y > 14){
				g->body.nextDir = Up;
				g->body.curDir = Up;
			}

			g->body.xOffset = -8;

			continue;
		}

		if (g->movementMode == LeavingPen)
		{
			//ghost is in center of tile
			//move em to the center of the pen (in x axis)
			//then more em up out the gate
			//when they are out of the gate, set them to be in normal chase mode then set them off on their way

			//if(g->ghostType == Inky && g->movementMode == LeavingPen)
				//printf("%d %d %d %d %d\n", g->body.x, g->body.y, g->body.xOffset, g->body.yOffset, g->movementMode == LeavingPen);
			//ghost has left the pen
			if(g->body.y < 13 && g->body.yOffset < 1){
				update_ghost_movement(g, game);
				continue;
			}

			switch(g->ghostType){
				case Pinky:
					g->body.curDir = Up;
					g->body.nextDir = Up;
					move_ghost(&g->body);
					g->body.xOffset = -8;
					break;
				case Inky:
					if(g->body.y > 14){
						g->body.curDir = Up;
						g->body.nextDir = Up;
						move_ghost(&g->body);
					}
					else if(g->body.y < 14 && g->body.x < 14){
						g->body.curDir = Down;
						g->body.nextDir = Down;
						move_ghost(&g->body);
					}
					else if(g->body.x < 14){
						g->body.curDir = Right;
						g->body.nextDir = Right;
						move_ghost(&g->body);
					}
					else{
						g->body.curDir = Up;
						g->body.nextDir = Up;
						move_ghost(&g->body);
						g->body.xOffset = -8;
					}

					break;
				case Clyde:
					if(g->body.y > 14){
						g->body.curDir = Up;
						g->body.nextDir = Up;
						move_ghost(&g->body);
					}
					else if(g->body.y < 14 && g->body.x > 14){
						g->body.curDir = Down;
						g->body.nextDir = Down;
						move_ghost(&g->body);
					}
					else if(g->body.x > 14 || (g->body.x == 14 && g->body.xOffset > -8)){
						g->body.curDir = Left;
						g->body.nextDir = Left;
						move_ghost(&g->body);
					}
					else{
						g->body.curDir = Up;
						g->body.nextDir = Up;
					    move_ghost(&g->body);
						g->body.xOffset = -8;
					}
					break;
				default:
					g->body.curDir = Up;
					g->body.nextDir = Up;
					move_ghost(&g->body);
					break;
			}
			continue;
		}

		if (g->movementMode == Eaten){
			if(g->body.x == 14 && (g->body.y >= 11 && g->body.y < 14)){
				g->body.nextDir = Down;
				g->body.curDir = Down;
			}
			else if(g->body.x == 14 && g->body.y == 14){
				g->movementMode = LeavingPen;
				g->body.velocity = ghost_speed_tunnel(game->currentLevel);
			}
		}

		//all other modes can move normally (I think)
		MovementResult result = move_ghost(&g->body);
		resolve_telesquare(&g->body);

		if (result == NewSquare)
		{
			//if they are in a new tile, rerun their target update logic
			execute_ghost_logic(g, g->ghostType, &game->ghosts[0], &game->pacman);

			g->nextDirection = next_direction(g, &game->board, g->newlyFrightened);
			g->newlyFrightened = false;
		}
		else if (result == OverCenter)
		{
			//they've hit the center of a tile, so change their current direction to the new direction
			g->body.curDir = g->transDirection;
			g->body.nextDir = g->nextDirection;
			g->transDirection = g->nextDirection;
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

static void frightened_set(PacmanGame *game, Pellet *p){
	if(p->type == LargePellet){
		game->frightened = true;
		game->frightenedThisLevel = game->frightenedThisLevel + 1;
		game->frightenedStart = frames_game();
		game->currentlyFrightened = 4;

		for(int i = 0; i < 4; i++){
			Ghost *g = &game->ghosts[i];
			if(g->movementMode != Eaten && g->movementMode != InPen){
				g->movementMode = Frightened;
				g->newlyFrightened = true;
				//g->nextDirection = dir_opposite(g->body.curDir); //doesn't work on corners
			}
		}
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
			frightened_set(game, p);
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

static int ghost_points(PacmanGame *game){
	switch(game->currentlyFrightened){
		case 4: {return 200; break;}
		case 3: {return 400; break;}
		case 2: {return 800; break;}
		case 1: {return 1600; break;}
		default: break;
	}

	return 0;
}

static bool check_pacghost_collision(PacmanGame *game)
{
	for (int i = 0; i < 4; i++)
	{
		Ghost *g = &game->ghosts[i];

		if (collides(&game->pacman.body, &g->body)){
			if(g->movementMode == Frightened){
				game->recentCollision = FrightenedCollision;
				g->movementMode = Eaten;
				int points = ghost_points(game);
				game->pacman.score += points;
				game->eatenInfo = (EatenInfo) {g->body.x, g->body.y, 4 - game->currentlyFrightened, frames_game()};
				game->currentlyFrightened--;
			}
			else if(g->movementMode == Eaten)
				return false;
			else
				game->recentCollision = NormalCollision;
			return true;
		}
	}

	return false;
}

void gamestart_init(PacmanGame *game)
{
	level_init(game);

	pacman_init(&game->pacman);
	//we need to reset all fruit
	//fuit_init();
	game->highscore = 0; //TODO maybe load this in from a file..?
	game->currentLevel = 1;
	game->frightenedThisLevel = 0;
	game->currentlyFrightened = 0;
	game->frightened = false;

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

void pacdeath_init(PacmanGame *game)
{
	pacman_level_init(&game->pacman);
	ghosts_init(game->ghosts);

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
    if (x >= 10000000)   return 8;
    if (x >= 1000000)    return 7;
    if (x >= 100000)     return 6;
    if (x >= 10000)      return 5;
    if (x >= 1000)       return 4;
    if (x >= 100)        return 3;
    if (x >= 10)         return 2;
    return 1;
}

static bool resolve_telesquare(PhysicsBody *body)
{
	//TODO: chuck this back in the board class somehow

	if (body->y != 14) return false;

	if (body->x == -1) { body->x = 27; body->y = 14; return true; }
	if (body->x == 28) { body->x =  0; body->y = 14; return true; }

	return false;
}
