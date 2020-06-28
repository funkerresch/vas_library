/**
 * @file vas_iir_highpass.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief 1 Pole lowpass <br>
 * From Miller Puckette's Implementation
 * https://puredata.info/
 */

#ifndef vas_lowpass_h
#define vas_lowpass_h

#include "vas_mem.h"
#include "vas_util.h"

typedef struct vas_iir_lowpass
{
    float sampleRate;
    float cutoff;
    float lastValue;
    float coefficient;
} vas_iir_lowpass;

vas_iir_lowpass *vas_iir_lowpass_new(float cutoff);

void vas_iir_lowpass_setCutoff(vas_iir_lowpass *x, float cutoff);

void vas_iir_lowpass_process(vas_iir_lowpass *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize);


#endif
