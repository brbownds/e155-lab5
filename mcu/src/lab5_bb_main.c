// Broderick Bownds
// brbownds@hmc.edu
// 10/8/2025

// lab5_bb_main.c
// This main code measures the speed (rev/s) of a DC motor using a quadrature encoder.
// It then compares interrupt-based edge detection to a polling-based toggle in the "while" loop
// and the interrupt handler so both can be viewed on an oscilloscope for timing comparison.

#include "lab5_bb_main.h"

#define PPR   408      // Pulses per revolution from encoder datasheet
#define EDGES (PPR*4) // 4x decoding (rising and falling on both channels)

volatile int32_t encoder_count = 0;
volatile int delta = 0;



int main(void) {
    // Enable GPIO pins (PA6 and PA8)for encoder pins
    gpioEnable(GPIO_PORT_A);
    pinMode(ENCODER_A, GPIO_INPUT);
    pinMode(ENCODER_B, GPIO_INPUT);
  
    // Enable GPIO pins (PA4 and PA5) for toggling polling pins
    // will compare throughout code
    pinMode(POLLING_A, GPIO_OUTPUT);
    pinMode(POLLING_B, GPIO_OUTPUT);


    // Enable pull-downs
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD6, 0b10); // PA6 pull-down
    GPIOA->PUPDR |= _VAL2FLD(GPIO_PUPDR_PUPD8, 0b10); // PA8 pull-down

    // Enable TIM2 (used for delay)
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initTIM(DELAY_TIM);

    // Enable SYSCFG clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Configure EXTI (external interrupt) lines for PA6 and PA8
    // EXTI6 -> EXTICR[1], EXTI8 -> EXTICR[2]
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI6); // route EXTI6 to PA6
    SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR3_EXTI8); // route EXTI8 to PA8

    // Unmask EXTI lines 6 and 8 for GPIO Pins
    EXTI->IMR1  |= (1 << gpioPinOffset(ENCODER_A)) | (1 << gpioPinOffset(ENCODER_B));

    // Trigger on both rising and falling edges (enabling both rising and falling)
    EXTI->RTSR1 |= (1 << gpioPinOffset(ENCODER_A)) | (1 << gpioPinOffset(ENCODER_B));
    EXTI->FTSR1 |= (1 << gpioPinOffset(ENCODER_A)) | (1 << gpioPinOffset(ENCODER_B));

    // Enable EXTI lines 5–9 interrupt in NVIC because both pins PA6 and PA8 fall within the interrupt
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // Enable global interrupts
    __enable_irq();

    // Main loop
    while (1) {
    togglePin(POLLING_A); // toggling pin for polling to compare later
        velocity_function();
        delay_millis(TIM2, 1000);
    }
}

// This interrupt service routine (ISR) executes whenever the encoder signals (PA6 or PA8)
// experience a rising or falling edge to determine the rotation direction.

void EXTI9_5_IRQHandler(void) {

// This ISR runs very quickly and captures every encoder edge even when 
// the motor spins at high speed. By toggling an output pin each time it runs,
// the interrupt frequency can be directly compared to a slower polling signal on the oscilloscope.

    // A brief toggle on POLLING_B is made to visualize interrupt timing on an oscilloscope.
    togglePin(POLLING_B);

    uint32_t pending = EXTI->PR1 & ((1 << gpioPinOffset(ENCODER_A)) | (1 << gpioPinOffset(ENCODER_B)));
    EXTI->PR1 = pending;  //  The pending EXTI flags are read and cleared (Write-1-to-Clear).

    int a = digitalRead(ENCODER_A);
    int b = digitalRead(ENCODER_B);

    // If the phase of Channel A leads B -> Clockwise (CW)
    // If the phase of Channel B leads A -> Counterclockwise (CCW)

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
    encoder_count += delta; // The global encoder_count is updated by -+1 accordingly.
}

// Loop computes speed by taking the encoder_count(counter) and dividing
// by the PPR and the four rising and falling edges of the channels
void velocity_function(void) {
    
    // printf("%.3f", encoder_count);
    float rps = ((float)encoder_count) / (float)EDGES;
    const char* direction = (encoder_count < 0) ? "CCW" : "CW";

    printf("Speed: %.3f rev/s | Direction: %s\n", rps, direction);

    encoder_count = 0;  // reset for next interval
}
