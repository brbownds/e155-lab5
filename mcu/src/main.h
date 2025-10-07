// Broderick Bownds
// brbownds@hmc.edu
// 10/6/2025

// This module is the header for the main code where we are ...

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Custom defines
///////////////////////////////////////////////////////////////////////////////

#define ENCODER_A PA6
#define ENCODER_B PA8
#define DELAY_TIM TIM2
#define COUNT_TIM TIM6

extern volatile int delta;  // time difference between encoder edges 
extern volatile int off;    // motor off flag (1 = stopped, 0 = running)

void EXTI9_5_IRQHandler(void);
 
#endif // MAIN_H