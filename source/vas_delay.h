/**
 * @file vas_delay.h
 * @author C.Jaedicke, A.Monciero, P.Schuhladen, F.Müller <br>
 * An interpolated delay <br>
 * <br>
 * @brief Audio Object for adding delay to the input<br>
 * <br>
 * vas_delay allows for delaying<br>
 * any incoming audio signal. <br>
 * <br>
 */

#ifndef vas_delay_h
#define vas_delay_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vas_mem.h"
#include "vas_util.h"

#ifdef __cplusplus
extern "C" {
#endif
    
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
typedef struct vas_delay
{
    float *buffer;              /**< Our delay buffer */
    long delay_in_samples;     /**< Our delay in samples */
    long targetDelayTime;
    long buffer_size;           /**< Size of the delay buffer */
    float *circular_pointer;    /**< Circular pointer to delay buffer */
    float delay_sample;         /**< The current sample from the delay buffer */
} vas_delay;

/**
 * @related vas_delay
 * @brief Creates a new delay object<br>
 * The function sets the buffer size and delay parameter of <br>
 * the delay class
 * @return a pointer to the newly created vas_delay object <br>
 */
vas_delay *vas_delay_new(long _buffer_size);

/**
 * @related vas_delay
 * @brief Frees a delay object<br>
 * @param x My delay object <br>
 * The function frees the allocated memory<br>
 * of a delay object
 */
void vas_delay_free(vas_delay *x);

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
void vas_delay_perform(vas_delay *x, float *in, float *out, int vector_size);

/**
 * @related vas_delay
 * @brief Sets the delay time in samples with floating point precision. <br>
 * @param x My delay object <br>
 * @param _delay_in_samples The delay in samples <br>
 * Sets the delay time in samples with floating point precision. <br>
 * Delays exceeding the buffer size are handeled by setting the delay to zero. <br>
 */
void vas_delay_setDelayTime(vas_delay *x, float _delay_in_samples);

/**
 * @related vas_delay
 * @brief Non-interpolating retrieval of samples in the delay buffer. <br>
 * @param buffer_size Size of the total delay buffer <br>
 * @param buffer_pointer Pointer to the delay buffer <br>
 * @param circular_pointer Circular pointer iterating through the buffer <br>
 * @param index Index of the sample we want to tap in <br>
 * Non-interpolating retrieval of samples in the delay buffer. <br>
 * @return A sample from the delay buffer. <br>
 */
float vas_delay_tap_into_buffer(long buffer_size, float *buffer_pointer, float *circular_pointer, int index);

/**
 * @related vas_delay
 * @brief Interpolating retrieval of samples in the delay buffer. <br>
 * @param buffer_size Size of the total delay buffer <br>
 * @param delay_in_samples Desired non-integer delay <br>
 * @param buffer_pointer Pointer to the delay buffer <br>
 * @param circular_pointer Circular pointer iterating through the buffer <br>
 * Interpolating retrieval of samples in the delay buffer. <br>
 * Delays exceeding the buffer size are handeled by setting the delay to zero. <br>
 * @return A interpolated sample from the delay buffer. <br>
 */
float vas_delay_interpolate_tap_into_buffer(vas_delay *x, long buffer_size, float delay_in_samples, float *buffer_pointer, float *circular_pointer);

/**
 * @related vas_delay
 * @brief Resets the circular pointer. <br>
 * @param buffer_size Size of the total delay buffer <br>
 * @param buffer_pointer Pointer to the delay buffer <br>
 * @param circular_pointer Circular pointer iterating through the buffer <br>
 * Resets the circular pointer to the start or end of the delay buffer. <br>
 */
void vas_delay_wrap_circular_pointer_to_start(long buffer_size, float *buffer_pointer, float **circular_pointer);

/**
 * @related vas_delay
 * @brief Decrements the circular pointer. <br>
 * @param buffer_size Size of the total delay buffer <br>
 * @param buffer_pointer Pointer to the delay buffer <br>
 * @param circular_pointer Circular pointer iterating through the buffer <br>
 * Decrements the circular pointer and calls the wrap function. <br>
 */
void vas_delay_decrement_circular_pointer(long buffer_size, float *buffer_pointer, float **circular_pointer);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_delay_h */
