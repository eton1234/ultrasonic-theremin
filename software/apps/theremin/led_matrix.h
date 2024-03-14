#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Initialize the LED matrix display
void led_matrix_init(void);
void readText(char* text); 
void updateLED(int32_t letterIndex);    
// You may need to add more functions here
