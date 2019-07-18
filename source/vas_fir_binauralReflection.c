//
//  vas_fir_binauralReflection.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#include "vas_fir_binauralReflection.h"

vas_fir_binauralReflection *vas_fir_binauralReflection_new(vas_fir_binaural *mainSource, long maxDelayTime)
{
    vas_fir_binauralReflection *x = (vas_fir_binauralReflection *) vas_mem_alloc(sizeof(vas_fir_binauralReflection));
    x->binauralEngine = vas_fir_binaural_new(VAS_VDSP | VAS_BINAURALSETUP_STD | VAS_LOCALFILTER, 1024, NULL);
    vas_dynamicFirChannel_shareFilterWith(x->binauralEngine->left, mainSource->left);
    vas_dynamicFirChannel_shareFilterWith(x->binauralEngine->right, mainSource->right);
    
    x->delay = vas_delay_crossfade_new(maxDelayTime);
    x->filter = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 1000, 10);
    return x;
}

void vas_fir_binauralReflection_setHighPassFrequency(vas_fir_binauralReflection *x, float frequency)
{
    
}

void vas_fir_binauralReflection_setDelayTime(vas_fir_binauralReflection *x, float delayTime)
{
     vas_delay_crossfade_setDelayTime(x->delay, delayTime);
}

void vas_fir_binauralReflection_setAzimuth(vas_fir_binauralReflection *x, float azimuth)
{
    vas_fir_binaural_setAzimuth(x->binauralEngine, azimuth);
}

void vas_fir_binauralReflection_setElevation(vas_fir_binauralReflection *x, float elevation)
{
    vas_fir_binaural_setElevation(x->binauralEngine, elevation);
}

void vas_fir_binauralReflection_process(vas_fir_binauralReflection *x, float *in, float *outL, float *outR, int vectorSize)
{
    vas_iir_biquad_process(x->filter, in, x->tmp, vectorSize);
    vas_delay_crossfade_process(x->delay, x->tmp, x->tmp, vectorSize);
    vas_fir_binaural_processOutputInPlace(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

void vas_fir_binauralReflection_free(vas_fir_binauralReflection *x)
{
    vas_mem_free(x->binauralEngine);
    vas_mem_free(x->filter);
    vas_mem_free(x->delay);
    vas_mem_free(x);
}
