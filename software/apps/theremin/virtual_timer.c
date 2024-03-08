// Virtual timer implementation

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "nrf.h"

#include "virtual_timer.h"


// Read the current value of the timer counter
uint32_t read_timer(void) {

  // Should return the value of the internal counter for TIMER4
  //tell timer to capture to CC[0]
  NRF_TIMER4->TASKS_CAPTURE[0] = 1;
  return NRF_TIMER4->CC[0];
}

// Initialize TIMER4 as a free running timer
// 1) Set to be a 32 bit timer
// 2) Set to count at 1MHz
// 3) Enable the timer peripheral interrupt (look carefully at the INTENSET register!)
//    Skip this for the first part of the lab.
// 4) Clear the timer
// 5) Start the timer
void virtual_timer_init(void) {
  
  // NRF_TIMER4->EVENTS_COMPARE[0] = ;
  // Place your timer initialization code here
  //1Mhz
  NRF_TIMER4->PRESCALER = 4;
  //Number of bits
  NRF_TIMER4->BITMODE = 3;

  //CC[0] INTENSET = 1
  NRF_TIMER4->INTENSET = (1 << 17); //for reg 1

  NRF_TIMER4->TASKS_CLEAR = 1;
  NRF_TIMER4->TASKS_START = 1;
}



