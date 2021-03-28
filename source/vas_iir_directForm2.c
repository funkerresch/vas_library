//
//  vas_iir_directForm2.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Harvey Keitel on 25.02.21.
//

#include "vas_iir_directForm2.h"

vas_iir_directForm2 *vas_iir_directForm2_new(float a1, float a2, float b0, float b1, float b2) {
    vas_iir_directForm2 *x = vas_mem_alloc(sizeof(vas_iir_directForm2));
    
    x->a1 = a1;
    x->a2 = a2;
    
    x->b0 = b0;
    x->b1 = b1;
    x->b2 = b2;
    
    x->vz1 = 0;
    x->vz2 = 0;
     
    return x;
}

void vas_iir_vas_iir_directForm2_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n) {
    float *in = data;
    float v;
    float y;
    
    while(n--) {
        v = *in - x->a1 * x->vz1 - x->a2 * x->vz2;
        y = x->b0 * v + x->b1 * x->b1 * x->vz1 + x->b2 *x->vz2;
        x->vz2 = x->vz1;
        x->vz1 = v;
        in++;
    }
}

void vas_iir_directForm2_free(vas_iir_directForm2 *x)
{
    vas_mem_free(x);
}
