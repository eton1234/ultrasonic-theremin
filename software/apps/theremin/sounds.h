#include <stdint.h>
#define SAMPLING_FREQUENCY 16000 // 16 kHz sampling rate

void pwm_init(void);
void gpio_init(void);
void stop_note(void);
void playNoteFromInputs(float note, float offset);
float tickToFreq(int32_t tick);