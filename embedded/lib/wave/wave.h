#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/*** Time Operations ***/

// Delay the input signal 'in' by 'dt'.
uint8_t wave_delay( uint32_t t, uint8_t in, uint16_t dt );

/*** Basic Waveforms ***/

// Generate a pulse from '0' to 'dt'.
uint8_t wave_pulse( uint32_t t, uint16_t dt );
// Generate pulses of length 'dt1' and period 'dt.
uint8_t wave_pwm( uint32_t t, uint16_t dt, uint16_t dt1 );

/*** Waveform Generator ***/

typedef struct wave_generator {
    // Time offset.
    uint32_t t_offset;
    // Current time function.
    uint32_t (*time)( void );
    // Wave function.
    uint8_t (*wave)( uint32_t );
} wave_generator_t;

// Null wave function.
uint8_t wave_null( uint32_t );
// Creates a new waveform generator based on 'timer'.
wave_generator_t* wave_new_generator( uint32_t (*time)(void) );
// Apply a specific wave function to the generator.
void wave_generator_apply( wave_generator_t* gen, uint8_t (*wave)(uint32_t) );
// Get the current output of the wave generator.
uint8_t wave_generator_output( wave_generator_t* gen );

#ifdef __cplusplus
}
#endif
