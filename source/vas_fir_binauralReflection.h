//
//  vas_fir_binauralReflection.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#ifndef vas_fir_binauralReflection_h
#define vas_fir_binauralReflection_h

#include <stdio.h>
#include "vas_fir_binaural.h"
#include "vas_iir_biquad.h"
#include "vas_delay_crossfade.h"

#ifdef __cplusplus
extern "C" {
#endif
     
typedef struct vas_fir_binauralReflection {
    vas_fir_binaural *binauralEngine;
    vas_delay_crossfade *delay;
    vas_iir_biquad *filterHP;
    vas_iir_biquad *filterLP;
    float tmp[VAS_MAXVECTORSIZE];
} vas_fir_binauralReflection;

typedef enum {
    UNDEFINED,
    CONCRETE,
    MIXED,
    WOOD,
    TEXTILE
} vas_reflectionMaterial;
    
vas_fir_binauralReflection *vas_fir_binauralReflection_new(vas_fir_binaural *mainSource, long maxDelayTime);
    
void vas_fir_binauralReflection_setDelayTime(vas_fir_binauralReflection *x, float delayTime);
    
void vas_fir_binauralReflection_setAzimuth(vas_fir_binauralReflection *x, float azimuth);
    
void vas_fir_binauralReflection_setElevation(vas_fir_binauralReflection *x, float elevation);

void vas_fir_binauralReflection_setHighPassFrequency(vas_fir_binauralReflection *x, float frequency);

void vas_fir_binauralReflection_setLowPassFrequency(vas_fir_binauralReflection *x, float frequency);

void vas_fir_binaural_setReflectionMaterial(vas_fir_binauralReflection *x, vas_reflectionMaterial material);
    
void vas_fir_binauralReflection_process(vas_fir_binauralReflection *x, float *in, float *outL, float *outR, int vectorSize);
    
void vas_fir_binauralReflection_free(vas_fir_binauralReflection *x);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_fir_binauralReflection_h */
