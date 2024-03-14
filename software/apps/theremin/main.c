//Need code to generate trigger signal from microbit
//and send to trigger pin on the distance sensor! 

//two distance sensor pins
#define TRIG_PIN EDGE_P7  // Output pin
#define ECHO_PIN EDGE_P8 // Input pin
#define SIG_PIN  EDGE_P9 //signal pin 

#include <stdint.h>
#include "time.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "virtual_timer.h"
#include "nrf_drv_timer.h"
//timer 
#include "microbit_v2.h"
#include "app_timer.h"
#include "nrfx_timer.h"
#include "../pwm_sine_tone/main.c"
#include "nrfx_gpiote.h"
#include "pthread.h"
pthread_mutex_t lock; // Declare a global mutex

//define the timer
static const nrfx_timer_t TIMER4 = NRFX_TIMER_INSTANCE(0);
//initialize play pause variable(initially play as default; global-scoped)
bool play = true;
//helepr to configure trigger and echo pins on breakout! 
void gpio_handler() {
    printf("Interrupt triggered value of play, %d \n", play);
    stop_note();
    play = !play;
}
bool lastVal = false;
void main() {
    // Enable timer! 
    // nrfx_timer_t const * const p_instance)
    nrfx_gpiote_init();
    virtual_timer_init();
    //set up gpio pins and pwm peripheral for playing audio notes! 
    gpio_init();
    pwm_init(); 
    //set the counter top that dictates sine signal wave! 
    compute_sine_wave((16000000 / (2 * SAMPLING_FREQUENCY)));
    //configure gpio pins
    nrf_gpio_pin_dir_set(TRIG_PIN, NRF_GPIO_PIN_DIR_OUTPUT);
    nrf_gpio_pin_dir_set(ECHO_PIN, NRF_GPIO_PIN_DIR_INPUT);
    nrf_gpio_pin_dir_set(SIG_PIN, NRF_GPIO_PIN_DIR_INPUT);
    ///interrupt handler for when sig pin goes high 
    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true); // high-accuracy mode
    // nrfx_gpiote_in_init(SIG_PIN, &in_config, gpio_handler);
    // nrfx_gpiote_in_event_enable(SIG_PIN, true); // enable interrupts
    lastVal = nrf_gpio_pin_read(SIG_PIN);
    while(true){
        //nrfx_timer_enable(&TIMER4); 
        // nrf_gpio_cfg_input(ECHO_PIN, NRF_GPIO_PIN_PULLDOWN);
        // nrf_gpio_cfg_output(TRIG_PIN);
        bool curVal = nrf_gpio_pin_read(SIG_PIN);
        if (curVal && !lastVal ) {
            play = !play;
            stop_note();
        }
        if (play) {
            nrf_gpio_pin_clear(TRIG_PIN);
            nrf_delay_us(1);
            nrf_gpio_pin_set(TRIG_PIN);
            nrf_delay_us(12);
            nrf_gpio_pin_clear(TRIG_PIN);
            nrf_delay_us(20);
            uint32_t start_ticks, end_ticks, elapsed_ticks;
            while (!nrf_gpio_pin_read(ECHO_PIN)){
            }
            start_ticks = read_timer(); 
            while (nrf_gpio_pin_read(ECHO_PIN)){
            }
            end_ticks = read_timer(); 
            elapsed_ticks = (int32_t) end_ticks - start_ticks; 
            playNoteFromTick(elapsed_ticks); 
        }
        lastVal = curVal; 
    }
}