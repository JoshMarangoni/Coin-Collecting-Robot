#include "coin.h"
#include "pins.h"
#include "variables.h"
#include <XC.h>

void pick_up_coin(varISR *V)
{
	V->ISR_pw_h = HORIZONTAL_STRAIGHT;
	delay_ms(300);
	V->ISR_pw_v = VERTICAL_DOWN;
	delay_ms(300);
	emagnet_pin = 0; //magnet on
	V->ISR_pw_h = HORIZONTAL_RIGHT;
	delay_ms(300);
	V->ISR_pw_h = HORIZONTAL_STRAIGHT;
	buzzer = 1;
	delay_ms(300);
	V->ISR_pw_h = HORIZONTAL_LEFT;
	delay_ms(300);
	V->ISR_pw_v = VERTICAL_UP;
	delay_ms(300);
	V->ISR_pw_h = HORIZONTAL_BIN;
	buzzer = 0;
	delay_ms(700);
	V->ISR_pw_v = LOWER_COIN;
	delay_ms(700);
	emagnet_pin = 1; //magnet off
	V->ISR_pw_v = VERTICAL_UP;
	delay_ms(700);
	V->ISR_pw_h = HORIZONTAL_STRAIGHT;
}

int detectCoin(float f, Frequencies *F)
{
	if (f > DETECT_FACTOR * F->air_freq)
	{ //very sensitive threshold
		return 1;
	}
	else
		return 0;
}
