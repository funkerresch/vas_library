//
//  vas_IIR_simple.c
//  tr.hproom1~
//
//  Created by Admin on 10.04.18.
//

#include "vas_iir_highpass.h"

void vas_iir_highpass_setCutoff(vas_iir_highpass *x, float cutoff)
{
    if (cutoff < 0)
        cutoff = 0;
    
    x->cutoff = cutoff;
    x->coefficient = 1 - cutoff * (2 * 3.14159) / x->sampleRate;
    if (x->coefficient < 0)
        x->coefficient = 0;
    else if (x->coefficient > 1)
        x->coefficient = 1;
}

vas_iir_highpass *vas_iir_highpass_new(float cutoff)
{
    vas_iir_highpass *x = (vas_iir_highpass *) vas_mem_alloc(sizeof(vas_iir_highpass));
    x->sampleRate = 44100;
    x->lastValue = 0;
    vas_iir_highpass_setCutoff(x, cutoff);
    
    return x;
}

void vas_iir_highpass_process(vas_iir_highpass *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize)
{
    int n = 0;
    float last = x->lastValue;
    float coef = x->coefficient;

    float normal = 0.5*(1+coef);
    while(n++ < vectorSize)
    {
        float new = *in++ + coef * last;
        *out++ = normal * (new - last);
        last = new;
    }
    
    x->lastValue = last;

}
