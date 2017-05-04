#include "wave.h"

/*** Time Operations ***/

// Delay the input signal 'in' by 'dt'.
uint8_t wave_delay( uint32_t t, uint8_t in, uint16_t dt )
{
    return 0;
}

/*** Basic Waveforms ***/

// Single pulse from '0' to 'dt'.
uint8_t wave_pulse( uint32_t t, uint16_t dt )
{
    return (t < dt);
}

// Pulses of length 'dt1' and period 'dt.
uint8_t wave_pwm( uint32_t t, uint16_t dt, uint16_t dt1)
{
    return ((t%dt) < dt1);
}

/*** Waveform Generator ***/

// Null wave function.
uint8_t wave_null( uint32_t t ) { return 0; }

// Creates a new waveform generator based on 'timer'.
wave_generator_t* wave_new_generator( uint32_t (*time)(void) )
{
    // Allocate memory on the heap.
    wave_generator_t* gen = malloc(sizeof(wave_generator_t));
    // Save timer pointer.
    gen->time = time;
    // Default wave pointer.
    gen->wave = wave_null;

    return gen;
}

// Apply a specific wave function to the generator.
void wave_generator_apply( wave_generator_t* gen, uint8_t (*wave)(uint32_t) )
{
    // Save wave pointer.
    gen->wave = wave;
    // Set origin time offset.
    gen->t_offset = (gen->time)();
}

// Get the current output of the wave generator.
uint8_t wave_generator_output( wave_generator_t* gen )
{
    return (gen->wave)( (gen->time)() - gen->t_offset );
}