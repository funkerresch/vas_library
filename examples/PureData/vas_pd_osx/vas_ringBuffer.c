//
//  vas_ringBuffer.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 23.12.20.
//

#include "vas_ringBuffer.h"

vas_ringBuffer *vas_ringBuffer_new(int size)
{
    vas_ringBuffer *x = (vas_ringBuffer *) malloc(sizeof(vas_ringBuffer));
    x->buffer = (float *) calloc (size , sizeof(float));
    int newSize = vas_utilities_roundUp2NextPowerOf2(size);
    x->bufferSize = vas_utilities_roundUp2NextPowerOf2(newSize);;
    x->writePointer = x->buffer;
    x->writeIndex = 0;
    x->tapCounter = 0;
    return x;
}

void vas_ringBuffer_free(vas_ringBuffer *x)
{
    if(!x->tapCounter)
    {
        free(x->buffer);
        free(x);
    }
}

void vas_ringBuffer_process(vas_ringBuffer *x, float *in, int vectorSize)
{
    x->writeIndex+=vectorSize;
    if(x->writeIndex & x->bufferSize)
        x->writeIndex = 0;
    
    vas_util_fcopy(in, &x->writePointer[x->writeIndex], vectorSize);
    
}
