#include "physics.h"

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

bool move(PhysicsBody *body)
{
	int xDir;
	int yDir;

	//TODO: maybe introduce a Point struct and move this logic into the definition for Direction
	if 		(body->dir == Left)  { xDir = -1; yDir =  0; }
	else if (body->dir == Right) { xDir =  1; yDir =  0; }
	else if (body->dir == Up)    { xDir =  0; yDir = -1; }
	else                         { xDir =  0; yDir =  1; }

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