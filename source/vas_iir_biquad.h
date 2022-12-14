/**
 * @file vas_highpass.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief Biquad Filter <br>
 * From Robert Bristow-Johnson's Webpage
 * http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 *
 */


#ifndef vas_biquad_h
#define vas_biquad_h

#include "vas_mem.h"
#include "vas_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VAS_IIR_BIQUAD_UNDEFINED 0
#define VAS_IIR_BIQUAD_LOWPASS 1
#define VAS_IIR_BIQUAD_HIGHPASS 2
#define VAS_IIR_BIQUAD_LOWSHELF 3
#define VAS_IIR_BIQUAD_HIGHSHELF 4
#define VAS_IIR_BIQUAD_PK 5

typedef struct vas_iir_biquad
{
    int filterType;
    float sampleRate;
    float f0;
    float lastOut;
    float lastLastOut;
    float lastIn;
    float lastLastIn;
    float Q;
    float bandwidth;
    float shelfSlope;
    float A;
    float w0;
    float cosW0;
    float sinW0;
    float alpha;
    float gain;
    float a0, a1, a2, b0, b1, b2;
    float b0_over_a0;
    float b1_over_a0;
    float b2_over_a0;
    float a1_over_a0;
    float a2_over_a0;
    bool legacy;
} vas_iir_biquad;

/**
 * \brief Allocates a new vas_iir_biquad struct with the given parameters.
 *
 * @param type The type of the filter.
 * @param f0 The center frequency of the filter.
 * @param Q The "quality factor" aka filter bandwidth. Note that if legacy biquad is used, a Q of ~10 results in a nearly flat filter response. Otherwise, a Q of 1 results in a flat response, as you may not it from other filters.
 * @param sampleRate The sample rate the filter should operate on.
 * @return A pointer to the new vas_iir_biquad object.
 */
vas_iir_biquad *vas_iir_biquad_new(int type, float f0, float Q, float sampleRate);
    
void vas_iir_biquad_free(vas_iir_biquad *x);

/**
 * \brief Sets the center frequency of the filter. Note that calling this function causes all filter coefficients to be re-calculated.
 *
 * @param x A pointer to a vas_iir_biquad struct.
 * @param f0 The new center frequency.
 */
void vas_iir_biquad_setFrequency(vas_iir_biquad *x, float f0);

void vas_iir_biquad_setParameters(vas_iir_biquad *x, float Q, float gain, float f);

/**
 * \brief In versions of IAGS prior to October 2021, there was a mistake in the biquad implementation. Software that uses IAGS, and is older than October 2021, should probably set this TRUE. Default is FALSE. Note that setting this parameter re-calculates the filter coefficients. Also note that setting this parameter resets Q to its default value, because there is no meaningful relation (and therefore no conversion) between the Q values of the old and the new implementation.
 *
 * @param x A pointer to a vas_iir_biquad struct.
 * @param legacy True or 1 if legacy implementation should be used. False or 0 if default implementation should be used.
 */

void vas_iir_biquad_setLegacy(vas_iir_biquad *x, bool legacy);

/**
 * \brief Processes an input buffer with a previously configured vas_iir_biquad struct and writes the result to an output buffer. Input and output buffer can be identical. Complexity: O(n).
 *
 * @param x A pointer to a vas_iir_biquad struct.
 * @param in The signal input buffer
 * @param out The signal output buffer.
 * @param vectorSize The number of samples to process.
 */
void vas_iir_biquad_process(vas_iir_biquad *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize);

//TODOHB1
/**
 * \brief Sets the sample rate of the filter. Default is 44100 kHz. Note that calling this causes a re-calculation of the filter coefficients.
 *
 * @param x A pointer to a vas_iir_biquad struct.
 * @param sampleRate The sample rate to use.
 */
void vas_iir_biquad_setSampleRate(vas_iir_biquad *x, float sampleRate);
    
#ifdef __cplusplus
}
#endif

#endif

