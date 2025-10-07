// Broderick Bownds
// brbownds@hmc.edu
// 10/6/2025

// 

#include "main.h"

volatile int delta = 0;
volatile int off = 1;

int main(void) {
    // Enable gpio pins as inputs
    gpioEnable(GPIO_PORT_A);
    pinMode(ENCODER_A, GPIO_INPUT);
    pinMode(ENCODER_B, GPIO_INPUT);

    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD6, 0b01); // Set PA6 as pull-up
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD8, 0b01); // Set PA8 as pull-up

    // enable timers TIM2 (delay) and TIM6 (pulse count)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
   
    initTIM(DELAY_TIM); // initialize delay timer

    // Counting timer (TIM6): used to measure time between edges
    initTIM(COUNT_TIM);  // this will start counting continuously

    COUNT_TIM->CR1 |= TIM_CR1_CEN;  // start counting


    // TOD0: check
    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input encoder interrupt (A + B)
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI6, 0b000); // Select PA6
    
    SYSCFG->EXTICR[2] |= _VAL2FLD(SYSCFG_EXTICR3_EXTI8, 0b000); // Select PA6
    // Enable interrupts globally
  

   // Configure EXTI mask and trigger settings
    EXTI->IMR1  |= (1 << 6) | (1 << 8);   // Unmask EXTI6, EXTI8
    EXTI->RTSR1 |= (1 << 6) | (1 << 8);   // Enable rising edge trigger
    EXTI->FTSR1 |= (1 << 6) | (1 << 8);   // Enable falling edge trigger

    // Clear any pending interrupts BEFORE enabling globally
    EXTI->PR1 = (1 << 6) | (1 << 8);

    // Enable EXTI lines 5–9 in NVIC (covers EXTI6 & EXTI8)
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);

    // Enable global interrupts (safe after all setup)
    __enable_irq();
    
    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);

   // in this loop we can calculate and print speed and direction for quadature
    while(1){   
        delay_millis(DELAY_TIM, 250);

        // if motor stop- detect as in no pulses
        if (COUNT_TIM->CNT > 45000){
        off = 1;
    }
    double rps = 0.0;

    // now evaulate rps (revolutions per sec)
    if (off) {
    rps = 0.0;
    } else {
    // calculate: 120 slots * 4 edges = 480
        rps = 1.0/(120.0*4.0* (fabs(delta)/1e6));
    }

    // print speed and direction
    printf("Speed: %.3f rev/s | Direction: %s\n", rps, (delta < 0)? "CCW" : "CW");
    }
}

// TOD0: What is the right name for the IRQHandler? 
// now we write our interrupt routine where it reads both signals, determines 
// direction adn measures the timing
void EXTI9_5_IRQHandler(void){
   int a = digitalRead(ENCODER_A);
  
   int b = digitalRead(ENCODER_B);

    // PA6
    if (EXTI->PR1 & (1 << 6 )){
        // If so, clear the interrupt (NB: Write 1 to reset.)
        EXTI->PR1 |= (1 << 6 );
        off = 0;
        if (a==b) {
          delta = -COUNT_TIM->CNT; //negative delta
        }
      COUNT_TIM->CNT = 0; //reset timer
     }
    //PA8
    if (EXTI->PR1 & (1<<8)){
    EXTI->PR1 |= (1<<8); // clear the pending flag
    off = 0;
    if (a==b) {
        delta = COUNT_TIM->CNT; // positive delta
        }
      COUNT_TIM->CNT = 0;
    }
}