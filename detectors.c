#include <XC.h>
#include <stdio.h>
#include <stdlib.h>
#include "detectors.h"
#include "Timer_init.h"

volatile int 	numEdges;
volatile int 	detectorFrequency;
volatile bool   lastPinState;

int getNumEdges(void){
    return numEdges;
}

int getDetectorFrequency(void){
    return detectorFrequency;
}

void setNumEdges(int newNumEdges){
    numEdges=newNumEdges;
}

void setDetectorFrequency(int newFrequency){
    detectorFrequency = newFrequency;
}
bool detectPerimeter(void){
	//if the ADC has passed threshold, return true
	if(ADCVoltage(PERIMETER_PIN) > THRESHOLD_VOLTAGE){
		return true;
	}
	else{
		return false;
	}
}

bool detectCoin(void){
	//detect rising edge
	if(pinState() == true && lastPinState == false){
		numEdges++;		//count the edge
	}
	lastPinState = pinstate();	//update the pin state
	
	//if the detector frequency has exceeded the threshold
	if(detectorFrequency > THRESHOLD_FREQUENCY){
		return true;
	}
	else{
		return false;
	}
}

void initCoinDetector(void){
	TRISBbits.TRISB4 	= 1;
	numEdges 		= 0;		//counts the number of rising edges before an interrupt
	detectorFrequency 	= 0;		//stores the current frquency measured
	lastPinState 		= pinState();	//set the lastpinState for edge detection
	SetupTimer2(0); 			//interupt every 1/8 seconds
	
}

//return the state of the copin detector pin as a boolean
bool pinState(void){
	if(COIN_DETECTOR_PIN == 1){
		return true;	
	}
	else{
		return false;
	}
}

// Good information about ADC in PIC32 found here:
// http://umassamherstm5.org/tech-tutorials/pic32-tutorials/pic32mx220-tutorials/adc
void ADCConf(void){
    	AD1CON1CLR 	= 0x8000;    // disable ADC before configuration
    	AD1CON1 	= 0x00E0;    // internal counter ends sampling and starts conversion (auto-convert), manual sample
    	AD1CON2 	= 0;         // AD1CON2<15:13> set voltage reference to pins AVSS/AVDD
    	AD1CON3 	= 0x0f01;    // TAD = 4*TPB, acquisition time = 15*TAD 
    	AD1CON1SET	= 0x8000;    // Enable ADC
}

int ADCRead(char analogPIN){
    	AD1CHS = analogPIN << 16;    // AD1CHS<16:19> controls which analog pin goes to the ADC
 
    	AD1CON1bits.SAMP = 1;        // Begin sampling
   	while(AD1CON1bits.SAMP);     // wait until acquisition is done
    	while(!AD1CON1bits.DONE);    // wait until conversion done
 
    	return ADC1BUF0;             // result stored in ADC1BUF0
}


float ADCVoltage(char analogPIN){
	//multiply by refrence voltage, divide by ADC resolution
	return (float)ADCRead(analogPIN)*REF_VOLTAGE/(1023.0);
}
