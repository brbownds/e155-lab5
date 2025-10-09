// Broderick Bownds
// brbownds@hmc.edu
// 10/6/2025

// lab5_bb_main.j=h
// This module is the header for the main code where we contain the defintions 
// to define in the main.c 

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include <stm32l432xx.h>

///////////////////////////////////////////////////////////////////////////////
// Custom definitions
///////////////////////////////////////////////////////////////////////////////

#define ENCODER_A PA6
#define ENCODER_B PA8
#define POLLING_A PA4
#define POLLING_B PA5
#define DELAY_TIM TIM2


void EXTI9_5_IRQHandler(void);
void velocity_function(void);
 
#endif // MAIN_H

