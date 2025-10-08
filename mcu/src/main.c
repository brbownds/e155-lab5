// Broderick Bownds
// brbownds@hmc.edu
// 10/8/2025

// T=In this 
#include "main.h"


#define PPR   408
#define EDGES (PPR*4)

volatile int32_t encoder_count = 0;
volatile int delta = 0;
volatile int prevA = 0;
volatile int prevB = 0;


// This is the main code

int main(void) {
    // Enable GPIO for encoder pins
    gpioEnable(GPIO_PORT_A);
    pinMode(ENCODER_A, GPIO_INPUT);

    pinMode(ENCODER_B, GPIO_INPUT);

    // Enable pull-downs (or pull-ups depending on your wiring)
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD6, 0b10); // PA6 pull-down

    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD8, 0b10); // PA8 pull-down

    // Enable TIM2 (used for delay)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Configure EXTI lines for PA6 and PA8
    // EXTI6 → EXTICR[1], EXTI8 → EXTICR[2]
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI6); // route EXTI6 to PA6

    SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI8); // route EXTI8 to PA8

    // Unmask EXTI lines 6 and 8
    EXTI->IMR1  |= (1 << 6) | (1 << 8);

    // Trigger on both rising and falling edges
    EXTI->RTSR1 |= (1 << 6) | (1 << 8);

    EXTI->FTSR1 |= (1 << 6) | (1 << 8);

    // Enable EXTI lines 5–9 interrupt in NVIC
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // Enable global interrupts
    __enable_irq();

    // Main loop
    while (1) {
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
    printf("%.3f", encoder_count);
    float rps = ((float)encoder_count) / (float)EDGES;
    const char* direction = (encoder_count < 0) ? "CCW" : "CW";

    printf("Speed: %.3f rev/s | Direction: %s\n", rps, direction);

    encoder_count = 0;  // reset for next interval
}




