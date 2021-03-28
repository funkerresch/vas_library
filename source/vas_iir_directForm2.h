//
//  vas_iir_directForm2.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Harvey Keitel on 25.02.21.
//

#ifndef vas_iir_directForm2_h
#define vas_iir_directForm2_h

#include <stdio.h>
#include "vas_util.h"

#include "vas_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_iir_directForm2{
    float fc;
    float sampleRate;
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
    
    VAS_INPUTBUFFER vz1;
    VAS_INPUTBUFFER vz2;
    
    VAS_INPUTBUFFER yz1;
} vas_iir_directForm2;

void vas_iir_directForm2_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n);
vas_iir_directForm2 *vas_iir_directForm2_new(float a1, float a2, float b0, float b1, float b2) ;
void vas_iir_directForm2_free(vas_iir_directForm2 *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_iir_directForm2_h */
