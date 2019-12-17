//
//  vas_delay_crossfade.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#include "vas_delay_crossfade.h"

vas_delay_crossfade *vas_delay_crossfade_new(long maxDelayTime)
{
    vas_delay_crossfade *x = (vas_delay_crossfade *)vas_mem_alloc(sizeof(vas_delay_crossfade));
    x->current = vas_delay_new(maxDelayTime);
    x->target = vas_delay_new(maxDelayTime);
    x->fadeLength = 8192;
    x->maxDelayTime = maxDelayTime;
    x->fadeOut = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->fadeIn = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->outputCurrent = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->outputTarget = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->fadeCounter = 0;
    vas_utilities_writeFadeOutArray(x->fadeLength, x->fadeOut);
    vas_utilities_writeFadeInArray(x->fadeLength, x->fadeIn);
    return x;
}

void vas_delay_crossfade_setDelayTime(vas_delay_crossfade *x, float delayTime)
{
    x->delayTimeTmp = delayTime;
}

void vas_delay_crossfade_updateDelayTime(vas_delay_crossfade *x)
{
    vas_delay_setDelayTime(x->current, x->target->delay_in_samples);
    vas_delay_setDelayTime(x->target, x->delayTimeTmp);
}

void vas_delay_crossfade_clear(vas_delay_crossfade *x)
{
    memset(x->current, 0, x->maxDelayTime * sizeof(float));
    memset(x->target, 0, x->maxDelayTime * sizeof(float));
}

void vas_delay_crossfade_process(vas_delay_crossfade *x, float *in, float *out, int vectorSize)
{
    x->numberOfFramesForCrossfade = x->fadeLength/vectorSize;
    
    if(!x->fadeCounter)
        vas_delay_crossfade_updateDelayTime(x);
    
    vas_delay_perform(x->current, in, x->outputCurrent, vectorSize);
    vas_delay_perform(x->target, in, x->outputTarget, vectorSize);
    
    if(x->current->delay_in_samples != x->target->delay_in_samples)
    {
        vas_util_fmultiply(x->fadeOut+x->fadeCounter*vectorSize, x->outputCurrent, x->outputCurrent, vectorSize);
        vas_util_fmultiply(x->fadeIn+x->fadeCounter*vectorSize, x->outputTarget, x->outputTarget, vectorSize);
        vas_util_fadd(x->outputCurrent, x->outputTarget, x->outputCurrent, vectorSize);
        
        x->fadeCounter++;
        
        if(x->fadeCounter == x->numberOfFramesForCrossfade)
        {
            x->fadeCounter = 0;
        }
    }
    vas_util_fcopy(x->outputCurrent, out, vectorSize);
}

void vas_delay_crossfade_free(vas_delay_crossfade *x)
{
    vas_mem_free(x->current);
    vas_mem_free(x->target);
    vas_mem_free(x->fadeIn);
    vas_mem_free(x->fadeOut);
    vas_mem_free(x);
}
