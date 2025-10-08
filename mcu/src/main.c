// Broderick Bownds
// brbownds@hmc.edu
// 10/6/2025

// 

#include "main.h"

#define CPR 120
#define PPR 408
#define EDGES (CPR*4)
 
int32_t encoder_count;
volatile int delta = 0;
volatile int prevA = 0;
volatile int prevB = 0;

int main(void) {
    // Enable gpio pins as inputs
    gpioEnable(GPIO_PORT_A);
    pinMode(ENCODER_A, GPIO_INPUT);
    pinMode(ENCODER_B, GPIO_INPUT);

    // intialize timers TIM2 (delay)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM); // initialize delay timer

    // TOD0: done
    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input encoder interrupt (A + B)
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI6, 0b000); // Select PA6

    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR3_EXTI8, 0b000); // Select PA6
    // Enable interrupts globally
      
    // Enable global interrupts (safe after all setup)
    __enable_irq();
    
   // Configure EXTI mask and trigger settings for both PA6 and PA8
    EXTI->IMR1  |= (1 << 6) | (1 << 8);   // Unmask EXTI6, EXTI8
    EXTI->RTSR1 |= (1 << 6) | (1 << 8);   // Enable rising edge trigger
    EXTI->FTSR1 |= (1 << 6) | (1 << 8);   // Enable falling edge trigger

    // Enable EXTI lines 5–9 in NVIC (covers EXTI6 & EXTI8)
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);

    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);

while(1){   
        velocity_function();
        delay_millis(TIM2, 1000);
    }
}

void EXTI9_5_IRQHandler(void) {
    uint32_t pending = EXTI->PR1 & ((1 << 6) | (1 << 8));
    EXTI->PR1 = pending;  // clear flags

    int a = digitalRead(ENCODER_A);
    int b = digitalRead(ENCODER_B);

    // If A triggered
    if (pending & (1 << 6)) {
        if (a && !b)        delta = 1;   // CW rising A
        else if (a && b)    delta = -1;  // CCW rising A
        else if (!a && b)   delta = 1;   // CW falling A
        else if (!a && !b)  delta = -1;  // CCW falling A;
    }

    // If B triggered
    if (pending & (1 << 8)) {
        if (a && b)         delta = 1;   // CW rising B
        else if (!a && b)   delta = -1;  // CCW rising B
        else if (!a && !b)  delta = 1;   // CW falling B
        else if (a && !b)   delta = -1;  // CCW falling B;
    }

    encoder_count += delta;

    prevA = a;
    prevB = b;
}

void velocity_function(void) {
    // Average revolutions per second
    float rps = ((float)encoder_count) / (float)EDGES;

    // Print in your original format
    printf("Speed: %.3f rev/s | Direction: %s\n",
           (rps >= 0) ? rps : -rps,
           (rps < 0) ? "CCW" : "CW");

    encoder_count = 0;  // reset after printing
}
    
   