//
//  vas_iir_biquad_crossfade.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 16.11.19.
//

#ifndef vas_iir_biquad_crossfade_h
#define vas_iir_biquad_crossfade_h

#include <stdio.h>
#include "vas_iir_biquad.h"

typedef struct vas_iir_biquad_crossfade
{
    float *fadeOut;
    float *fadeIn;
    float *outputCurrent;
    float *outputTarget;
    int fadeLength;
    vas_iir_biquad *current;
    vas_iir_biquad *target;
    int fadeCounter;
    int numberOfFramesForCrossfade;
} vas_iir_biquad_crossfade;

#endif /* vas_iir_biquad_crossfade_h */
