#include "Tank.h"

void Tank::Update(DWORD dt)
{
	x += vx * dt;
	y += vy * dt;

	if (x < 0.0f)
		x = 0.0f;

	if (x > 290.0f)
		x = 290.0f;

	if (y < 0.0f)
		y = 0.0f;

	if (y > 210.0f)
		y = 210.0f;
}

Tank::Tank(float x, float y)
{
	this->x = x;
	this->y = y;

	vx = 0.0f;
	vy = 0.0f;

	Vx = 0.1f;
	Vy = 0.1f;
}