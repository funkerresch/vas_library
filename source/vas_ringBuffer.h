//
//  vas_ringBuffer.h
//  vas_binauralrir_unity
//
//  Created by Thomas Resch on 23.12.20.
//

#ifndef vas_ringBuffer_h
#define vas_ringBuffer_h

#include <stdio.h>
#include <math.h>
#include "vas_mem.h"
#include "vas_util.h"

#define VAS_RINGBUFFER_MAXSIZE 100000
#define VAS_RINGBUFFER_MINVECTORSIZE 64

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct vas_ringBuffer
 * @brief A structure for a ring buffer obect<br>
 * @var vas_ringBuffer::buffer The buffer we save the incoming signal in <br>
 * @var vas_ringBuffer::bufferSize The size of the ring buffer. <br>
 * @var vas_ringBuffer::writeIndex The current write index. <br>
 * @var vas_ringBuffer::writePointer Circular pointer to the delay buffer <br>
 * @var vas_ringBuffer::tapCounter Number of objecs using the ring buffer.<br>
 */

typedef struct vas_ringBuffer
{
    float *buffer;              /**< Our delay buffer */
    int bufferSize;           /**< Size of the delay buffer */
    int writeIndex;
    float *writePointer;    /**< Circular pointer to delay buffer */
    int tapCounter;
} vas_ringBuffer;

/**
 * @related vas_delayTap
 * @brief Creates a new ring buffer object.<br>
 * @param size The size of the ring buffer to be created <br>
 * Creates a new ring buffer. Necessary object <br>
 * for the vas_delayTap object. <br>
 * @return a pointer to the newly created vas_ringBuffer object <br>
 */

vas_ringBuffer *vas_ringBuffer_new(int size);

/**
 * @related vas_delayTap
 * @brief Processing function for the ring buffer object.<br>
 * @param x Pointer to the ring buffer we want to  write to. <br>
 * @param in Pointer to the input signal we want to copy to the ring buffer. <br>
 * @param vectorSize Size of the input vector. <br>
 * Copies the vector in at the current position of the ring buffer x<br>
 */

void vas_ringBuffer_process(vas_ringBuffer *x, float *in, int vectorSize);

void vas_ringBuffer_free(vas_ringBuffer *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_ringBuffer_h */
