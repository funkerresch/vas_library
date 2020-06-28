//
//  vas_iir_biquad.c
//  tr.hproom1~
//
//  Created by Admin on 15.04.18.
//

#include "vas_iir_biquad.h"

vas_iir_biquad *vas_iir_biquad_new(int type, float f0, float Q)
{
    vas_iir_biquad *x = (vas_iir_biquad *) vas_mem_alloc(sizeof(vas_iir_biquad));
    x->sampleRate = 44100;
    x->lastOut = 0;
    x->lastLastOut = 0;
    x->lastIn = 0;
    x->lastLastIn = 0;
    x->f0 = f0;
    x->Q = Q;
    x->filterType = type;
    x->gain = 1;
    vas_iir_biquad_setFrequency(x, f0);
    return x;
}

void vas_iir_biquad_free(vas_iir_biquad *x)
{
    vas_mem_free(x);
}

void vas_iir_biquad_setSampleRate(vas_iir_biquad *x, float sampleRate)
{
    x->sampleRate = sampleRate;
    vas_iir_biquad_setFrequency(x, x->f0);
}

void vas_iir_biquad_setFilterType(vas_iir_biquad *x, int type)
{
    x->filterType = type;
    vas_iir_biquad_setFrequency(x, x->f0);
}

void vas_iir_biquad_setQ(vas_iir_biquad *x, float Q)
{
    x->Q = Q;
    vas_iir_biquad_setFrequency(x, x->f0);
}

void vas_iir_biquad_setFrequency(vas_iir_biquad *x, float f0)
{
    x->f0 = f0;
    x->A = sqrtf(powf(x->gain/20, 10));
    x->w0 = 2*M_PI*x->f0/x->sampleRate;
    x->cosW0 = cosf(x->w0);
    x->sinW0 = sinf(x->w0);
    x->alpha = x->sinW0/2*x->Q;
    
    switch (x->filterType)
    {
        case VAS_IIR_BIQUAD_LOWPASS:
            x->a0 = 1 + x->alpha;
            x->a1 = -2*x->cosW0;
            x->a2 = 1 - x->alpha;
            x->b0 = (1 - x->cosW0)/2;
            x->b1 = 1 - x->cosW0;
            x->b2 = (1 - x->cosW0)/2;
            break;

        case VAS_IIR_BIQUAD_HIGHPASS:
            x->a0 = 1 + x->alpha;
            x->a1 = -2*x->cosW0;
            x->a2 = 1 - x->alpha;
            x->b0 = (1 + x->cosW0)/2;
            x->b1 = -(1 + x->cosW0);
            x->b2 = (1 + x->cosW0)/2;
            break;

        case VAS_IIR_BIQUAD_LOWSHELF:
            x->b0 = x->A*((x->A+1) - (x->A-1)*x->cosW0 + 2*sqrtf(x->A)*x->alpha);
            x->b1 = 2*x->A*((x->A-1) - (x->A+1)*x->cosW0);
            x->b2 = x->A*((x->A+1) - (x->A-1)*x->cosW0 - 2*sqrtf(x->A)*x->alpha);
            x->a0 = (x->A+1) + (x->A-1)*x->cosW0 + 2*sqrtf(x->A)*x->alpha;
            x->a1 = -2*((x->A-1) + (x->A+1)*x->cosW0);
            x->a2 = (x->A+1) + (x->A-1)*x->cosW0 - 2*sqrtf(x->A)*x->alpha;
            break;

        case VAS_IIR_BIQUAD_HIGHSHELF:
            x->b0 = x->A*((x->A+1) + (x->A-1)*x->cosW0 + 2*sqrtf(x->A)*x->alpha);
            x->b1 = -2*x->A*((x->A-1) + (x->A+1)*x->cosW0);
            x->b2 = x->A*((x->A+1) + (x->A-1)*x->cosW0 - 2*sqrtf(x->A)*x->alpha);
            x->a0 = (x->A+1) - (x->A-1)*x->cosW0 + 2*sqrtf(x->A)*x->alpha;
            x->a1 = 2*((x->A-1) - (x->A+1)*x->cosW0);
            x->a2 = (x->A+1) - (x->A-1)*x->cosW0 - 2*sqrtf(x->A)*x->alpha;
            break;

        case VAS_IIR_BIQUAD_PK:
            x->a0 = 1 + x->alpha/x->A;
            x->a1 = -2 * x->cosW0;
            x->a2 = 1 - x->alpha/x->A;
            x->b0 = 1 + x->alpha*x->A;
            x->b1 = -2 * x->cosW0;
            x->b2 = 1 - x->alpha*x->A;
            break;
    
            default:
                break;
        }
        
        x->b0_over_a0 = x->b0/x->a0;
        x->a1_over_a0 = x->a1/x->a0;
        x->a2_over_a0 = x->a2/x->a0;
        x->b1_over_a0 = x->b1/x->a0;
        x->b2_over_a0 = x->b2/x->a0;
}

void vas_iir_biquad_process(vas_iir_biquad *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize)
{
    int n = 0;
    float lastOut = x->lastOut;
    float lastLastOut = x->lastLastOut;
    float lastIn = x->lastIn;
    float lastLastIn = x->lastLastIn;
    float currentIn;
    float currentOut;
    
    while(n++ < vectorSize)
    {
        currentIn = *in++;
        
        currentOut = x->b0_over_a0 * currentIn + x->b1_over_a0 * lastIn + x->b2_over_a0 * lastLastIn
               - x->a1_over_a0 * lastOut - x->a2_over_a0 * lastLastOut;
        
        *out++ = currentOut;
        
        lastLastOut = lastOut;
        lastOut = currentOut;
        lastLastIn = lastIn;
        lastIn = currentIn;
    }
    
    x->lastLastIn = lastLastIn;
    x->lastIn = lastIn;
    x->lastLastOut = lastLastOut;
    x->lastOut = lastOut;
}



