//Need code to generate trigger signal from microbit
//and send to trigger pin on the distance sensor! 

//two distance sensor pins
#define TRIG_PIN EDGE_P7  // Output pin
#define ECHO_PIN EDGE_P8 // Input pin

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
//define the timer
static const nrfx_timer_t TIMER4 = NRFX_TIMER_INSTANCE(0);

//helepr to configure trigger and echo pins on breakout! 
void main() {
    // Enable timer! 
    // nrfx_timer_t const * const p_instance)
    virtual_timer_init();
    while(true){
        //nrfx_timer_enable(&TIMER4); 
        // nrf_gpio_cfg_input(ECHO_PIN, NRF_GPIO_PIN_PULLDOWN);
        // nrf_gpio_cfg_output(TRIG_PIN);
        nrf_gpio_pin_dir_set(TRIG_PIN, NRF_GPIO_PIN_DIR_OUTPUT);
        nrf_gpio_pin_dir_set(ECHO_PIN, NRF_GPIO_PIN_DIR_INPUT);
        nrf_gpio_pin_clear(TRIG_PIN);
        nrf_delay_us(1);
        nrf_gpio_pin_set(TRIG_PIN);
        nrf_delay_us(12);
        nrf_gpio_pin_clear(TRIG_PIN);
        nrf_delay_us(20);
        uint32_t start_ticks, end_ticks, elapsed_ticks;
        //printf("hi");
        while (!nrf_gpio_pin_read(ECHO_PIN)){
        }
        //  nrf_drv_timer_capture(&TIMER_INSTANCE, NRF_TIMER_CC_CHANNEL0, &end_ticks);
        start_ticks = read_timer(); 
        while (nrf_gpio_pin_read(ECHO_PIN)){
        }
        end_ticks = read_timer(); 
        elapsed_ticks = end_ticks - start_ticks; 
        printf("time dur: %ld", (int32_t)(elapsed_ticks));
        nrf_delay_ms(1000); 
    }
    
}