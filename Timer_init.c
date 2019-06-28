#include <xc.h>
#include "Timer_init.h"
#include <sys/attribs.h>

/*
 * Created March 20, 2019 by Yousof Al-Autman
 * Segments of this code were taken from Dr.Calvino-Fragas
 * This code allows you to setup various timers at a rate of 0.5s
 * speed_select | timer interrupt
 *      0               0.125s
 *      1               0.25s
 *      2               0.5s
 *      3               1s
 */

void SetupTimer1 (unsigned char speed_select)
{
	// Explanation here:
	// https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
    switch (speed_select){
        case 0: PR1 =(SYSCLK/(FREQ_eighthsecond*256))-1;
        break;
        case 1: PR1 =(SYSCLK/(FREQ_quartersecond*256))-1;
        break;
        case 2: PR1 =(SYSCLK/(FREQ_halfsecond*256))-1;
        break;
        case 3: PR1 =(SYSCLK/(FREQ_second*256))-1;
        break;
                
    }

	TMR1 = 0;
	T1CONbits.TCKPS = 3; // Pre-scaler: 256
	T1CONbits.TCS = 0; // Clock source
	T1CONbits.ON = 1;
	IPC1bits.T1IP = 5;
	IPC1bits.T1IS = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
	
	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void SetupTimer2 (unsigned char speed_select) //Don't call this
{
	// Explanation here:
	// https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
    switch (speed_select){
        case 0: PR1 =(SYSCLK/(FREQ_eighthsecond*256))-1;
        break;
        case 1: PR1 =(SYSCLK/(FREQ_quartersecond*256))-1;
        break;
        case 2: PR1 =(SYSCLK/(FREQ_halfsecond*256))-1;
        break;
        case 3: PR1 =(SYSCLK/(FREQ_second*256))-1;
        break;
                
    }

	TMR2 = 0;
	T2CONbits.TCKPS = 3; // Pre-scaler: 256
	T2CONbits.TCS = 0; // Clock source
	T2CONbits.ON = 1;
	IPC2bits.T2IP = 5;
	IPC2bits.T2IS = 0;
	IFS0bits.T2IF = 0;
	IEC0bits.T2IE = 1;
	
	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}


