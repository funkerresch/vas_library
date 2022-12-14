//
//  vas_airAbsorption.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 20.01.21.
//

#include <stdio.h>
#include "vas_airAbsorption.h"
#include "vas_mem.h"

#define VAS_AIRABS_SIMPLE
#ifdef VAS_AIRABS_SIMPLE
#include "vas_airAbsorptionSimple.h"
#else
#include "vas_airAbsoprtionFIR.h"
#endif

#ifndef MIN
#define MIN(a, b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define MAX(a, b) (((a)>(b))?(a):(b))
#endif  /* MAX */

#ifdef __cplusplus
extern "C" {
#endif

    //tested. works!
    void vas_airAbsorptionFilter_udpate_f_O_N(vas_airAbsorptionFilter *x) {
        double h = x->h * pow(10, -6.8346 * pow(T01 / x->t, 1.261) + 4.6151) / x->p;
        x->f_O = (float) ( 24 + 40400 * h * (0.02 + h) / (0.391 + h) / x->p );
        x->f_N = (float) ( sqrt(T0 / x->t) * (9 + 280 * h * exp(-4.17 * (pow(T0 / x->t, 1.0 / 3) - 1))) / x->p );
    }
    
    void vas_airAbsorptionFilter_validate(vas_airAbsorptionFilter *x) {
            if(x->h < VAS_AIRABS_MIN_H) x->h = VAS_AIRABS_MIN_H;
            else if(x->h > VAS_AIRABS_MAX_H) x->h = VAS_AIRABS_MAX_H;
            if(x->t < VAS_AIRABS_MIN_T) x->t = VAS_AIRABS_MIN_T;
            else if(x->t > VAS_AIRABS_MAX_T) x->t = VAS_AIRABS_MAX_T;
            if(x->p < VAS_AIRABS_MIN_P) x->p = VAS_AIRABS_MIN_P;
            else if(x->p > VAS_AIRABS_MAX_P) x->p = VAS_AIRABS_MAX_P;
    }
    
    //tested. works!
    //is it 0.01275 or 0.01278? literature is inconsistent
    double vas_airAbsorptionFilter_get_abs_coeff(vas_airAbsorptionFilter *x, float f) {
        float F = f/x->p;
        return 20 / log(10) * F * F * (1.84 * pow(10,-11) * sqrt(x->t/T0) +
            pow(x->t/T0,-2.5) * ( 0.01278*exp(-2239.1/x->t)/(x->f_O+F*F/x->f_O) + 0.1068*exp(-3352/x->t)/(x->f_N+F*F/x->f_N) )) * x->p;
    }
    
    void vas_airAbsorptionFilter_update_t_h_p(vas_airAbsorptionFilter *x, float t, float h, float p) {
        bool changed = t != x->t || h != x->h || p != x->p;
        if (!changed) return;
        vas_airAbsorptionFilter_validate(x);
        vas_airAbsorptionFilter_udpate_f_O_N(x);
        #ifdef VAS_AIRABS_SIMPLE
            vas_airAbsorbtionSimple_update_t_h_p((vas_airAbsorptionSimple*)x->filter_guts);
        #else
            vas_airAbsorbtionFIR_update_params((vas_airAbsorptionFIR*)x->filter_guts);
        #endif
    }
    
    void vas_airAbsorptionFilter_update_d(vas_airAbsorptionFilter *x, float distance) {
        if(distance == x->d) return;
            x->d = distance;
        #ifdef VAS_AIRABS_SIMPLE
            vas_airAbsorptionSimple_update_d((vas_airAbsorptionSimple*)x->filter_guts);
        #else
            vas_airAbsorptionFIR_distance_changed((vas_airAbsorptionFIR*)x->filter_guts);
        #endif
    }
    
    vas_airAbsorptionFilter *vas_airAbsorptionFilter_new(float sampleRate) {
        vas_airAbsorptionFilter *x = vas_mem_alloc(sizeof(vas_airAbsorptionFilter));
        x->damp = true;
        x->attenuate = true;
        x->t = T0;
        x->h = H0;
        x->p = P0;
        x->d = 1.0f;
        x->scaling = 1.0f;
        x->sampleRate = sampleRate;
        vas_airAbsorptionFilter_udpate_f_O_N(x);
        
        #ifdef VAS_AIRABS_SIMPLE
            x->filter_guts = (void*)vas_airAbsorbtionSimple_new(x);
        #else
            x->filter_guts = (void*)vas_airAbsorptionFIR_new(x);
        #endif
        return x;
    }
    
    void vas_airAbsorbtionFilter_free(vas_airAbsorptionFilter *x) {
        #ifdef VAS_AIRABS_SIMPLE
            vas_airAbsorbtionSimple_free((vas_airAbsorptionSimple*)x->filter_guts);
        #else
            vas_airAbsorptionFIR_free((vas_airAbsorptionFIR*)x->filter_guts);
        #endif
    }
    
    void vas_airAbsorptionFilter_update(vas_airAbsorptionFilter *x, float t, float h, float p, float d, float scaling, float shouldAttenuate, float shouldDamp) {
        x->attenuate = shouldAttenuate != 0.0f;
        x->damp = shouldDamp != 0.0f;
        x->scaling = scaling;
        vas_airAbsorptionFilter_update_t_h_p(x, t, h, p);
        vas_airAbsorptionFilter_update_d(x, d);
    }
    
    void vas_airAbsorbtionFilter_perform(vas_airAbsorptionFilter *x, VAS_INPUTBUFFER* in, VAS_OUTPUTBUFFER* out, int vectorSize) {
        if(in != out) {
            vas_util_fcopy(in, out, vectorSize);
        }
        
        if(x->damp) {
            #ifdef VAS_AIRABS_SIMPLE
                vas_airAbsorbtionSimple_perform((vas_airAbsorptionSimple*)x->filter_guts, out, out, vectorSize);
            #else
                vas_airAbsorptionFIR_perform((vas_airAbsorptionFIR*)x->filter_guts, out, out, vectorSize);
            #endif
        }
        
        if(x->attenuate) {
            float gain = x->d > 0 ? MIN((x->scaling / x->d), 1.0f) : 1.0f;
            vas_util_fscale(out, gain, vectorSize);
        }
    }

#ifdef __cplusplus
}
#endif
