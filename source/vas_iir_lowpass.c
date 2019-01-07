//
//  vas_IIR_simple.c
//  tr.hproom1~
//
//  Created by Admin on 10.04.18.
//

#include "vas_iir_lowpass.h"

void vas_iir_lowpass_setCutoff(vas_iir_lowpass *x, float cutoff)
{
    if (cutoff < 0)
        cutoff = 0;
    
    x->cutoff = cutoff;
    x->coefficient = cutoff * (2 * 3.14159) / x->sampleRate;
    if (x->coefficient < 0)
        x->coefficient = 0;
    else if (x->coefficient > 1)
        x->coefficient = 1;
}

vas_iir_lowpass *vas_iir_lowpass_new(float cutoff)
{
    vas_iir_lowpass *x = (vas_iir_lowpass *) vas_mem_alloc(sizeof(vas_iir_lowpass));
    x->sampleRate = 44100;
    x->lastValue = 0;
    vas_iir_lowpass_setCutoff(x, cutoff);
    
    return x;
}

void vas_iir_lowpass_process(vas_iir_lowpass *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize)
{
    int n = 0;
    float last = x->lastValue;
    float coef = x->coefficient;
    float feedback = 0.5;
    
    while(n++ < vectorSize)
        last =  *out++ = coef * *in++ + feedback * last;
   
    x->lastValue = last;
}
