#pragma once

//perimeter detector and ADC constants
#define REF_VOLTAGE 3.3
#define PERIMETER_PIN1 5
#define PERIMETER_PIN2 9
#define THRESHOLD_VOLTAGE 0.3
#define OTHER_THRESHOLD 0.2

//perimeter functions
int detectPerimeter();
void ADCConf(void);

//ADC functions
int ADCRead(char analogPIN);
float ADCVoltage(char analogPIN);