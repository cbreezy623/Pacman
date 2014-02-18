#include "physics.h"

#include <stdlib.h>

bool collides(PhysicsBody *body1, PhysicsBody *body2)
{
	return body1->x == body2->x && body1->y == body2->y;
}

bool collides_obj(PhysicsBody *body, int otherX, int otherY)
{
	return body->x == otherX && body->y == otherY;
}

bool collides_accurate(PhysicsBody *body1, PhysicsBody *body2)
{
	return body1->xOffset == body2->xOffset && body1->yOffset == body2->yOffset && collides(body1, body2);
}

MovementResult move_ghost(PhysicsBody *body)
{
	//default movement to not changing position
	MovementResult result = SameSquare;

	int xBefore = body->xOffset;
	int yBefore = body->yOffset;

	int xDir = 0;
	int yDir = 0;

	dir_xy(body->curDir, &xDir, &yDir);

	int MULT_VALUE = 4500;

	//our pixel value for the velocity 
	//range between 1800 for v==40, to 4725 for v==105
	int velPixValue = (body->velocity / 100.0f) * MULT_VALUE;

	//add any new speed to current pixel position (only useful for velocity >= 100)
	body->xOffset += (velPixValue / MULT_VALUE) * xDir;
	body->yOffset += (velPixValue / MULT_VALUE) * yDir;

	//add any new carry to current carry (always useful except for velocity == 100)
	body->xOffsetInternal += (velPixValue % MULT_VALUE) * xDir;
	body->yOffsetInternal += (velPixValue % MULT_VALUE) * yDir;

	//add any new pixels we've introduced with our carry
	//this value will be non-zero if we have introduced enough carry
	int offX = body->xOffsetInternal / MULT_VALUE;
	int offY = body->yOffsetInternal / MULT_VALUE;

	//add to our pixel offset any new carry pixels
	body->xOffset += offX;
	body->yOffset += offY;

	//and subtract that carrys pixel from our offsetinternal
	body->xOffsetInternal -= offX * MULT_VALUE;
	body->yOffsetInternal -= offY * MULT_VALUE;

	// we should now have resolved all the values we need to

	int xNow = body->xOffset;
	int yNow = body->yOffset;
	//Now handle if we've moved over center, or into a new square

	bool overCenter = false;
	int overOffset = 0;

	//move over center square
	if ((xBefore < 0 && xNow >= 0) || (xBefore > 0 && xNow <= 0))
	{
		overCenter = true;

		result = OverCenter;

		//transitioned over
		overOffset = abs(body->xOffset) * MULT_VALUE + abs(body->xOffsetInternal);
		
		body->curDir = body->nextDir;
	}
	else if ((yBefore < 0 && yNow >= 0) || (yBefore > 0 && yNow <= 0))
	{
		overCenter = true;

		result = OverCenter;

		//transitioned over
		overOffset = abs(body->xOffset) * MULT_VALUE + abs(body->xOffsetInternal);
		
		body->curDir = body->nextDir;
	}

	if (overCenter)
	{
		dir_xy(body->curDir, &xDir, &yDir);

		body->xOffset = overOffset / MULT_VALUE * xDir;
		body->yOffset = overOffset / MULT_VALUE * yDir;

		body->xOffsetInternal = (overOffset % MULT_VALUE) * xDir;
		body->yOffsetInternal = (overOffset % MULT_VALUE) * yDir;

		return result;
	}

	//check if we've gone onto a new square
	//int low = -8;
	//int high = 7;
	int low = -8;
	int high = 7;
	int offset = 15;

	if (body->xOffset < low) 
	{
		body->xOffset += offset;
		body->x--;

		result = NewSquare;
	} 
	else if (body->xOffset > high)
	{
		body->xOffset -= offset;
		body->x++;

		result = NewSquare;
	} 
	else if (body->yOffset < low) 
	{
		body->yOffset += offset;
		body->y--;

		result = NewSquare;
	} 
	else if (body->yOffset > high)
	{
		body->yOffset -= offset;
		body->y++;

		result = NewSquare;
	}

	return result;
}

MovementResult move_ghost_old(PhysicsBody *body)
{
	MovementResult result = SameSquare;

	int xDir = 0;
	int yDir = 0;

	dir_xy(body->curDir, &xDir, &yDir);

	//int change = body->velocity * 4500 / 10

	//detect if they go over the middle tile

	// +ve -> -ve, -ve -> +ve
	if (xDir)
	{
		int xBefore = body->xOffsetInternal;

		body->xOffsetInternal += xDir * body->velocity * 45;;

		int over = 0;
		if (xBefore < 0 && body->xOffsetInternal >= 0)
		{
			result = OverCenter;

			//transitioned over
			over = body->xOffsetInternal * xDir;
			body->xOffsetInternal = 0;
			body->curDir = body->nextDir;
		}
		else if (xBefore > 0 && body->xOffsetInternal <= 0)
		{
			result = OverCenter;

			over = body->xOffsetInternal * xDir;
			body->xOffsetInternal = 0;
			body->curDir = body->nextDir;
		}

		dir_xy(body->nextDir, &xDir, &yDir);

		body->yOffsetInternal += yDir * over;
		body->xOffsetInternal += xDir * over;
	}
	else
	{
		int yBefore = body->yOffsetInternal;

		body->yOffsetInternal += yDir * body->velocity * 45;

		int over = 0;
		if (yBefore < 0 && body->yOffsetInternal >= 0)
		{
			result = OverCenter;

			//transitioned over
			over = body->yOffsetInternal * yDir;
			body->yOffsetInternal = 0;
			body->curDir = body->nextDir;
		}
		else if (yBefore > 0 && body->yOffsetInternal <= 0)
		{
			result = OverCenter;

			over = body->yOffsetInternal * yDir;
			body->yOffsetInternal = 0;
			body->curDir = body->nextDir;
		}

		dir_xy(body->nextDir, &xDir, &yDir);

		body->yOffsetInternal += yDir * over;
		body->xOffsetInternal += xDir * over;
	}

	
	//body->yOffset += yDir * body->velocity;

	//TODO: see if I can make this more pretty

	//int low = -8;
	//int high = 7;
	int low = -36000;
	int high = 31500;
	int offset = 15 * 75 * 70;

	if (body->xOffsetInternal < low) 
	{
		body->xOffsetInternal += offset;
		body->x--;

		result = NewSquare;
	} 
	else if (body->xOffsetInternal > high)
	{
		body->xOffsetInternal -= offset;
		body->x++;

		result = NewSquare;
	} 
	else if (body->yOffsetInternal < -low) 
	{
		body->yOffsetInternal += offset;
		body->y--;

		result = NewSquare;
	} 
	else if (body->yOffsetInternal > high)
	{
		body->yOffsetInternal -= offset;
		body->y++;

		result = NewSquare;
	}

	body->xOffset = body->xOffsetInternal / (75 * 60);
	body->yOffset = body->yOffsetInternal / (75 * 60);

	return result;
}

bool move_pacman(PhysicsBody *body)
{
	int xDir = 0;
	int yDir = 0;

	dir_xy(body->curDir, &xDir, &yDir);

	body->xOffset += xDir * body->velocity;
	body->yOffset += yDir * body->velocity;

	//TODO: see if I can make this more pretty

	if (body->xOffset < -8) 
	{
		body->xOffset = 7;
		body->x--;

		return true;
	} 
	else if (body->xOffset > 7)
	{
		body->xOffset = -8;
		body->x++;

		return true;
	} 
	else if (body->yOffset < -8) 
	{
		body->yOffset = 7;
		body->y--;

		return true;
	} 
	else if (body->yOffset > 7)
	{
		body->yOffset = -8;
		body->y++;

		return true;
	}

	return false;
}

// Note on center/ horo/ vert functions:
// This pacman game uses 2 pixels for every pixel in the original game.
// This means being in the "center" of a specific axis actually means either of 2 pixel locations
// These functions deal with that

bool on_center(PhysicsBody *body)
{
	return on_horo(body) && on_vert(body);
}

bool on_horo(PhysicsBody *body)
{
	return body->yOffset == 0;// || body->yOffset == 1;
}

bool on_vert(PhysicsBody *body)
{
	return body->xOffset == 0;// || body->xOffset == 1;
}
