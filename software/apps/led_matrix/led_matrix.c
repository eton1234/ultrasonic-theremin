// LED Matrix Driver
// Displays characters on the LED matrix

#include <stdbool.h>
#include <stdio.h>

#include "nrf_gpio.h"
#include "app_timer.h"

#include "led_matrix.h"
#include "font.h"
#include "microbit_v2.h"

APP_TIMER_DEF(part2_timer); 
APP_TIMER_DEF(last_timer); 
//part2 timer variable! 
bool shouldSet = false; 

//part 3 variables! 
int curRow = 0; 
bool led_states[5][5] = {false};
uint32_t rowleds[] = {LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4, LED_ROW5};
uint32_t colleds[]  = {LED_COL1, LED_COL2, LED_COL3, LED_COL4, LED_COL5}; 
/*Part 2's callback function 
//takes in flag variable indicating whether we should set columns on high! 
void part2_cb(void){
  //turn col 1 and 5 of row 1 of matrix off! 
  if(shouldSet){
    nrf_gpio_pin_set(LED_COL1);
     nrf_gpio_pin_set(LED_COL5);
  } 
  //otherwise, turn both entries on! 
  else{
    nrf_gpio_pin_clear(LED_COL1);
     nrf_gpio_pin_clear(LED_COL5);
  }
  shouldSet = !shouldSet; 
}*/
//Part 3's callback! 
void part3_cb(void){
  //Invalidate previous row and clear colss(set corresponding column pins back to high)! 
  //1)Set cur row to low! 
  nrf_gpio_pin_write(rowleds[curRow], 0);
  for (size_t i = 0; i < 5; i++) {
     nrf_gpio_pin_write(colleds[i], 1); 
  }
  //next row 
  curRow = (curRow + 1) % 5;
  //2) set active row high! 
  nrf_gpio_pin_write(rowleds[curRow], 1); 
  //set proper leds to low
  for (size_t i = 0; i < 5; i++) {
    if (led_states[curRow][i]){
      nrf_gpio_pin_write(colleds[i], 0); 
    }
  }
}

//Part 4 helper function to update led states boolean matrix based off font 2-d matrix! 
//It will take in the letterIndex corresponding to letter we want to display! 
void updateLED(int32_t letterIndex){
  //loop through encoding corresponding to each row for the letter pattern! 
  for(int i = 0; i < 5; i++){
    uint8_t curRowEncoding = font[letterIndex][i];
    //loop through each of 5 columns, get the bit-val, and dep. whether it's set, 
    //make sure to modify the boolean 2-d matrix led_states to reflect this info! 
    for(int j = 0; j < 5; j ++){
      //check whether cur LSB is set corresponding to jth row! If set, that means
      //row i, col j led point should be active and set on! 
      uint8_t res = curRowEncoding & 0x01; 
      if(res){
        led_states[i][j] = true; 
      } else{
        led_states[i][j] = false; 
      }
      //then, need to bit shift 1 right for cur row encoding to process state of next col.
      //in same row! 
      curRowEncoding = curRowEncoding >> 1; 
    }
  }
}
//LAST PART! 
//might need additional global vars to keep track of current text we are displaying! 
//Cur Text being displayed onto LED display, the current character we are displaying within text
//by index offset! 
char* curInput = "";
int currentCharIndex = 0; 

//helper function that takes in the text to print out onto LED display!
void readText(char* text){
  //set global variable to new text we are about to read! 
  curInput = text;
  //maybe start the timers here and be blocked till we process every char??
  app_timer_start(last_timer, 32768, NULL);
  app_timer_start(part2_timer, 100, NULL); 
  //basically, we made readText blocking until it read every single character! 
  while(currentCharIndex <= strlen(curInput)){
    continue;
  }
  //once we are completely done reading cur text, reset the cur input state back to empty string
  //and char offset back to 0! Also, maybe kill the timer until we make another call to 
  //read another separate text?? 
  app_timer_stop(last_timer);
  app_timer_stop(part2_timer);
  curInput = "";
  currentCharIndex = 0; 
  //also, we want to make sure all columns are off by setting all columns high! 
  for(int i = 0; i < 5; i ++){
    nrf_gpio_pin_write(colleds[i], 1); 
  }
  //and set all rows back to low at default initialization state! 
  for(int row = 0; row < 5; row++){
    nrf_gpio_pin_clear(rowleds[row]); 
  }
}

//need to define a callback for 2nd timer that periodically updates the current char
//we are displaying within input Text! This callback will update the state of global-scoped
//2-d matrix led_states to reflect current char and then increase the char index so 
//next call to callback will update led_states for next corresponding char! 
void part5_cb(){
    if(curInput){
      //get current character within cur input text! 
      char curChar = curInput[currentCharIndex];
      printf("cur char: %s", curChar); 
      //get the current char's ascii encoding! 
      int curLetterIndex = (int) curChar; 
      printf("curLetter ascii val: %d", curLetterIndex); 
      //call updateLED helper to reflect current character's LED pattern! 
      updateLED(curLetterIndex);
      //and then increment cur char index to next char! 
      currentCharIndex++;
    }
}

void led_matrix_init(void) {
  //1)Initialization: Set all pins of LED matrix to output direction! 
  //arrays that store macros that store specific pin mappings for each row/col! 
  for(int row = 0; row < 5; row++){
    nrf_gpio_pin_dir_set(rowleds[row], NRF_GPIO_PIN_DIR_OUTPUT); 
    nrf_gpio_pin_clear(rowleds[row]); 
  }
  for(int col = 0; col < 5; col++){
    nrf_gpio_pin_dir_set(colleds[col], NRF_GPIO_PIN_DIR_OUTPUT); 
    nrf_gpio_pin_set(colleds[col]);
  }
  // set default values for pins 
  //for now, let's just focus on first row! 
  /* Part 1 Checkoff code 
  nrf_gpio_pin_set(LED_ROW1); 
  nrf_gpio_pin_clear(LED_COL1);
  nrf_gpio_pin_clear(LED_COL2);
  nrf_gpio_pin_clear(LED_COL3);
  nrf_gpio_pin_clear(LED_COL4);
  nrf_gpio_pin_set(LED_COL5); */
  // initialize timer(s) (Step 2 and onwards)
  //Part 2
  //middle 3 is on along 1st row! (Initial values along first row of LED)
  

  /*Part 3: We want to display X pattern! 
  led_states[0][0] = true; 
  led_states[0][4] = true; 
  led_states[1][1] = true; 
  led_states[1][3] = true; 
  led_states[2][2] = true; 
  led_states[3][1] = true;
  led_states[3][3] = true;
  led_states[4][0] = true;
  led_states[4][4] = true; */
  //Part 4: To display single character, we need to modify led_states based on the font array
  //which is available from including font.h header file! We first call updateLED function
  //to make sure boolean 2-d matrix led_states is up to date. Then, based on this info, we need
  //to make sure timer triggers part3_cb function to iterate through rows of LED to show pattern!
  /*
  //For now, let's test letter 'R' => index 82! 
   //updateLED(82); 
   //test for letter 'K' => index 0 
   updateLED(75); */

   //LAST PART! 

   //initialize timer module! 
   app_timer_init();
   //might need another timer to periodically switch characters for curText!
   app_timer_create(&last_timer, APP_TIMER_MODE_REPEATED, part5_cb);
   //create a timer to display led pattern for single current letter! 
   app_timer_create(&part2_timer, APP_TIMER_MODE_REPEATED, part3_cb);
   //start the sample timer! 
   //switch char every 1 sec?
   /*
   app_timer_start(last_timer, 32768, NULL);
   app_timer_start(part2_timer, 100, NULL); */
  // set default state for the LED display (Step 3 and onwards)
}

