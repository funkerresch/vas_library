//
//  vas_delayTap.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 23.12.20.
//

#include "vas_delayTap.h"

vas_delayTap *vas_delayTap_new(vas_ringBuffer *ringBuffer)
{
    vas_delayTap *x = (vas_delayTap *)malloc(sizeof(vas_delayTap));
    x->ringBuffer = ringBuffer;
    x->bufferSize = x->ringBuffer->bufferSize;
    x->readPointer = x->ringBuffer->buffer;
    x->readIndex = 0;
    x->delayInSamples = 0;
    x->ringBuffer->tapCounter++;

    return x;
}

void vas_delayTap_free(vas_delayTap *x)
{
    x->ringBuffer->tapCounter--;
    free(x);
}

void vas_delayTap_processInPlace(vas_delayTap *x, float *out, int vectorSize)
{
    if(x->readIndex & x->bufferSize)
        x->readIndex = 0;
    
    vas_util_fadd(&x->readPointer[x->readIndex], out, out, vectorSize);
    x->readIndex+=vectorSize;
}

void vas_delayTap_process(vas_delayTap *x, float *out, int vectorSize)
{
    if( (x->readIndex+vectorSize) < x->bufferSize)
    {
        vas_util_fcopy(&x->readPointer[x->readIndex], out, vectorSize);
        x->readIndex+=vectorSize;
        return;
    }

    int n = vectorSize;
    while(n--)
    {
        *out++ = x->readPointer[x->readIndex++];
        if(x->readIndex & x->bufferSize)
            x->readIndex = 0;
    }
}

void vas_delayTap_set(vas_delayTap *x, float delayInSamples)
{
    x->delayInSamples = delayInSamples;
    if ( (delayInSamples < 0) || (delayInSamples >= x-> bufferSize) )
        x->delayInSamples = 0;

    x->readIndex = (x->bufferSize - (int)x->delayInSamples + x->ringBuffer->writeIndex) % x->bufferSize;
}
