//
//  vas_fir_binauralReflection1.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#include "vas_fir_binauralReflection1.h"
extern int vas_thpool_globalQueue;

vas_fir_binauralReflection1 *vas_fir_binauralReflection1_new(vas_fir_binaural *mainSource, vas_ringBuffer *delayBuffer)
{
    vas_fir_binauralReflection1 *x = (vas_fir_binauralReflection1 *) vas_mem_alloc(sizeof(vas_fir_binauralReflection1));
    x->binauralEngine = vas_fir_binaural_new(0);
    x->scale = 1.0;
    x->delayTap = vas_delayTap_crossfade_new(delayBuffer);
    x->tmp = vas_mem_alloc(VAS_MAXVECTORSIZE * sizeof(float));
    vas_util_writeZeros(VAS_MAXVECTORSIZE, x->tmp);
    x->airAbsorption = vas_airAbsorptionFilter_new(48000); //TODOHB1
    x->materialAbsorption = vas_iir_directForm2_new(2); // currently Fixed to 2nd order, some materials need 3rd, some would work with 1st order
    return x;
}

void vas_fir_binauralReflection1_setReflectionBuffer(vas_fir_binauralReflection1 *x, vas_ringBuffer *delayBuffer)
{
    vas_delayTap_crossfade_setRingBuffer(x->delayTap, delayBuffer);
}

void vas_fir_binauralReflection1_setScaling(vas_fir_binauralReflection1 *x, float scaling)
{
    x->scale = scaling;
}

void vas_fir_binauralReflection1_setDelayTime(vas_fir_binauralReflection1 *x, float delayTime)
{
     vas_delayTap_crossfade_setDelayTime(x->delayTap, delayTime);
}

void vas_fir_binauralReflection1_setAzimuth(vas_fir_binauralReflection1 *x, float azimuth)
{
    vas_fir_binaural_setAzimuth(x->binauralEngine, azimuth);
}

void vas_fir_binauralReflection1_setElevation(vas_fir_binauralReflection1 *x, float elevation)
{
    vas_fir_binaural_setElevation(x->binauralEngine, elevation);
}

void vas_fir_binauralReflection1_process_first(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);
    vas_airAbsorbtionFilter_perform(x->airAbsorption, x->tmp, x->tmp, vectorSize);
    vas_iir_directForm2_process(x->materialAbsorption, x->tmp, vectorSize);
    vas_fir_binaural_process(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

void vas_fir_binauralReflection1_process(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);
    vas_airAbsorbtionFilter_perform(x->airAbsorption, x->tmp, x->tmp, vectorSize);
    vas_iir_directForm2_process(x->materialAbsorption, x->tmp, vectorSize);
    vas_fir_binaural_processOutputInPlace(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

void vas_fir_binauralReflection1_process_first_noMaterialNoAir(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);
    vas_fir_binaural_process(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

void vas_fir_binauralReflection1_process_noMaterialNoAir(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);
    vas_fir_binaural_processOutputInPlace(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

#ifdef VAS_USE_MULTITHREADREFLECTION

void doThreadedWork(void *args)
{
    if(args == NULL)
        return;
    
    vas_threadedReflectionArg * arg = (vas_threadedReflectionArg *)args;
    vas_fir_binauralReflection1 **reflections = arg->x;
    int reflectionCount = arg->reflectionCount;
    VAS_INPUTBUFFER *outL = arg->outputL;
    VAS_INPUTBUFFER *outR = arg->outputR;
    int vectorSize = arg->vectorSize;

    __LFQ_ADD_AND_FETCH(arg->jobQueue, 1);
    
    vas_fir_binauralReflection1_process_first(reflections[0], outL, outR, vectorSize); // we guarantee at least one reflection in the caller
    
    for(int i = 1; i < reflectionCount; i++)
        vas_fir_binauralReflection1_process(reflections[i], outL, outR, vectorSize);

    __LFQ_ADD_AND_FETCH(arg->jobQueue, -1);
}

void doThreadedWork_noMaterialNoAir(void *args)
{
    if(args == NULL)
        return;
    
    vas_threadedReflectionArg * arg = (vas_threadedReflectionArg *)args;
    vas_fir_binauralReflection1 **reflections = arg->x;
    int reflectionCount = arg->reflectionCount;
    VAS_INPUTBUFFER *outL = arg->outputL;
    VAS_INPUTBUFFER *outR = arg->outputR;
    int vectorSize = arg->vectorSize;
    
    __LFQ_ADD_AND_FETCH(arg->jobQueue, 1);
 
    vas_fir_binauralReflection1_process_first_noMaterialNoAir(reflections[0], outL, outR, vectorSize); // we guarantee at least one reflection in the caller
    
    for(int i = 1; i < reflectionCount; i++)
        vas_fir_binauralReflection1_process_noMaterialNoAir(reflections[i], outL, outR, vectorSize);

    __LFQ_ADD_AND_FETCH(arg->jobQueue, -1);
}

#endif

void vas_fir_binauralReflection1_free(vas_fir_binauralReflection1 *x)
{
    vas_fir_binaural_free(x->binauralEngine);
    vas_delayTap_crossfade_free(x->delayTap);
    vas_airAbsorbtionFilter_free(x->airAbsorption);
    vas_iir_directForm2_free(x->materialAbsorption);
    vas_mem_free(x);
}
