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
#include "vas_airAbsorption.h"
#include "vas_iir_directForm2.h"

#ifdef VAS_USE_MULTITHREADREFLECTION
#include "vas_threads.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_fir_binauralReflection1 vas_fir_binauralReflection1;

struct vas_fir_binauralReflection1 {
    float *tmp;
    vas_fir_binaural *binauralEngine;
    vas_delayTap_crossfade *delayTap;
    vas_airAbsorptionFilter *airAbsorption;
    vas_iir_directForm2 *materialAbsorption;
    
    float testCoeffs[5];
    
    float last;
    float lastLast;
    float scale;
};

#ifdef VAS_USE_MULTITHREADREFLECTION

typedef struct vas_threadedReflectionArg
{
    vas_threads_job *job;
    vas_threads_lfnode *node;
    vas_fir_binauralReflection1 **x; //reflectionCount
    int reflectionCount;
    int vectorSize;
    float *in;
    float *outputL;
    float *outputR;
    int *jobQueue;
    int extra1;
} vas_threadedReflectionArg;

void doThreadedWork(void *args);
void doThreadedWork_noMaterialNoAir(void *args);

#endif
    
vas_fir_binauralReflection1 *vas_fir_binauralReflection1_new(vas_fir_binaural *mainSource, vas_ringBuffer *delayBuffer);

void vas_fir_binauralReflection1_setReflectionBuffer(vas_fir_binauralReflection1 *x, vas_ringBuffer *delayBuffer);
    
void vas_fir_binauralReflection1_setDelayTime(vas_fir_binauralReflection1 *x, float delayTime);

void vas_fir_binauralReflection1_setScaling(vas_fir_binauralReflection1 *x, float scaling);
    
void vas_fir_binauralReflection1_setAzimuth(vas_fir_binauralReflection1 *x, float azimuth);
    
void vas_fir_binauralReflection1_setElevation(vas_fir_binauralReflection1 *x, float elevation);

void vas_fir_binauralReflection1_process_first(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize);
    
void vas_fir_binauralReflection1_process(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize);

void vas_fir_binauralReflection1_process_first_noMaterialNoAir(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize);

void vas_fir_binauralReflection1_process_noMaterialNoAir(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize);
    
void vas_fir_binauralReflection1_free(vas_fir_binauralReflection1 *x);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_fir_binauralReflection1_h */
