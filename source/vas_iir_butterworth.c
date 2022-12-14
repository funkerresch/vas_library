//
//  vas_iir_butterworth.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 22.01.21.
//

#include "vas_iir_butterworth.h"
#include "vas_mem.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

    void vas_iir_butterworth_process(vas_iir_butterworth *x, VAS_INPUTBUFFER *data, int n) {
        if(x->fc >= x->sampleRate / 2)
            return;
        if(x->fc <= 0)
        {
            vas_util_writeZeros(n, data);
            return;
        }
        
        float *in = data;
        float y;
        while(n--) {
            
            y = *in * x->b0 + x->xz1 * x->b1 + x->yz1 * x->a1;
            
            x->xz1 = *in;
            
            *in = x->yz1 = y;
            
            in++;
        }
    }
    
    void vas_iir_butterworth_setFrequency(vas_iir_butterworth *x, float f) {
        if(f == x->fc) return;
        x->fc = f;
        float w = 2.0f * x->sampleRate;
        float Norm;
        float fw = w * tanf( (2*(float)M_PI*f) / w ); //pre-warp cutoff frequency and convert to rad/s
        Norm = 1.0f / (fw + w);
        x->a1 = (w - fw) * Norm;
        x->b0 = x->b1 = (fw * Norm);
    }
    
    vas_iir_butterworth *vas_iir_butterworth_new(float sampleRate) {
        vas_iir_butterworth *x = vas_mem_alloc(sizeof(vas_iir_butterworth));
        x->sampleRate = sampleRate;
        
        x->fc = sampleRate / 2;
        
        x->a1 = 0.0f;
        x->b0 = 1.0f;
        x->b1 = 0.0f;
        
        x->xz1 = 0.0f;
        x->yz1 = 0.0f;
        
        return x;
    }
    
    void vas_iir_butterworth_free(vas_iir_butterworth *x) {
        vas_mem_free(x);
    }

#ifdef __cplusplus
}
#endif
