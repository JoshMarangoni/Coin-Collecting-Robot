#include "serial.h"
#include <XC.h>

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