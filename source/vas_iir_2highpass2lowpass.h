/**
 * @file vas_iir_2highpass2lowpass.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief 2 Highpass and 2 Lowpass filter <br>
 *
 */

#ifndef vas_iir_2highpass2lowpass_h
#define vas_iir_2highpass2lowpass_h

#include "vas_mem.h"
#include "vas_util.h"
#include "vas_iir_lowpass.h"
#include "vas_iir_highpass.h"

typedef struct vas_iir_2highpass2lowpass
{
    float hpCutoff;
    float lpCutoff;
    vas_iir_highpass *hp1;
    vas_iir_highpass *hp2;
    vas_iir_lowpass *lp1;
    vas_iir_lowpass *lp2;
    AK_INPUTVECTOR tmp1[1024];
    AK_OUTPUTVECTOR tmp2[1024];
    
} vas_iir_2highpass2lowpass;

vas_iir_2highpass2lowpass *vas_iir_2highpass2lowpass_new(float hpCutoff, float lpCutoff);

void vas_iir_2highpass2lowpass_setHighpassCutoff(vas_iir_2highpass2lowpass *x, float cutoff);

void vas_iir_2highpass2lowpass_setLowpassCutoff(vas_iir_2highpass2lowpass *x, float cutoff);

void vas_iir_2highpass2lowpass_process(vas_iir_2highpass2lowpass *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize);


#endif
