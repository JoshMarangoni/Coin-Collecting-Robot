/*
* Main file for robot coin picker.
* Contributors: Joshua Marangoni, Yousof Al-Autman
* Pins 1, 4,5, 16, 20 and 22 are used for communication with the pickit 4
* Pins 13 and 28 are connected to 3.3V, pins 8, 19 and 27 are connected to ground
*/

//#include <stdlib.h>    maybe xc32 already has this header
#include <stdio.h>
//#include <XC.h>
#include <xc.h>
#include "config_bits.h"
#include "motor_control.h"+
#include <attribs.h>


#define SYSCLK 40000000L
#define FREQ 4000000L // We need the ISR for timer 1 every 10 us
#define Baud2BRG(desired_baud)( (SYSCLK / (16*desired_baud))-1)
#define FREQ_second 1L // 1Hz or 1 second interrupt rate
#define FREQ_halfsecond 2L // 2Hz or 0.5 seconds interrupt rate
#define FREQ_quartersecond 4L // 4Hz or 0.25 seconds interrupt rate
#define FREQ_eighthsecond 8L //  8Hz or 0.125 seconds interrupt rate
#define vertical_arm LATBbits.LATB15
#define horizontal_arm LATBbits.LATB14

#define UP_REST 34
#define DOWN_PICK 65
#define PICK_START 30
#define PICK_END 40
#define DROP_REST 29 

void __ISR(_TIMER_1_VECTOR, IPL1SOFT) Timer1_Handler(void);
void __ISR(_TIMER_3_VECTOR, IPL3SOFT) Timer3_Handler(void);
volatile int ISR_pw_horiz;
volatile int ISR_pw_vert;
volatile int motor_select=0;
volatile int ISRT1_cnt = 0;
volatile int ISRT3_cnt = 0;
volatile int arm_select;
volatile int ISRT1_frc =0;
volatile int ISRT3_frc =0;



// The Interrupt Service Routine for timer 1 is used to generate one or more standard
// hobby servo signals.  The servo signal has a fixed period of 20ms and a pulse width
// between 0.6ms and 2.4ms.
void __ISR(_TIMER_1_VECTOR, IPL1SOFT) Timer1_Handler(void)
{
	IFS0CLR=_IFS0_T1IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0
    __builtin_disable_interrupts();

    if(arm_select==2){ //Arm is far left
        ISR_pw_horiz=DROP_REST;
    }
    else if(arm_select==3){
        ISR_pw_horiz=PICK_START;//Arm is mid left
    }
    else{
        ISR_pw_horiz=PICK_END;// Arm is right
    }
	ISRT1_cnt++;
	if(ISRT1_cnt<ISR_pw_horiz)
	{
        switch (arm_select){
            case 2://Move arm left
                horizontal_arm=1;
                vertical_arm=0;
                break;
            case 3://Move the right
                horizontal_arm=1;
                vertical_arm=0;
                break;
            default:
                horizontal_arm=0;
                vertical_arm=0;
           /* case 5:
                horizontal_arm=1;
                vertical_arm=0;
                break;  */   
        }
	}
	else
	{
         switch (arm_select){
            case 2://Move is far left
                horizontal_arm=0;
                vertical_arm=0;
                break;
            case 3://arm is mid left
                horizontal_arm=0;
                vertical_arm=0;
             case 5: //arm is right
                 horizontal_arm=1;
                vertical_arm=0;
            default:
                horizontal_arm=0;
                vertical_arm=0;
            /*case 5:
                horizontal_arm=0;
                vertical_arm=0;
                break;*/
        }
	}
	if(ISRT1_cnt>=2000)
	{
		ISRT1_cnt=0; // 2000 * 10us=20ms
		ISRT1_frc++;
	}
    __builtin_enable_interrupts();
}

void __ISR(_TIMER_3_VECTOR, IPL4SOFT) Timer3_Handler(void)
{
	IFS0CLR=_IFS0_T3IF_MASK; // Clear timer 3 interrupt flag, bit 4 of IFS0
    __builtin_disable_interrupts();
    if (arm_select==1){ //Arm is up
        ISR_pw_vert=UP_REST;
    }
    else if (arm_select==4){
        ISR_pw_vert=DOWN_PICK; //Arm is down
    }
	ISRT3_cnt++;
	if(ISRT3_cnt<ISR_pw_vert)
	{
        switch (arm_select){
            case 1://Move arm up
                horizontal_arm=0;
                vertical_arm=1;
                break;
            case 4://Move the arm down
                horizontal_arm=0;
                vertical_arm=1;
                break;
            default:
                horizontal_arm=0;
                vertical_arm=0;
        }
	}
	else
	{
         switch (arm_select){
            case 1://Move the arm up
                horizontal_arm=0;
                vertical_arm=0;
                break;
            case 4://Move the arm down
                horizontal_arm=0;
                vertical_arm=0;
                break;
            default:
                horizontal_arm=0;
                vertical_arm=0;
        }
	}
	if(ISRT3_cnt>=2000)
	{
		ISRT3_cnt=0; // 2000 * 10us=20ms
		ISRT3_frc++;
	}
    __builtin_enable_interrupts();
}
void SetupTimer1 (void)
{
	// Explanation here: https://www.youtube.com/watch?v=bu6TTZHnMPY
	__builtin_disable_interrupts();
	PR1 =(SYSCLK/FREQ)-1; // since SYSCLK/FREQ = PS*(PR1+1)
	TMR1 = 0;
	T1CONbits.TCKPS = 0; // 3=1:256 prescale value, 2=1:64 prescale value, 1=1:8 prescale value, 0=1:1 prescale value
	T1CONbits.TCS = 0; // Clock source
	T1CONbits.ON = 1;
	IPC1bits.T1IP = 5;
	IPC1bits.T1IS = 0;
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 1;
    
	
	INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void SetupTimer3 (void)
{
__builtin_disable_interrupts();
PR3 =(SYSCLK/FREQ)-1; // since SYSCLK/FREQ = PS*(PR2+1)
TMR3 = 0; // Clear timer register
T3CONbits.TCKPS = 0; 
T3CONbits.TCS = 0; // Clock source
T3CONbits.ON = 1;
IPC3bits.T3IP = 4; // Set Timer 1 Interrupt Priority Level(priority less than timer 1 )
IPC3bits.T3IS = 0;
IFS0bits.T3IF = 0;
IEC0bits.T3IE = 1;
INTCONbits.MVEC = 1; //Int multi-vector
	__builtin_enable_interrupts();
}

void delay_ms (int msecs)
{	
	int ticks;
	ISRT1_frc=0;
	ticks=msecs/20;
	while(ISRT1_frc<ticks);
}
void Drop_2_Pick(void){
    T3CONbits.TON = 0;
    T1CONbits.TON = 1;
    arm_select=3;
    delay_ms(1000);
    T3CONbits.TON = 1;
    T1CONbits.TON = 0;
    arm_select=4;
    delay_ms(1000);
    T3CONbits.TON = 0;
    T1CONbits.TON = 1;
    arm_select=5;
    delay_ms(3000);
}
void Pick_2_Drop(void){
    T3CONbits.TON = 1;
    T1CONbits.TON = 0;
    arm_select=1;
    delay_ms(1000);
    T3CONbits.TON = 0;
    T1CONbits.TON = 1;
    arm_select=2;
    delay_ms(1000);
    
}
void Resting(void){
    T3CONbits.TON = 0;
    T1CONbits.TON = 1;
    arm_select=3;
    delay_ms(1000);
    T3CONbits.TON = 1;
    T1CONbits.TON = 0;
    arm_select=1;
    delay_ms(1000);
    T3CONbits.TON = 0;
    T1CONbits.TON = 1;
    arm_select=2;
    delay_ms(1000);      
}

void main(void){
    char buf[32];
    int pw;
    int i;
	DDPCON = 0;
	for(i = 0; i<10000;i++){
        Nop();
    }
    INTCONbits.MVEC = 1;
    //Configure pins 12, 14, 15 and 16 as outputs
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB10 = 0;
    TRISBbits.TRISB11 = 0;
    TRISBbits.TRISB7 = 0;
    TRISBbits.TRISB15 = 0;
    TRISBbits.TRISB14 = 0;
	// Configure the pin we are using for servo control as output
	LATBbits.LATB15 = 0;
    LATBbits.LATB14 = 0;
    //TRISAbits.TRISA2 = 1;//pin 9 is an input
    //Set  pins 12, 14, 15 and 16 as low initially
	pin15 = 1;	
    pin22 = 1;
    pin21 = 0;
    pin16 = 0;
    SetupTimer1();
    SetupTimer3();
    CFGCON = 0;
    //arm_select=1;
	//SetipTimer2();
    delay_ms(500); // wait 500 ms
    
    while(1){
        arm_select=1;
        off();
         //Drop_2_Pick();
    }
}


