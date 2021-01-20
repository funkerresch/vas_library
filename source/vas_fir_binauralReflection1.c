//
//  vas_fir_binauralReflection1.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#include "vas_fir_binauralReflection1.h"

vas_fir_binauralReflection1 *vas_fir_binauralReflection1_new(vas_fir_binaural *mainSource, vas_ringBuffer *delayBuffer)
{
    vas_fir_binauralReflection1 *x = (vas_fir_binauralReflection1 *) vas_mem_alloc(sizeof(vas_fir_binauralReflection1));
    x->binauralEngine = vas_fir_binaural_new(0);
    x->scale = 1.0;
    x->delayTap = vas_delayTap_crossfade_new(delayBuffer);
    x->filterHP = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 500, 20);
    x->filterLP = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWPASS, 1500, 20);
    vas_utilities_writeZeros(VAS_MAXVECTORSIZE, x->tmp);
//    vas_utilities_writeZeros(VAS_MAXVECTORSIZE, x->input);
//    x->testCoeffs[0] = 0.742393813628057;
//    x->testCoeffs[1] = 0.605222264668768;
//    x->testCoeffs[2] = -0.007922046639901;
//    x->testCoeffs[3] = -0.867425583451915;
//    x->testCoeffs[4] = 0.009201931018148;

    //{ b0/a0, b1/a0, b2/a0, a1/a0, a2/a0 }
    

    return x;
}

void vas_fir_binauralReflection1_setDistance(vas_fir_binauralReflection1 *x, float distance)
{
    float tmp = distance * 0.001;
    x->scale = 1 - tmp;
}

void vas_fir_binauralReflection1_setScaling(vas_fir_binauralReflection1 *x, float scaling)
{
    x->scale = scaling;
}

void vas_fir_binauralReflection1_setHighPassFrequency(vas_fir_binauralReflection1 *x, float frequency)
{
    vas_iir_biquad_setFrequency(x->filterHP, frequency);
}

void vas_fir_binauralReflection1_setLowPassFrequency(vas_fir_binauralReflection1 *x, float frequency)
{
    vas_iir_biquad_setFrequency(x->filterLP, frequency);
}

void vas_fir_binauralReflection1_setMaterial(vas_fir_binauralReflection1 *x, int material)
{
    switch(material)
    {
        case UNDEFINED:
            vas_fir_binauralReflection1_setHighPassFrequency(x, 10);
            vas_fir_binauralReflection1_setLowPassFrequency(x, 20000);
            break;
            
        case CONCRETE:
            vas_fir_binauralReflection1_setHighPassFrequency(x, 300);
            vas_fir_binauralReflection1_setLowPassFrequency(x, 19000);
            break;
            
        case MIXED:
            vas_fir_binauralReflection1_setHighPassFrequency(x, 1200);
            vas_fir_binauralReflection1_setLowPassFrequency(x, 5000);
            break;
            
        case WOOD:
            vas_fir_binauralReflection1_setHighPassFrequency(x, 300);
            vas_fir_binauralReflection1_setLowPassFrequency(x, 800);
            break;
            
        case TEXTILE:
            vas_fir_binauralReflection1_setHighPassFrequency(x, 300);
            vas_fir_binauralReflection1_setLowPassFrequency(x, 300);
            break;
        default:
            vas_fir_binauralReflection1_setHighPassFrequency(x, 10);
            vas_fir_binauralReflection1_setLowPassFrequency(x, 20000);
            break;
    }
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

void vas_fir_binauralReflection1_process(vas_fir_binauralReflection1 *x, float *outL, float *outR, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);
    //vas_iir_biquad_process(x->filterHP, x->tmp, x->tmp, vectorSize);
    //vas_iir_biquad_process(x->filterLP, x->tmp, x->tmp, vectorSize);
    //vDSP_deq22(x->input, 1, x->testCoeffs, x->tmp, 1, vectorSize);
    
    vas_fir_binaural_processOutputInPlace(x->binauralEngine, x->tmp, outL, outR, vectorSize);
}

void vas_fir_binauralReflection1_processMono(vas_fir_binauralReflection1 *x, float *out, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale*8, vectorSize);
    //vas_iir_biquad_process(x->filterLP, x->tmp, x->tmp, vectorSize);
    vas_util_fadd(x->tmp, out, out, vectorSize);
}

void vas_fir_binauralReflection1_processStereo(vas_fir_binauralReflection1 *x, float *outL, float *outR, float scaleLeft, float scaleRight, int vectorSize)
{
    vas_delayTap_crossfade_process(x->delayTap, x->tmp, vectorSize);
    vas_util_fscale(x->tmp, x->scale, vectorSize);
    vas_iir_biquad_process(x->filterLP, x->tmp, x->tmp, vectorSize);
    vas_util_fmulitplyScalar(x->tmp, scaleLeft, x->tmp2, vectorSize);
    vas_util_fscale(x->tmp, scaleRight, vectorSize);
    vas_util_fadd(x->tmp, outL, outL, vectorSize);
    vas_util_fadd(x->tmp2, outR, outR, vectorSize);
}

void vas_fir_binauralReflection1_free(vas_fir_binauralReflection1 *x)
{
    vas_mem_free(x->binauralEngine);
    vas_mem_free(x->filterHP);
    vas_mem_free(x->filterLP);
    vas_mem_free(x->delayTap);
    vas_mem_free(x);
}
