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


    // TOD0: check
    // 1. Enable SYSCFG clock domain in RCC
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // 2. Configure EXTICR for the input encoder interrupt (A + B)
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI6, 0b000); // Select PA6
    
    SYSCFG->EXTICR[2] |= _VAL2FLD(SYSCFG_EXTICR3_EXTI8, 0b000); // Select PA6
    // Enable interrupts globally
    __enable_irq();

    // TOD0: check
    // Configure interrupt for falling edge of GPIO pin for button
    // 1. Configure mask bits
    EXTI->IMR1 |= (1 << gpioPinOffset(ENCODER_A)); // Configure the mask bit unmask PA8
  
    EXTI->IMR1 |= (1 << gpioPinOffset(ENCODER_B)); // Configure the mask bit unmask PA6
    
    // 2. enable rising edge trigger
    EXTI->RTSR1 |= (1 << gpioPinOffset(ENCODER_A));// enable rising edge trigger
   
    EXTI->RTSR1 |= (1 << gpioPinOffset(ENCODER_B));// enable rising edge trigger
   
    // 3. Enable falling edge trigger
    EXTI->FTSR1 |= (1 << gpioPinOffset(ENCODER_A));// Enable falling edge trigger
  
    EXTI->FTSR1 |= (1 << gpioPinOffset(ENCODER_B));// Enable falling edge trigger
    
    // 4. Turn on EXTI interrupt in NVIC_ISER
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);

   // in this loop we can calculate and print speed and direction for quadature
    while(1){   
        delay_millis(DELAY_TIM, 250);

        // if motor stop- detect as in no pulses
        if (COUNT_TIM->CNT > 45000){
        off = 1;
    }
    double rps = 0;

    // now evaulate rps (revolutions per sec)
    if (off) {
    rps = 0;
    } else {
    // calculate: 120 slots * 4 edges = 480
        rps = 1/(120*4.0* (fabs(delta)/1e6));
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