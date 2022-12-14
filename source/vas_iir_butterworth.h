//
//  vas_iir_butterworth.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 22.01.21.
//

#ifndef vas_iir_butterworth_h
#define vas_iir_butterworth_h

#include <stdio.h>
#include "vas_util.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_iir_butterworth {
    float fc;
    float sampleRate;
    float a1;
    float b0;
    float b1;
    VAS_INPUTBUFFER xz1;
    VAS_INPUTBUFFER yz1;
} vas_iir_butterworth;

/**
 * \brief Processes an input buffer with a previously configured vas_iir_biquad struct. Complexity: O(n).
 *
 * @param x A pointer to a vas_iir_butterworth struct.
 * @param data The signal input and output buffer
 * @param n The number of samples to process.
 */
void vas_iir_butterworth_process(vas_iir_butterworth *x, VAS_INPUTBUFFER *data, int n);

/**
 * \brief Sets the center frequency of the filter. Note that calling this function causes all filter coefficients to be re-calculated.
 *
 * @param x A pointer to a vas_iir_butterworth struct.
 * @param f The new center frequency.
 */
void vas_iir_butterworth_setFrequency(vas_iir_butterworth *x, float f);

/**
 * \brief Allocates a new vas_iir_butterworth struct with the given parameters.
 *
 * @param sampleRate The sample rate the filter should operate on.
 * @return A pointer to the new vas_iir_butterworth object.
 */
vas_iir_butterworth *vas_iir_butterworth_new(float sampleRate);
void vas_iir_butterworth_free(vas_iir_butterworth *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_iir_butterworth_h */
