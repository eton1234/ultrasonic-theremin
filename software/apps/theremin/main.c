//Need code to generate trigger signal from microbit
//and send to trigger pin on the distance sensor! 

//two distance sensor pins
#define TRIG_PIN EDGE_P8  // Output pin
#define ECHO_PIN EDGE_P12 // Input pin
#define SIG_PIN  EDGE_P2 // signal pin 

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
// for the joy stick
NRF_TWI_MNGR_DEF(twi_mngr_instance, 1, 0);
char notes[262];
const int freqs[] = {130,146,164,174,196,220,246,261};
const char letters[] = {'C','D','E','F','G','A','B','C'};
void i2c_setup() {
    ///********** I2C setup **********///
    nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
    // WARNING!!
    // These are NOT the correct pins for external I2C communication.
    // If you are using QWIIC or other external I2C devices, the are
    // connected to EDGE_P19 (SCL) and EDGE_P20 (SDA).
    i2c_config.scl = EDGE_P19;
    i2c_config.sda = EDGE_P20;
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
    // nrfx_timer_t const * const p_instance)
    nrfx_gpiote_init();
    led_matrix_init();
    virtual_timer_init();
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
            float freq = tickToFreq(elapsed_ticks);
            //printf("freq: %f\n", freq); 
            //printf("floored freq: %.2f\n", floor(freq)); 
            int letterIdx = (int) notes[(int)floor(freq)];
            //update LED display's state based on note letter! 
            updateLED(letterIdx);
            //printf("letter %d\n",letterIdx);
            playNoteFromInputs(freq, get_vertical());
        }
        lastVal = curVal; 
    }
}