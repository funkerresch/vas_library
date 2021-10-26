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
    
    int newSize = vas_utilities_roundUp2NextPowerOf2(size); //this should round up to the next multiple of vector size, not power of 2!
    x->buffer = vas_mem_alloc(newSize * sizeof(float));
    x->bufferSize = newSize;;
    x->writePointer = x->buffer;
    x->writeIndex = 0;
    x->tapCounter = 0;
    return x;
}

void vas_ringBuffer_free(vas_ringBuffer *x)
{
    if(!x->tapCounter)
    {
        vas_mem_free(x->buffer);
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
