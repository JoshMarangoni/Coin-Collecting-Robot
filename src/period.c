#include "period.h"
#include <XC.h>
#include "sysclk.h"

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