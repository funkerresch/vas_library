/**
 * @file vas_iir_highpass.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief 1 Pole Highpass <br>
 * From Miller Puckette's Implementation
 * https://puredata.info/
 */

#ifndef vas_highpass_h
#define vas_highpass_h

#include "vas_mem.h"
#include "vas_util.h"

typedef struct vas_iir_highpass
{
    float sampleRate;
    float cutoff;
    float lastValue;
    float coefficient;
} vas_iir_highpass;

vas_iir_highpass *vas_iir_highpass_new(float cutoff);

void vas_iir_highpass_setCutoff(vas_iir_highpass *x, float cutoff);

void vas_iir_highpass_process(vas_iir_highpass *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize);


#endif 
