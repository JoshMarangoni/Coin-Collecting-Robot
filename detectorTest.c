#include <XC.h>
#include <string.h>
#include <stdio.h>
#include "config_bits.h"
#include "detectors.h"
#include <sys/attribs.h>


// Defines
#define SYSCLK 40000000L
#define Baud2BRG(desired_baud)      ( (SYSCLK / (16*desired_baud))-1)
 


//I don't know if this works, it may need to be moved to main
void __ISR(_TIMER_2_VECTOR, IPL5SOFT) Timer2_Handler(void){
	setDetectorFrequency(getNumEdges() * 8); //interupts every 1/8 secnds
	setNumEdges(0);		//reset edge counter
}

void UART2Configure(int baud_rate)
{
    // Peripheral Pin Select
    U2RXRbits.U2RXR = 4;    //SET RX to RB8
    RPB9Rbits.RPB9R = 2;    //SET RB9 to TX

    U2MODE = 0;         // disable autobaud, TX and RX enabled only, 8N1, idle=HIGH
    U2STA = 0x1400;     // enable TX and RX
    U2BRG = Baud2BRG(baud_rate); // U2BRG = (FPb / (16*baud)) - 1
    
    U2MODESET = 0x8000;     // enable UART2
}

/* SerialReceive() is a blocking function that waits for data on
 *  the UART2 RX buffer and then stores all incoming data into *buffer
 *
 * Note that when a carriage return '\r' is received, a nul character
 *  is appended signifying the strings end
 *
 * Inputs:  *buffer = Character array/pointer to store received data into
 *          max_size = number of bytes allocated to this pointer
 * Outputs: Number of characters received */
unsigned int SerialReceive(char *buffer, unsigned int max_size)
{
    unsigned int num_char = 0;
 
    /* Wait for and store incoming data until either a carriage return is received
     *   or the number of received characters (num_chars) exceeds max_size */
    while(num_char < max_size)
    {
        while( !U2STAbits.URXDA);   // wait until data available in RX buffer
        *buffer = U2RXREG;          // empty contents of RX buffer into *buffer pointer

        while( U2STAbits.UTXBF);    // wait while TX buffer full
        U2TXREG = *buffer;          // echo
 
        // insert nul character to indicate end of string
        if( *buffer == '\r')
        {
            *buffer = '\0';     
            break;
        }
 
        buffer++;
        num_char++;
    }
 
    return num_char;
}

 
void main(void)
{
    char   buf[1024];       // declare receive buffer with max size 1024
    unsigned int rx_size;

	CFGCON = 0;
    initCoinDetector();
    ADCConf();
    UART2Configure(115200);  // Configure UART2 for a baud rate of 115200
		
	printf("\x1b[2J\x1b[1;1H"); // Clear screen using ANSI escape sequence.
    while(1){
        printf("Freq = %i",getDetectorFrequency());
    }
}
