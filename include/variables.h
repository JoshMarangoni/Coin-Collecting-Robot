#pragma once

//Global structs
typedef struct { 
    volatile int freq_change;
    volatile float air_freq;
} Frequencies;

typedef struct { 
    int ISR_pw_h;
    int ISR_pw_v;
    int ISR_cnt;
    int ISR_frc;
    int ISR_pw_buzz;
    int ISR_cnt_buzz;
} varISR;

typedef struct { 
    int coin_counter;
    float myWallet;
} coinCounters;