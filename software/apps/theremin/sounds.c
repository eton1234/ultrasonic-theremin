// PWM Sine App
//
// Create a sine wave and use PWM to play it at diferent frequencies

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrfx_pwm.h"

#include "microbit_v2.h"
#include "sounds.h"

// PWM configuration
static const nrfx_pwm_t PWM_INST = NRFX_PWM_INSTANCE(0);
//last note
static float last_note = -20;
static float notes[] = {0,0,0,0,0};
// Holds a pre-computed sine wave
#define SINE_BUFFER_SIZE 500
uint16_t sine_buffer[SINE_BUFFER_SIZE] = {0};

// Sample data configurations
// Note: this is a 32 kB buffer (about 25% of RAM)
#define BUFFER_SIZE 16000 // one second worth of data
uint16_t samples[BUFFER_SIZE] = {0}; // stores PWM duty cycle values


/*** Initialization & handling code ***/

void gpio_init(void) {
  // Initialize pins
  // Microphone pin MUST be high drive
  nrf_gpio_pin_dir_set(LED_MIC, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_cfg(LED_MIC, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT,
  NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0H1, NRF_GPIO_PIN_NOSENSE);

  // Enable microphone
  nrf_gpio_pin_set(LED_MIC);
}
#define COUNTERTOP_CONST 16000000 / (2 * SAMPLING_FREQUENCY)
void pwm_init(void) {

  // Initialize the PWM
  // SPEAKER_OUT is the output pin, mark the others as NRFX_PWM_PIN_NOT_USED
  // Set the clock to 16 MHz
  // Set a countertop value based on sampling frequency and repetitions
  // TODO
  nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
  pwm_config.output_pins[0] = EDGE_P0;
  pwm_config.output_pins[1] = NRFX_PWM_PIN_NOT_USED;
  pwm_config.output_pins[2] = NRFX_PWM_PIN_NOT_USED;
  pwm_config.output_pins[3] = NRFX_PWM_PIN_NOT_USED;
  pwm_config.base_clock = NRF_PWM_CLK_16MHz;
  pwm_config.count_mode = NRF_PWM_MODE_UP;
  pwm_config.load_mode = NRF_PWM_LOAD_COMMON;
  pwm_config.step_mode = NRF_PWM_STEP_AUTO;
  pwm_config.top_value = COUNTERTOP_CONST;
  nrfx_pwm_init(&PWM_INST, &pwm_config, NULL);
}

void compute_sine_wave(uint16_t max_value) {
  for (int i=0; i<SINE_BUFFER_SIZE; i++) {
    // what percent into the sine wave are we?
    float percent = (float)i / (float)SINE_BUFFER_SIZE;

    // get sine value
    float twopi = 2.0*3.14159;
    float radians = twopi * percent;
    float sine_value = sin(radians);

    // translate from "-1 to 1" into "0 to 2"
    float abs_sine = sine_value + 1.0;

    // scale from "0 to 2" into "0 to 1"
    float mag_1_sine = abs_sine / 2.0;

    // scale to the max_value desired ("0 to max_value")
    // and truncate to an integer value
    uint16_t value = (uint16_t)(max_value * mag_1_sine);

    // save value in buffer
    sine_buffer[i] = value;
  }
}

void play_note(uint16_t frequency) {

  // determine number of sine values to "step" per played sample
  // units are (sine-values/cycle) * (cycles/second) / (samples/second) = (sine-values/sample)
  float step_size = (float)SINE_BUFFER_SIZE * (float)frequency / (float)SAMPLING_FREQUENCY;
  // Fill sample buffer based on frequency
  // Each element in the sample buffer will be an element from the sine_buffer
  // The first should be sine_buffer[0],
  //    then each successive sample will be "step_size" further along the sine wave
  // Be sure to convert the cumulative step_size into an integer to access an item in sine_buffer
  // If the cumulative steps are greater than SINE_BUFFER_SIZE, wrap back around zero
  //    but don't just set it to zero, as that would be discontinuous
  // TODO
  float cumulative_step = 0;
  for (int i=0; i<BUFFER_SIZE; i++) {
    samples[i] = sine_buffer[(int) floor(cumulative_step)];
    cumulative_step += step_size;
    if (cumulative_step >= SINE_BUFFER_SIZE) {
      cumulative_step = cumulative_step - SINE_BUFFER_SIZE;
    }
  }

  // Create the pwm sequence (nrf_pwm_sequence_t) using the samples
  // Do not make another buffer for this. You can reuse the sample buffer
  // You should set a non-zero repeat value (this is how many times each _sample_ repeats)
  // TODO
  nrf_pwm_sequence_t pwm_sequence = {
    .values.p_common = samples,
    .length = BUFFER_SIZE,
    .repeats = 1,
    .end_delay = 0,
  };

  // Start playback of the samples
  // You will need to pass in a flag to loop the sound
  // The playback count here is the number of times the entire buffer will repeat
  //    (which doesn't matter if you set the loop flag)
  // TODO
  nrfx_pwm_simple_playback(&PWM_INST, &pwm_sequence, 1, NRFX_PWM_FLAG_LOOP);
}

//helper to play note based on difference in clock ticks! 
//playing note range [C, E] across octaves 4,5,6! 
 void stop_note(){
  printf("Stopping note!\n");
  last_note = -20;
  nrfx_pwm_stop(&PWM_INST, false);
}

// Define the Western musical scale frequencies
const float scale_notes[] = {
  // Octave 3
  130.81, // C3
  146.83, // D3
  164.81, // E3
  174.61, // F3
  196.00, // G3
  220.00, // A3
  246.94, // B3
  // Octave 4
  261.63, // C4
  // 277.18, // C#4
  // 293.66, // D4
  // 311.13, // D#4
  // 329.63, // E4
  // 349.23, // F4
  // 369.99, // F#4
  // 392.00, // G4
  // 415.30, // G#4
  // 440.00, // A4
  // 466.16, // A#4
  // 493.88  // B4
};

// Number of notes in the scale
const int num_notes = sizeof(scale_notes) / sizeof(scale_notes[0]);

// Hash function to map continuous frequencies to discrete frequencies in the scale
float hash_frequency(float freq) {
  // Quantize the input frequency to the nearest note in the scale
  float min_diff = fabs(scale_notes[0] - freq);
  float nearest = scale_notes[0];
  for (int i = 1; i < num_notes; ++i) {
    float diff = fabs(scale_notes[i] - freq);
    if (diff < min_diff) {
      min_diff = diff;
      nearest = scale_notes[i];
    }
  }
  return nearest;
}

void playNoteFromTick(int32_t time_diff){
  //Eq: freq_note = 0.4163 * Clock_Tick + 70.1; 
  //   float freq_note = (0.0466 * time_diff) + 100;
  float freq_note = (0.03 * time_diff) + 100;
  
  freq_note = hash_frequency(freq_note);
  if (last_note < (freq_note + 5) && last_note > (freq_note - 5)) {
    printf("hi");
    return;
  }
  // printf("freq_note: %f Hz", freq_note); 
  if (freq_note < 600) {
    play_note(freq_note);
    last_note = freq_note;
    printf("Playing note: %f Hz", last_note);
  }
  // play_note(440);
}