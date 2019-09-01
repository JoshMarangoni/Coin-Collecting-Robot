#include "perimeter.h"
#include <XC.h>

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

float ADCVoltage(char analogPIN)
{
	//multiply by refrence voltage, divide by ADC resolution
	return (float)ADCRead(analogPIN) * REF_VOLTAGE / (1023.0);
}