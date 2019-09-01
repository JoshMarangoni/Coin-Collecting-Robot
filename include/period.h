#pragma once

#define PIN_PERIOD (PORTB & (1 << 5)) //pin used to read collpits oscillator frequency

long int GetPeriod(int n);