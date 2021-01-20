//
//  vas_fir_binauralReflection1.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#ifndef vas_fir_binauralReflection1_h
#define vas_fir_binauralReflection1_h

#include <stdio.h>
#include "vas_fir_binaural.h"
#include "vas_iir_biquad.h"
#include "vas_delayTap_crossfade.h"

#ifdef __cplusplus
extern "C" {
#endif
     
typedef struct vas_fir_binauralReflection1 {
    vas_fir_binaural *binauralEngine;
    vas_delayTap_crossfade *delayTap;
    vas_iir_biquad *filterHP;
    vas_iir_biquad *filterLP;
    float testCoeffs[5];
    float tmp[VAS_MAXVECTORSIZE];
    float tmp2[VAS_MAXVECTORSIZE];
//    float input[VAS_MAXVECTORSIZE];
    float last;
    float lastLast;
    float scale;
} vas_fir_binauralReflection1;


#define UNDEFINED 0
#define CONCRETE 1
#define MIXED 2
#define WOOD 3
#define TEXTILE 4

    
vas_fir_binauralReflection1 *vas_fir_binauralReflection1_new(vas_fir_binaural *mainSource, vas_ringBuffer *delayBuffer);
    
void vas_fir_binauralReflection1_setDelayTime(vas_fir_binauralReflection1 *x, float delayTime);

void vas_fir_binauralReflection1_setScaling(vas_fir_binauralReflection1 *x, float scaling);
    
void vas_fir_binauralReflection1_setAzimuth(vas_fir_binauralReflection1 *x, float azimuth);

void vas_fir_binauralReflection1_setDistance(vas_fir_binauralReflection1 *x, float distance);
    
void vas_fir_binauralReflection1_setElevation(vas_fir_binauralReflection1 *x, float elevation);

void vas_fir_binauralReflection1_setHighPassFrequency(vas_fir_binauralReflection1 *x, float frequency);

void vas_fir_binauralReflection1_setLowPassFrequency(vas_fir_binauralReflection1 *x, float frequency);

void vas_fir_binauralReflection1_setMaterial(vas_fir_binauralReflection1 *x, int material);
    
void vas_fir_binauralReflection1_process(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize);

void vas_fir_binauralReflection1_processMono(vas_fir_binauralReflection1 *x, float *out, int vectorSize);

void vas_fir_binauralReflection1_processStereo(vas_fir_binauralReflection1 *x, float *outL, float *outR, float scaleLeft, float scaleRight, int vectorSize);
    
void vas_fir_binauralReflection1_free(vas_fir_binauralReflection1 *x);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_fir_binauralReflection1_h */
