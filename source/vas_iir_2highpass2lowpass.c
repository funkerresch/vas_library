//
//  vas_iir_2highpass2lowpass.c
//  tr.hproom1~
//
//  Created by Admin on 14.04.18.
//

#include "vas_iir_2highpass2lowpass.h"

vas_iir_2highpass2lowpass *vas_iir_2highpass2lowpass_new(float hpCutoff, float lpCutoff)
{
    vas_iir_2highpass2lowpass *x = (vas_iir_2highpass2lowpass *) vas_mem_alloc(sizeof(vas_iir_2highpass2lowpass));
    x->hpCutoff = hpCutoff;
    x->lpCutoff = lpCutoff;
    x->hp1 = vas_iir_highpass_new(hpCutoff);
    x->hp2 = vas_iir_highpass_new(hpCutoff);
    x->lp1 = vas_iir_lowpass_new(lpCutoff);
    x->lp2 = vas_iir_lowpass_new(lpCutoff);
    
    vas_iir_highpass_setCutoff(x->hp1, x->hpCutoff);
    vas_iir_highpass_setCutoff(x->hp2, x->hpCutoff);
    vas_iir_lowpass_setCutoff(x->lp1, x->lpCutoff);
    vas_iir_lowpass_setCutoff(x->lp2, x->lpCutoff);
    return x;
}

void vas_iir_2highpass2lowpass_setHighpassCutoff(vas_iir_2highpass2lowpass *x, float cutoff)
{
    vas_iir_highpass_setCutoff(x->hp1, cutoff);
    vas_iir_highpass_setCutoff(x->hp2, cutoff);
}

void vas_iir_2highpass2lowpass_setLowpassCutoff(vas_iir_2highpass2lowpass *x, float cutoff)
{
    vas_iir_lowpass_setCutoff(x->lp1, cutoff);
    vas_iir_lowpass_setCutoff(x->lp2, cutoff);
}

void vas_iir_2highpass2lowpass_process(vas_iir_2highpass2lowpass *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize)
{
    vas_iir_highpass_process(x->hp1, in, x->tmp1, vectorSize);
    vas_iir_highpass_process(x->hp2, x->tmp1, x->tmp2, vectorSize);
    vas_iir_lowpass_process(x->lp1, x->tmp2, x->tmp1, vectorSize);
    vas_iir_lowpass_process(x->lp2, x->tmp1, out, vectorSize);
}
