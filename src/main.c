#include <XC.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "coin.h"
#include "frequency.h"
#include "motor.h"
#include "perimeter.h"
#include "period.h"
#include "pins.h"
#include "serial.h"
#include "tune.h"
#include "variables.h"
#include "sysclk.h"

//struct DEFINITIONS
varISR V = {0,0,0,0,500,0};
coinCounters C = {0,0.0};
Frequencies F = {0,0.0};

// The Interrupt Service Routine for timer 1 is used to generate one or more standard
// hobby servo signals.  The servo signal has a fixed period of 20ms and a pulse width
// between 0.6ms and 2.4ms.
void __ISR(_TIMER_1_VECTOR, IPL5SOFT) Timer1_Handler()
{
	IFS0CLR = _IFS0_T1IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0

	V.ISR_cnt++;

	if (V.ISR_cnt < V.ISR_pw_v)
	{
		servo1 = 1; //vertical
	}
	else
	{
		servo1 = 0;
	}

	if (V.ISR_cnt < V.ISR_pw_h)
	{
		servo2 = 1; //horizontal
	}
	else
	{
		servo2 = 0;
	}

	if (V.ISR_cnt >= 2000)
	{
		V.ISR_cnt = 0; // 2000 * 10us=20ms
		V.ISR_frc++;
	}
}

//ISR for buzzer
void __ISR(_TIMER_4_VECTOR, IPL5SOFT) Timer4_Handler()
{
	IFS0CLR = _IFS0_T4IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0

	V.ISR_cnt_buzz++;

	if (V.ISR_cnt_buzz < V.ISR_pw_buzz)
	{
		buzzer = 1;
	}
	else
	{
		buzzer = 0;
	}

	if (V.ISR_cnt_buzz >= F.freq_change)
	{
		V.ISR_cnt_buzz = 0; // 2000 * 10us=20ms
	}
}

void SetupTimer1(void)
{
	// Explanation here: https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
	PR1 = (SYSCLK / FREQ) - 1; // since SYSCLK/FREQ = PS*(PR1+1)
	TMR1 = 0;
	T1CONbits.TCKPS = 0; // 3=1:256 prescale value, 2=1:64 prescale value, 1=1:8 prescale value, 0=1:1 prescale value
	T1CONbits.TCS = 0;   // Clock source
	T1CONbits.ON = 1;
	IPC1bits.T1IP = 5;
	IPC1bits.T1IS = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;

	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void SetupTimer4(void)
{
	// Explanation here: https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
	PR4 = (SYSCLK / FREQ) - 1; // since SYSCLK/FREQ = PS*(PR1+1)
	TMR4 = 0;
	T4CONbits.TCKPS = 0; // 3=1:256 prescale value, 2=1:64 prescale value, 1=1:8 prescale value, 0=1:1 prescale value
	T4CONbits.TCS = 0;   // Clock source
	T4CONbits.ON = 1;
	IPC4bits.T4IP = 5;
	IPC4bits.T4IS = 0;
	IFS0bits.T4IF = 0;
	IEC0bits.T4IE = 1;

	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void UART2Configure(int baud_rate) //configure for bluetooth
{
	// Peripheral Pin Select
	U2RXRbits.U2RXR = 4; //SET RX to RB8
	RPB9Rbits.RPB9R = 2; //SET RB9 to TX

	U2MODE = 0;					 // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
	U2STA = 0x1400;				 // enable TX and RX
	U2BRG = Baud2BRG(BAUD_RATE); // U2BRG = (FPb / (16*baud)) - 1
	U2MODESET = 0x8000;			 // enable UART2

	//Putting the UART interrupt flag down.
	IFS1bits.U2RXIF = 0;
}

// Good information about ADC in PIC32 found here:
// http://umassamherstm5.org/tech-tutorials/pic32-tutorials/pic32mx220-tutorials/adc
void ADCConf(void)
{
	AD1CON1CLR = 0x8000; // disable ADC before configuration
	AD1CON1 = 0x00E0;	// internal counter ends sampling and starts conversion (auto-convert), manual sample
	AD1CON2 = 0;		 // AD1CON2<15:13> set voltage reference to pins AVSS/AVDD
	AD1CON3 = 0x0f01;	// TAD = 4*TPB, acquisition time = 15*TAD
	AD1CON1SET = 0x8000; // Enable ADC
}

void delay_ms(int msecs)
{
	int ticks;
	V.ISR_frc = 0;
	ticks = msecs / 20;
	while (V.ISR_frc < ticks)
		;
}

void manual(int direction){
	switch(direction){
		case FORWARD:{
			forward_fast();
			break;
		}
		case BACK:{
			reverse_fast();
			break;
		}
		case LEFT:{
			rotateCCW_fast();
			break;
		}
		case RIGHT:{
			rotateCW_fast();
			break;
		}
		case STOP:{
			off();
			break;
		}
		case COLLECT_COIN:{
			off();
			pick_up_coin();
			break;
		}
	}
}

void main(void)
{	
	DDPCON = 0;
	float f;
	int command = 0; //command from serial
	int coin_counter = 0;
	int robotMode = MANUAL;
	int serialTemp;

	//Motor pins
	TRISBbits.TRISB4 = 0;  //white motor pin
	TRISBbits.TRISB2 = 0;  //blue motor pin
	TRISBbits.TRISB6 = 0;  //black motor pin
	TRISBbits.TRISB10 = 0; //green motor pin
	white_motor = 0;
	blue_motor = 0;
	black_motor = 0;
	green_motor = 0;

	//coin detector - input
	TRISBbits.TRISB1 = 1;

	//speaker pin
	TRISAbits.TRISA2 = 0;
	buzzer = 0;

	//electro magnet
	TRISBbits.TRISB12 = 0;
	emagnet_pin = 1;

	// Configure the pin we are using for servo control as output
	TRISBbits.TRISB13 = 0;  //vertical servo
	TRISBbits.TRISB14 = 0;  //horizontal servo

	INTCONbits.MVEC = 1;

	SetupTimer1();
	SetupTimer4();
	ADCConf();

	CFGCON = 0;
	ANSELB &= ~(1 << 5);	// Set RB5 as a digital I/O
	TRISB |= (1 << 5);		// configure pin RB5 as input
	CNPUB |= (1 << 5);		// Enable pull-up resistor for RB5
	UART2Configure(115200); // Configure UART2 for a baud rate of 115200

	delay_ms(2000);
	//F->air_freq = getFrequency(SYSCLK);

	while (1)
	{
		f = getFrequency(SYSCLK);

		if(coin_counter < 20) 
		{       
			T4CONbits.TON=0;
			f = getFrequency(SYSCLK);

			if(detectCoin(f)==1) {
				reverse_fast();      
				delay_ms(200);
				off();   
				delay_ms(20);
				pick_up_coin();
				playTune();
				coin_counter++;
			}

			else {
				forward_fast();
			}

			if(detectPerimeter()==1) {
				reverse_fast();
				delay_ms(700);
				rotateCCW_fast();
				delay_ms(500);
			}
		}		

		else {    
			serialTemp = SerialReceive();
			manual(serialTemp);
		}
	}
}
