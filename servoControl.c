/*
 * Servo Control Library
 * Contributors: Justin Scott, Joshua Marangoni
 */

#include <xc.h>
#include "servoControl.h"
#include "bool.h"



void configSoftServo(void){
	// Configure the pin we are using for servo control as output
	TRISBbits.TRISB14 = 0;
	LATBbits.LATB14 = 0;
	TRISBbits.TRISB15 = 0;
	LATBbits.LATB15 = 0;
	INTCONbits.MVEC = 1;
	ISR_frc = 0;
	
	SetupTimer1(); // Set timer 5 to interrupt every 10 us
}

bool setSoftServo(int pin, int position){
  	//invalid position
	if( (position<60) || (position>240) ){
	 	return false;
  	}
	
  	if(pin == 14){
		ISR_pw1 = position;
	}
	else if(pin == 15){
		ISR_pw2 = position;
	}
	//invalid pin
	else{
		return false;
	}
  	
  	return true;
}
/*******************************************************************
*  configServo
*
*  Configures the selected pin tp cpntrol a servo. Returns False if 
*  error checking fails and function does not configure the pin 
* (ie pin does not exist or is not available for PWM)
*******************************************************************/
bool configServo(int pin){
  bool returnValue = true;
  switch(pin){
    case 6:{
      RPB6Rbits.RPB6R = 0x0005;
      // Configure standard PWM mode for output compare module 1
      OC4CON = 0b1110; 
      // A write to PRy configures the PWM frequency
      // PR = [FPB / (PWM Frequency * TMR Prescale Value)] – 1
      // : note the TMR Prescaler is 1 and is thus ignored
      PR3 = (SYSCLK / PWM_DEFAULT_FREQ) - 1;
      // A write to OCxRS configures the duty cycle
      // : OCxRS / PRy = duty cycle
      OC4RS = (PR2 + 1) * ((float)DEFAULT_DUTY_CYCLE / 100);
 
      T3CONSET = 0x8000;      // Enable Timer2, prescaler 1:1
      OC4CONSET = 0x8000; // Enable Output Compare Module 1 
      //successfully configured PWM
      break;
    }
    case 7:{
      RPB7Rbits.RPB7R = 0x0005; 
      // Configure standard PWM mode for output compare module 1
      OC1CON = 0x0006; 
      // A write to PRy configures the PWM frequency
      // PR = [FPB / (PWM Frequency * TMR Prescale Value)] – 1
      // : note the TMR Prescaler is 1 and is thus ignored
      PR2 = (SYSCLK / PWM_DEFAULT_FREQ) - 1;
      // A write to OCxRS configures the duty cycle
      // : OCxRS / PRy = duty cycle
      OC1RS = (PR2 + 1) * ((float)DEFAULT_DUTY_CYCLE / 100);
 
      T2CONSET = 0x8000;      // Enable Timer2, prescaler 1:1
      OC1CONSET = 0x8000; // Enable Output Compare Module 1 
      //successfully configured PWM
      break;
    }
    default:{
      //requested pin does not support PWM, so function is unable to configure PWM
      //and therfore returns false
      returnValue = false;
      break;
    }
  }
  return returnValue;
}

/*******************************************************************
*  setPWM
*
*  Sets the frequency and duty Cycle at a given pin. Returns False if 
*  error checking fails and function does not set the PWM 
* (ie the frequency or duty cycle is outside the acceptable range)
*******************************************************************/
bool setPWM(int pin, int frequency, int dutyCycle){
  bool returnValue = true;
  //input validation
  if(frequency < 0 || dutyCycle < 0 || dutyCycle > 100){
    returnValue = false;
  }
  else{
	switch(pin){
		case 6:{
			// A write to PRy configures the PWM frequency
			// PR = [FPB / (PWM Frequency * TMR Prescale Value)] – 1
			// : note the TMR Prescaler is 1 and is thus ignored
			PR3 = (SYSCLK / frequency) - 1;
			// A write to OCxRS configures the duty cycle
			// : OCxRS / PRy = duty cycle
			OC4RS = (PR2 + 1) * ((float)dutyCycle / 100);
			break;
		}
		case 7:{
			PR2 = (SYSCLK / frequency) - 1;
			OC1RS = (PR2 + 1) * ((float)dutyCycle / 100);
			break;
		}
		default:{
			returnValue = false;
			break;
		}
	}
  }
  return returnValue;
}

/*******************************************************************
*  setServo
*
* Generates one or more standard hobby servo signals. The servo 
* signal has a fixed period of 20ms and a pulse width between 0.6ms 
* and 2.4ms. Returns False if error checking fails and function does 
* not set the PWM. (ie the postion is outside the acceptable range 
* of 60-240)
*******************************************************************/

bool setServo(int pin, int position){
  if( (position<60) || (position>240) ){
	  return false;
  }
  //set the PWM for the expected pin, return the error state
  return setPWM(pin, SERVO_FREQUENCY, 100.0*((float)position*(0.00001))/SERVO_PERIOD);
}
