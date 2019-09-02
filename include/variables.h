#pragma once

//Global structs
typedef struct { 
    volatile int freq_change;
    volatile float air_freq;
} Frequencies;

typedef struct { 
    int volatile ISR_pw_h;
    int volatile ISR_pw_v;
    int volatile ISR_cnt;
    int volatile ISR_frc;
    int volatile ISR_pw_buzz;
    int volatile ISR_cnt_buzz;
} varISR;

typedef struct { 
    int volatile coin_counter;
    float volatile myWallet;
} coinCounters;


extern varISR V;
extern coinCounters C;
extern Frequencies F;