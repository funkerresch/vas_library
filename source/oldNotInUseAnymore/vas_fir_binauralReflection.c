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
    x->binauralEngine = vas_fir_binaural_new(0);
    x->scale = 1.0;
    x->delay = vas_delay_crossfade_new(maxDelayTime);
    x->filterHP = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 500, 20);
    x->filterLP = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWPASS, 17000, 20);
    vas_utilities_writeZeros(VAS_MAXVECTORSIZE, x->tmp);

    return x;
}

void vas_fir_binauralReflection_setDistance(vas_fir_binauralReflection *x, float distance)
{
    float tmp = distance * 0.001;
    x->scale = 1 - tmp;
}

void vas_fir_binauralReflection_setScaling(vas_fir_binauralReflection *x, float scaling)
{
    x->scale = scaling;
}

void vas_fir_binauralReflection_clear(vas_fir_binauralReflection *x)
{
    vas_delay_crossfade_clear(x->delay);
}

void vas_fir_binauralReflection_setHighPassFrequency(vas_fir_binauralReflection *x, float frequency)
{
    vas_iir_biquad_setFrequency(x->filterHP, frequency);
}

void vas_fir_binauralReflection_setLowPassFrequency(vas_fir_binauralReflection *x, float frequency)
{
    vas_iir_biquad_setFrequency(x->filterLP, frequency);
}

void vas_fir_binauralReflection_setMaterial(vas_fir_binauralReflection *x, int material)
{
    switch(material)
    {
        case UNDEFINED:
            vas_fir_binauralReflection_setHighPassFrequency(x, 10);
            vas_fir_binauralReflection_setLowPassFrequency(x, 20000);
            break;
            
        case CONCRETE:
            vas_fir_binauralReflection_setHighPassFrequency(x, 300);
            vas_fir_binauralReflection_setLowPassFrequency(x, 19000);
            break;
            
        case MIXED:
            vas_fir_binauralReflection_setHighPassFrequency(x, 1200);
            vas_fir_binauralReflection_setLowPassFrequency(x, 5000);
            break;
            
        case WOOD:
            vas_fir_binauralReflection_setHighPassFrequency(x, 300);
            vas_fir_binauralReflection_setLowPassFrequency(x, 800);
            break;
            
        case TEXTILE:
            vas_fir_binauralReflection_setHighPassFrequency(x, 300);
            vas_fir_binauralReflection_setLowPassFrequency(x, 300);
            break;
        default:
            vas_fir_binauralReflection_setHighPassFrequency(x, 10);
            vas_fir_binauralReflection_setLowPassFrequency(x, 20000);
            break;
    }
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
    vas_util_fcopy(in, x->tmp, vectorSize);
    vas_iir_biquad_process(x->filterHP, in, x->tmp, vectorSize);
    vas_iir_biquad_process(x->filterLP, x->tmp, x->tmp, vectorSize);
    vas_delay_crossfade_process(x->delay, x->tmp, x->tmp, vectorSize);
    //vas_util_fscale(x->tmp, x->scale, vectorSize);  // simplified Air Absorbtion
    vas_util_fcopy(x->tmp, in, vectorSize);
    vas_fir_binaural_processOutputInPlace(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

void vas_fir_binauralReflection_process_mute(vas_fir_binauralReflection *x, float *in, int vectorSize)
{
    vas_iir_biquad_process(x->filterHP, in, x->tmp, vectorSize);
    vas_iir_biquad_process(x->filterLP, x->tmp, x->tmp, vectorSize);
    vas_delay_crossfade_process(x->delay, x->tmp, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);  // simplified Air Absorbtion
    vas_util_fcopy(x->tmp, in, vectorSize);
}

void vas_fir_binauralReflection_free(vas_fir_binauralReflection *x)
{
    vas_mem_free(x->binauralEngine);
    vas_mem_free(x->filterHP);
    vas_mem_free(x->filterLP);
    vas_mem_free(x->delay);
    vas_mem_free(x);
}
