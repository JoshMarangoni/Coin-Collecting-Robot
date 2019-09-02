#include "tune.h"
#include "variables.h"
#include <XC.h>

//PWM values used for speaker
#define BUZZER_PWM 10

void playTune()
{
	T4CONbits.TON = 1;
	V.ISR_pw_buzz = 51;
	F.freq_change = 102;
	delay_ms(175);
	V.ISR_pw_buzz = 38;
	F.freq_change = 77;
	delay_ms(300);
	T4CONbits.TON = 0;
}