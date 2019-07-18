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

#define VAS_IIR_BIQUAD_LOWPASS 1
#define VAS_IIR_BIQUAD_HIGHPASS 2

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
    
} vas_iir_biquad;

vas_iir_biquad *vas_iir_biquad_new(int type, float f0, float Q);
    
void vas_iir_biquad_free(vas_iir_biquad *x);

void vas_iir_biquad_setFrequency(vas_iir_biquad *x, float f0);

void vas_iir_biquad_process(vas_iir_biquad *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize);
    
#ifdef __cplusplus
}
#endif

#endif

