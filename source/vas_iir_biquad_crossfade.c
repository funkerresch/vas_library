//
//  vas_iir_biquad_crossfade.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 16.11.19.
//

#include "vas_iir_biquad_crossfade.h"

vas_iir_biquad_crossfade *vas_iir_biquad_crossfade_new(int type, float f0, float Q)
{
    vas_iir_biquad_crossfade *x = (vas_iir_biquad_crossfade *)vas_mem_alloc(sizeof(vas_iir_biquad_crossfade));
    x->current = vas_iir_biquad_new(type, f0,  Q);
    x->target = vas_iir_biquad_new( type,  f0,  Q);
    x->fadeLength = 1024;
    x->fadeOut = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->fadeIn = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->outputCurrent = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->outputTarget = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->fadeCounter = 0;
    vas_utilities_writeFadeOutArray(x->fadeLength, x->fadeOut);
    vas_utilities_writeFadeInArray(x->fadeLength, x->fadeIn);
    return x;
}
