#pragma once

//constants used for defining coin picking arm servo motor positions
#define VERTICAL_DOWN 178
#define VERTICAL_UP 65
#define LOWER_COIN 100
#define HORIZONTAL_STRAIGHT 120
#define HORIZONTAL_RIGHT 200
#define HORIZONTAL_LEFT 60
#define HORIZONTAL_BIN 190

//coin detector frequency multiplier
#define DETECT_FACTOR 1.0078 //1.001 this might change with battery level
#define DIME_FACTOR 1.009110138//1.5025
#define NICKEL_FACTOR 1.018747873
#define TOONIE_FACTOR 1.018843976//1.009
#define LOONIE_FACTOR 1.018435686 //1.0083
#define QUARTER_FACTOR 1.016470676// 1.008

#define TOONIE_F 47168.699219
#define DIME_F 46718.058594
#define NICKEL_F 47164.250000
#define LOONIE_F 47149.796875
#define QUARTER_F 47058.824219
#define AIR_F 46296.292969

//coin collecting functions
int detectCoin(float f);
void pick_up_coin();
long int GetPeriod(int n);
float getFrequency();