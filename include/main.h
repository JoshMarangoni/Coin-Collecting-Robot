#pragma once
#include "variables.h"

#pragma config FNOSC = FRCPLL   // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2 // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20 // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2 // Divide After PLL (now 40 MHz) see figure 8.1 in datasheet for more info
#pragma config FWDTEN = OFF		// Watchdog Timer Disabled
#pragma config FPBDIV = DIV_1   // PBCLK = SYCLK
#pragma config FSOSCEN = OFF
#pragma config POSCMOD = OFF
#pragma config OSCIOFNC = OFF

#define FREQ 100000L // We need the ISR for timer 1 every 10 us
#define BAUD_RATE 9600L
#define Baud2BRG(desired_baud) ((SYSCLK / (16 * desired_baud)) - 1)

//non-blocking delay function
void delay_ms(int msecs);

//manual mode robot control
void manual(int direction);
