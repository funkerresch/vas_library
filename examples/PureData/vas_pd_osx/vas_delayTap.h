//
//  vas_delayTap.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 23.12.20.
//

#ifndef vas_delayTap_h
#define vas_delayTap_h

#include <stdio.h>
#include "vas_ringBuffer.h"

/**
 * @struct vas_delay
 * @brief A structure for a delay object <br>
 * @var vas_delay::buffer The buffer we save the incoming signal in <br>
 * @var vas_delay::delay_in_samples The parameter value for adjusting the <br>
 * delay of the incoming signal
 * @var vas_delay::buffer_size The size of the delay buffer <br>
 * @var vas_delay::circular_pointer Circular pointer to the delay buffer <br>
 * @var vas_delay::delay_sample The current sample from the delay buffer <br>
 */
typedef struct vas_delayTap
{
    vas_ringBuffer *ringBuffer;
    float *buffer;              /**< Our delay buffer */
    int delayInSamples;     /**< Our delay in samples */
    int bufferSize;           /**< Size of the delay buffer */
    int readIndex;
    float *readPointer;

} vas_delayTap;

/**
 * @related vas_delay
 * @brief Creates a new delay object<br>
 * The function sets the buffer size and delay parameter of <br>
 * the delay class
 * @return a pointer to the newly created vas_delay object <br>
 */
vas_delayTap *vas_delayTap_new(vas_ringBuffer *ringBuffer);

/**
 * @related vas_delay
 * @brief Frees a delay object<br>
 * @param x My delay object <br>
 * The function frees the allocated memory<br>
 * of a delay object
 */
void vas_delayTap_free(vas_delayTap *x);

/**
 * @related vas_delay
 * @brief Performs the delay in realtime. <br>
 * @param x My delay object <br>
 * @param in The input vector <br>
 * @param out The output vector <br>
 * @param vector_size The size of the i/o vectors <br>
 * The function vas_delay_perform delays any <br>
 * incoming signal and copies the result to the output vector <br>
 */
void vas_delayTap_process(vas_delayTap *x, float *out, int vector_size);

/**
 * @related vas_delay
 * @brief Performs the delay in realtime in place. <br>
 * @param x My delay object <br>
 * @param in The input vector <br>
 * @param out The output vector <br>
 * @param vector_size The size of the i/o vectors <br>
 * The function vas_delay_perform delays any <br>
 * incoming signal and copies the result to the output vector <br>
 */
void vas_delayTap_processInPlace(vas_delayTap *x, float *out, int vector_size);

/**
 * @related vas_delay
 * @brief Sets the delay time in samples with floating point precision. <br>
 * @param x My delay object <br>
 * @param _delay_in_samples The delay in samples <br>
 * Sets the delay time in samples with floating point precision. <br>
 * Delays exceeding the buffer size are handeled by setting the delay to zero. <br>
 */
void vas_delayTap_set(vas_delayTap *x, float _delay_in_samples);


#endif /* vas_delayTap_h */
