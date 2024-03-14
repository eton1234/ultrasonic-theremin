//Need code to generate trigger signal from microbit
//and send to trigger pin on the distance sensor! 

//two distance sensor pins
#define TRIG_PIN EDGE_P8  // Output pin
#define ECHO_PIN EDGE_P12 // Input pin
#define SIG_PIN  EDGE_P1 // signal pin 

#include <stdint.h>
#include <math.h>
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
#include "sounds.h"
#include "nrfx_gpiote.h"
#include "pthread.h"
#include "vibrato_sensor.h"
#include "nrf_twi_mngr.h"
#include "led_matrix.h"
pthread_mutex_t lock; // Declare a global mutex

//define the timer
static const nrfx_timer_t TIMER4 = NRFX_TIMER_INSTANCE(0);

//initialize play pause variable(initially play as default; global-scoped)
bool play = true;
bool lastVal = false;

// joy stick i2c instance
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);

// ****** dictionary of notes *****///
char notes[262];
const int freqs[] = {130,146,164,174,196,220,246,261};
const char letters[] = {'C','D','E','F','G','A','B','C'};

///********** I2C setup **********///
void i2c_setup() {
    nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    i2c_config.scl = EDGE_P19; //external SCL
    i2c_config.sda = EDGE_P20; //external SDA
    i2c_config.frequency = NRF_TWIM_FREQ_100K;
    i2c_config.interrupt_priority = 0;
    //start a manager with the conifg 
    nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
    joy_init(&twi_mngr_instance);
}
void sound_setup() {
    //set up gpio pins and pwm peripheral for playing audio notes! 
    gpio_init();
    pwm_init(); 
}

void main() {
    //initializing notes dictionary
    for(int i = 0; i < sizeof(freqs)/sizeof(freqs[0]); i++) {
        notes[freqs[i]] = letters[i];
    }
    i2c_setup();
    sound_setup();
    // Enable timer! 
    nrfx_gpiote_init();
    led_matrix_init();
    virtual_timer_init();
    //set the counter top that dictates sine signal wave! 
    compute_sine_wave((16000000 / (2 * SAMPLING_FREQUENCY)));
    //configure gpio pins
    nrf_gpio_pin_dir_set(TRIG_PIN, NRF_GPIO_PIN_DIR_OUTPUT);
    nrf_gpio_pin_dir_set(ECHO_PIN, NRF_GPIO_PIN_DIR_INPUT);
    nrf_gpio_pin_dir_set(SIG_PIN, NRF_GPIO_PIN_DIR_INPUT);
    lastVal = nrf_gpio_pin_read(SIG_PIN);

    while(true){
        bool curVal = nrf_gpio_pin_read(SIG_PIN);
        //detecting low-to-high transition (tried interrupts but didn't work well)
        if (curVal && !lastVal ) {
            play = !play;
            stop_note();
        }
        if (play) {
            //send trigger signal to distance sensor
            nrf_gpio_pin_clear(TRIG_PIN);
            nrf_delay_us(1);
            nrf_gpio_pin_set(TRIG_PIN);
            nrf_delay_us(12);
            nrf_gpio_pin_clear(TRIG_PIN);
            nrf_delay_us(20);
            uint32_t start_ticks, end_ticks, elapsed_ticks;
            while (!nrf_gpio_pin_read(ECHO_PIN)){
            }
            //time how long the echo pin is high
            start_ticks = read_timer(); 
            while (nrf_gpio_pin_read(ECHO_PIN)){
            }
            end_ticks = read_timer(); 
            elapsed_ticks = (int32_t) end_ticks - start_ticks; 

            //convert ticks to frequency
            float freq = tickToFreq(elapsed_ticks);
            //get the note letter index
            int letterIdx = (int) notes[(int)floor(freq)];

            //update LED display's state based on note letter! 
            updateLED(letterIdx);

            
            //play note with offset vibrato based off vertical position of the joy stick
            playNoteFromInputs(freq, get_vertical());
        }
        lastVal = curVal; 
    }
}