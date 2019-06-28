#include <XC.h>
#include <sys/attribs.h>
#include <stdio.h>
#include <stdlib.h>

#pragma config FNOSC = FRCPLL   // Internal Fast RC oscillator (8 MHz) w/ PLL
#pragma config FPLLIDIV = DIV_2 // Divide FRC before PLL (now 4 MHz)
#pragma config FPLLMUL = MUL_20 // PLL Multiply (now 80 MHz)
#pragma config FPLLODIV = DIV_2 // Divide After PLL (now 40 MHz) see figure 8.1 in datasheet for more info
#pragma config FWDTEN = OFF		// Watchdog Timer Disabled
#pragma config FPBDIV = DIV_1   // PBCLK = SYCLK
#pragma config FSOSCEN = OFF
#pragma config POSCMOD = OFF
#pragma config OSCIOFNC = OFF

#define SYSCLK 40000000L
#define FREQ 100000L // We need the ISR for timer 1 every 10 us
#define BAUD_RATE 9600L
#define Baud2BRG(desired_baud) ((SYSCLK / (16 * desired_baud)) - 1)

//constants used defining incoming serial data
#define NO_DATA 0
#define FORWARD 1
#define LEFT 2
#define BACK 3
#define RIGHT 4
#define COLLECT_COIN 5
#define AUTONOMOUS 6
#define MANUAL 7
#define SCRATCH 8
#define STOP 9

//constants used for defining coin picking arm servo motor positions
#define VERTICAL_DOWN 178
#define VERTICAL_UP 65
#define LOWER_COIN 100
#define HORIZONTAL_STRAIGHT 120
#define HORIZONTAL_RIGHT 200
#define HORIZONTAL_LEFT 60
#define HORIZONTAL_BIN 190

//perimeter detector and ADC constants
#define REF_VOLTAGE 3.3
#define PERIMETER_PIN1 5 //pin7
#define PERIMETER_PIN2 9 //pin26
#define THRESHOLD_VOLTAGE 0.3
#define OTHER_THRESHOLD 0.2

//coin detector frequency multiplier
#define DETECT_FACTOR 1.0078 //1.001 this might change with battery level
#define DIME_FACTOR 1.009110138//1.5025
#define NICKEL_FACTOR 1.018747873
#define TOONIE_FACTOR 1.018843976//1.009
#define LOONIE_FACTOR 1.018435686 //1.0083
#define QUARTER_FACTOR 1.016470676// 1.008

#define TOONIE_F 47168.699219
#define DIME_F 46718.058594
#define NICKEL_F 47164.250000
#define LOONIE_F 47149.796875
#define QUARTER_F 47058.824219
#define AIR_F 46296.292969

//PWM values used for speaker
#define BUZZER_PWM 10

//pin assignments
#define coin_pin LATBbits.LATB5		//pin14
#define buzzer LATAbits.LATA2		//pin 9 buzzer
#define white_motor LATBbits.LATB4  //pin11 - right wheel
#define blue_motor LATBbits.LATB2   //pin6 - right wheel
#define black_motor LATBbits.LATB6  //pin15 - left wheel
#define green_motor LATBbits.LATB10 //pin21 - left wheel
#define emagnet_pin LATBbits.LATB12 //pin23
#define servo1 LATBbits.LATB13		//pin24, vertical
#define servo2 LATBbits.LATB14		//pin25, horizontal
#define buzzer LATAbits.LATA2		//pin9

//motor functions
void autonomous(int* coin_counter);
void manual(int direction);
void off();			   //motor_select=0
void forward_fast();   //motor_select=1
void reverse_fast();   //motor_select=2
void rotateCW_fast();  //motor_select=3
void rotateCCW_fast(); //motor_select=4

//coin collecting functions
int detectCoin(float);
void pick_up_coin(void);
long int GetPeriod(int n);
float getFrequency();
void determineCoin(float);

//perimeter functions
int detectPerimeter();
void ADCConf(void);
int ADCRead(char analogPIN);
float ADCVoltage(char analogPIN);

//bluetooth communication functions
int SerialReceive();

void playTune();

//Global variables
volatile int ISR_pw_h, ISR_pw_v, ISR_cnt = 0, ISR_frc = 0;
volatile int ISR_pw_buzz = 500, ISR_cnt_buzz = 0;
volatile float myWallet = 0;
volatile int freq_change;
volatile float air_freq;
volatile int dime_counter=0, nickel_counter=0, quarter_counter=0, loonie_counter=0, toonie_counter=0, coin_counter=0;

// The Interrupt Service Routine for timer 1 is used to generate one or more standard
// hobby servo signals.  The servo signal has a fixed period of 20ms and a pulse width
// between 0.6ms and 2.4ms.
void __ISR(_TIMER_1_VECTOR, IPL5SOFT) Timer1_Handler(void)
{
	IFS0CLR = _IFS0_T1IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0

	ISR_cnt++;

	if (ISR_cnt < ISR_pw_v)
	{
		servo1 = 1; //vertical
	}
	else
	{
		servo1 = 0;
	}

	if (ISR_cnt < ISR_pw_h)
	{
		servo2 = 1; //horizontal
	}
	else
	{
		servo2 = 0;
	}

	if (ISR_cnt >= 2000)
	{
		ISR_cnt = 0; // 2000 * 10us=20ms
		ISR_frc++;
	}
}

void __ISR(_TIMER_4_VECTOR, IPL5SOFT) Timer4_Handler(void)
{
	IFS0CLR = _IFS0_T4IF_MASK; // Clear timer 1 interrupt flag, bit 4 of IFS0

	ISR_cnt_buzz++;

	if (ISR_cnt_buzz < ISR_pw_buzz)
	{
		buzzer = 1;
	}
	else
	{
		buzzer = 0;
	}

	if (ISR_cnt_buzz >= freq_change)
	{
		ISR_cnt_buzz = 0; // 2000 * 10us=20ms
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

void delay_ms(int msecs)
{
	int ticks;
	ISR_frc = 0;
	ticks = msecs / 20;
	while (ISR_frc < ticks)
		;
}

#define PIN_PERIOD (PORTB & (1 << 5)) //pin used to read collpits oscillator frequency

// GetPeriod() seems to work fine for frequencies between 200Hz and 700kHz.
long int GetPeriod(int n)
{
	int i;
	unsigned int saved_TCNT1a, saved_TCNT1b;

	_CP0_SET_COUNT(0);		// resets the core timer count
	while (PIN_PERIOD != 0) // Wait for square wave to be 0
	{
		if (_CP0_GET_COUNT() > (SYSCLK / 4))
			return 0;
	}

	_CP0_SET_COUNT(0);		// resets the core timer count
	while (PIN_PERIOD == 0) // Wait for square wave to be 1
	{
		if (_CP0_GET_COUNT() > (SYSCLK / 4))
			return 0;
	}

	_CP0_SET_COUNT(0);		// resets the core timer count
	for (i = 0; i < n; i++) // Measure the time of 'n' periods
	{
		while (PIN_PERIOD != 0) // Wait for square wave to be 0
		{
			if (_CP0_GET_COUNT() > (SYSCLK / 4))
				return 0;
		}
		while (PIN_PERIOD == 0) // Wait for square wave to be 1
		{
			if (_CP0_GET_COUNT() > (SYSCLK / 4))
				return 0;
		}
	}

	return _CP0_GET_COUNT();
}

/* SerialReceive() is a blocking function that waits for data on
 *  the UART2 RX buffer and then stores all incoming data into *buffer
 *
 * Note that when a carriage return '\r' is received, a nul character
 *  is appended signifying the strings end
 *
 * Inputs:  *buffer = Character array/pointer to store received data into, (NOT ANYMORE)
 *          max_size = number of bytes allocated to this pointer, (NOT ANYMORE)
 * Outputs: Number of characters received (NOT ANYMORE)*/
int SerialReceive()
{
	char buffer[10];
	unsigned int max_size = 10;
	unsigned int i = 0;

	if (IFS1bits.U2RXIF == 1)
	{ //flag is high if incoming data
		/* Store incoming data until either a carriage return is received
		*   or the number of received characters (num_chars) exceeds max_size */
		IFS1bits.U2RXIF = 0;
		while (i < max_size)
		{
			while (!U2STAbits.URXDA);				 // wait until data available in RX buffer
			buffer[i] = U2RXREG; // empty contents of RX buffer into *buffer pointer

			while (U2STAbits.UTXBF);				 // wait while TX buffer full
			U2TXREG = buffer[i]; // echo

			// insert null character to indicate end of string
			if (buffer[i] == '\r')
			{
				buffer[i] = '\0';
				break;
			}

			i++;
		}

		switch (buffer[0])
		{
		case 'w':
			return FORWARD;
		case 'a':
			return LEFT;
		case 's':
			return BACK;
		case 'd':
			return RIGHT;
		case 'c':
			return COLLECT_COIN;
		case 'p':
			return AUTONOMOUS;
		case 'm':
			return MANUAL;
		case 'q':
			return STOP;
		default:
			return SCRATCH;
		}
	}

	else
		return NO_DATA;
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

int ADCRead(char analogPIN)
{
	AD1CHS = analogPIN << 16; // AD1CHS<16:19> controls which analog pin goes to the ADC

	AD1CON1bits.SAMP = 1; // Begin sampling
	while (AD1CON1bits.SAMP)
		; // wait until acquisition is done
	while (!AD1CON1bits.DONE)
		; // wait until conversion done

	return ADC1BUF0; // result stored in ADC1BUF0
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
	TRISBbits.TRISB13 = 0; //vertical servo
	TRISBbits.TRISB14 = 0; //horizontal servo

	INTCONbits.MVEC = 1;

	SetupTimer1(); // Set timer 1 (to interrupt every 10 us?)
	SetupTimer4();
	ADCConf(); //configure ADC

	CFGCON = 0;
	ANSELB &= ~(1 << 5);	// Set RB5 as a digital I/O
	TRISB |= (1 << 5);		// configure pin RB5 as input
	CNPUB |= (1 << 5);		// Enable pull-up resistor for RB5
	UART2Configure(115200); // Configure UART2 for a baud rate of 115200

	delay_ms(2000);
	air_freq = getFrequency();

	while (1)
	{
		f = getFrequency();

		if((coin_counter) < 20) {       
			T4CONbits.TON=0;
			f = getFrequency();
			if(detectCoin(f)==1) {
				reverse_fast();      
				delay_ms(200);
				off();   
				delay_ms(200);
				pick_up_coin();
				playTune();
				(coin_counter)++;
				printf("%d", coin_counter);
			}
			else {
				forward_fast();
			}

			if(detectPerimeter()==1){
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

int detectCoin(float f)
{
	if (f > DETECT_FACTOR * air_freq)
	{ //very sensitive threshold
		return 1;
	}
	else
		return 0;
}

void determineCoin(float f){
	if (DIME_FACTOR*air_freq>=f){  //dime
		dime_counter++;
		myWallet = myWallet + .1;
	}
	else if (TOONIE_FACTOR*air_freq>=f){ //toonie
		toonie_counter++;
		myWallet = myWallet + 2.0;
	}
	else if (f>NICKEL_FACTOR*air_freq>=f){ //nickel
		nickel_counter++;
		myWallet = myWallet + 0.05;
	}
	else if(f>QUARTER_FACTOR*air_freq>=f){ //quarter
		quarter_counter++;
		myWallet = myWallet + 0.25;
	}
	else if (f>LOONIE_FACTOR*air_freq>=f){//loonie
		loonie_counter++;
		myWallet = myWallet + 1.0;
	}
}

float getFrequency()
{
	long int count;
	float T, f;

	count = GetPeriod(100);
	T = (count * 2.0) / (SYSCLK * 100.0);
	f = 1 / T;

	return f;
}

void pick_up_coin(void)
{
	ISR_pw_h = HORIZONTAL_STRAIGHT;
	delay_ms(300);
	ISR_pw_v = VERTICAL_DOWN;
	delay_ms(300);
	emagnet_pin = 0; //magnet on
	ISR_pw_h = HORIZONTAL_RIGHT;
	delay_ms(300);
	ISR_pw_h = HORIZONTAL_STRAIGHT;
	buzzer = 1;
	delay_ms(300);
	ISR_pw_h = HORIZONTAL_LEFT;
	delay_ms(300);
	ISR_pw_v = VERTICAL_UP;
	delay_ms(300);
	ISR_pw_h = HORIZONTAL_BIN;
	buzzer = 0;
	delay_ms(700);
	ISR_pw_v = LOWER_COIN;
	delay_ms(700);
	emagnet_pin = 1; //magnet off
	ISR_pw_v = VERTICAL_UP;
	delay_ms(700);
	ISR_pw_h = HORIZONTAL_STRAIGHT;
}

void playTune()
{
	T4CONbits.TON = 1;
	ISR_pw_buzz = 51;
	freq_change = 102;
	delay_ms(175);
	ISR_pw_buzz = 38;
	freq_change = 77;
	delay_ms(300);
	T4CONbits.TON = 0;
}

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

int detectPerimeter()
{
	//if the ADC has passed threshold, return true
	if ((ADCVoltage(PERIMETER_PIN1) > OTHER_THRESHOLD) || (ADCVoltage(PERIMETER_PIN2) > THRESHOLD_VOLTAGE))
	{
		return 1;
	}
	else
		return 0;
}

float ADCVoltage(char analogPIN)
{
	//multiply by refrence voltage, divide by ADC resolution
	return (float)ADCRead(analogPIN) * REF_VOLTAGE / (1023.0);
}

void autonomous(int* coin_counter){
	float f = getFrequency();

		if((*coin_counter) < 20) {       
			T4CONbits.TON=0;
			f = getFrequency();
				
			if(detectCoin(f)==1) {
				reverse_fast();      
				delay_ms(200);
				off();   
				delay_ms(200);
				pick_up_coin();
				playTune();
				(*coin_counter)++;
			}
			else {
				forward_fast();
			}

			if(detectPerimeter()==1){
				reverse_fast();
				delay_ms(700);
				rotateCCW_fast();
				delay_ms(500);
			}
		
		}		
		
		else {    
			off();
		}
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