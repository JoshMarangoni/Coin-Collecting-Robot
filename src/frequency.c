#include "period.h"

float getFrequency(long int SYSCLK)
{
	long int count;
	float T, f;

	count = GetPeriod(100);
	T = (count * 2.0) / (SYSCLK * 100.0);
	f = 1 / T;

	return f;
}