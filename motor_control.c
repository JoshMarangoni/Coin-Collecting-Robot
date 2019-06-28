/*
 *Created by Yousof Al-Autman March 19, 2019
 *Pin 21 and pin 22 are used for the right wheel motor h-bridge
 * Pin 15 and pin 16 are used the the left wheel motor h-bridge
 * Red wires from motor are always connected to left of h-bridge and black to  right (PMOS on top)
 * Connect pin 21 and pin 15 to the left mosfets of the h-bridges
 * Connect pin 22 and pin 16 to the right mosfets of the h-bridges

 */
#include "motor_control.h"
#include <xc-pic32m.h>

void forward_fast(){
    pin15 = 1;
    pin16 = 0;
    pin21 = 0;
    pin22 = 1;
}
void reverse_fast(){
    pin15 = 0;
    pin16 = 1;
    pin21 = 1;
    pin22 = 0;
    
}
void rotateCW_fast(){
    pin15 = 1;
    pin16 = 0;
    pin21 = 1;
    pin22 = 0;
    
}
void rotateCCW_fast(){
    pin15 = 0;
    pin16 = 1;
    pin21 = 0;
    pin22 = 1;
}

void off(){
    pin15 = 0;
    pin16 = 0;
    pin21 = 0;
    pin22 = 0;
}