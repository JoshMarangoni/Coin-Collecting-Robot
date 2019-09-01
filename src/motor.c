#include "motor.h"
#include "pins.h"
#include <XC.h>

void forward_fast()
{
	white_motor = 0;
	blue_motor = 1;
	black_motor = 1;
	green_motor = 0;
}

void reverse_fast()
{
	white_motor = 1;
	blue_motor = 0;
	black_motor = 0;
	green_motor = 1;
}

void rotateCW_fast()
{
	white_motor = 1;
	blue_motor = 0;
	black_motor = 1;
	green_motor = 0;
}

void rotateCCW_fast()
{
	white_motor = 0;
	blue_motor = 1;
	black_motor = 0;
	green_motor = 1;
}

void off()
{
	white_motor = 1;
	blue_motor = 1;
	black_motor = 1;
	green_motor = 1;
}