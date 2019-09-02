#pragma once

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


/* SerialReceive() is a blocking function that waits for data on
 *  the UART2 RX buffer and then stores all incoming data into *buffer
 *
 * Note that when a carriage return '\r' is received, a nul character
 *  is appended signifying the strings end
 */

int SerialReceive();