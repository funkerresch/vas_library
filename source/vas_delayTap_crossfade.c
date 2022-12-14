//
//  vas_delayTap_crossfade.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#include "vas_delayTap_crossfade.h"

vas_delayTap_crossfade *vas_delayTap_crossfade_new(vas_ringBuffer *ringBuffer)
{
    vas_delayTap_crossfade *x = (vas_delayTap_crossfade *)vas_mem_alloc(sizeof(vas_delayTap_crossfade));
    x->current = vas_delayTap_new(ringBuffer);
    x->target = vas_delayTap_new(ringBuffer);
    x->delayTimeTmp = 0;
    x->fadeLength = 2048;
    x->maxdelayTapTime = ringBuffer->bufferSize;
    x->fadeOut = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->fadeIn = (float *)vas_mem_alloc(sizeof(float) * x->fadeLength);
    x->outputCurrent = (float *)vas_mem_alloc(sizeof(float) * VAS_MAXVECTORSIZE);
    x->outputTarget = (float *)vas_mem_alloc(sizeof(float) * VAS_MAXVECTORSIZE);
    x->fadeCounter = 0;
    x->startCrossfade = 0;
    vas_util_writeFadeOutArray(x->fadeLength, x->fadeOut);
    vas_util_writeFadeInArray(x->fadeLength, x->fadeIn);
    x->numberOfFramesForCrossfade = x->fadeLength/1024; //   GET BUFFERSIZE ON CREATING IAGS_SPAT
    return x;
}

void vas_delayTap_crossfade_setRingBuffer(vas_delayTap_crossfade *x, vas_ringBuffer *ringBuffer)
{
    vas_delayTap_setRingBuffer(x->current, ringBuffer);
    vas_delayTap_setRingBuffer(x->target, ringBuffer);
    x->maxdelayTapTime = ringBuffer->bufferSize;
}

void vas_delayTap_crossfade_setDelayTime(vas_delayTap_crossfade *x, float delayTime)
{
    if(abs((int)delayTime - x->delayTimeTmp) > 256) // prevents to many changes, resolves unclean fades..
        x->delayTimeTmp = delayTime;
}

void vas_delayTap_crossfade_updateDelayTime(vas_delayTap_crossfade *x)
{
    vas_delayTap_set(x->current, x->target->delayInSamples);
    vas_delayTap_set(x->target, x->delayTimeTmp);
}

void vas_delayTap_crossfade_clear(vas_delayTap_crossfade *x)
{
    memset(x->current, 0, x->maxdelayTapTime * sizeof(float));
    memset(x->target, 0, x->maxdelayTapTime * sizeof(float));
}

void vas_delayTap_crossfade_process(vas_delayTap_crossfade *x, float *out, int vectorSize)
{
    //x->numberOfFramesForCrossfade = x->fadeLength/vectorSize; // A lot of divisions, is done in new function() for now, but need to pass vectorsize there..
    
    if(!x->fadeCounter)
        vas_delayTap_crossfade_updateDelayTime(x);
    
    vas_delayTap_process(x->current, x->outputCurrent, vectorSize);
    
    if(x->current->delayInSamples != x->target->delayInSamples)
    {
        vas_delayTap_process(x->target,  x->outputTarget, vectorSize);
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

void vas_delayTap_crossfade_free(vas_delayTap_crossfade *x)
{
    vas_delayTap_free(x->current);
    vas_delayTap_free(x->target);
    vas_mem_free(x->fadeIn);
    vas_mem_free(x->fadeOut);
    vas_mem_free(x->outputCurrent);
    vas_mem_free(x->outputTarget);
    vas_mem_free(x);
}
