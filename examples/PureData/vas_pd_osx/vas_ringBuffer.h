//
//  vas_ringBuffer.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 23.12.20.
//

#ifndef vas_ringBuffer_h
#define vas_ringBuffer_h

#include <stdio.h>
#include <math.h>
#include "vas_mem.h"
#include "vas_util.h"

#define VAS_RINGBUFFER_MAXSIZE 44100
#define VAS_RINGBUFFER_MINVECTORSIZE 64

typedef struct vas_ringBuffer
{
    float *buffer;              /**< Our delay buffer */
    int bufferSize;           /**< Size of the delay buffer */
    int writeIndex;
    float *writePointer;    /**< Circular pointer to delay buffer */
    int tapCounter;
} vas_ringBuffer;

vas_ringBuffer *vas_ringBuffer_new(int size);

void vas_ringBuffer_process(vas_ringBuffer *x, float *in, int vectorSize);

void vas_ringBuffer_free(vas_ringBuffer *x);

#endif /* vas_ringBuffer_h */
