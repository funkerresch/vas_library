//
//  vas_airAbsorptionSimple.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 19.01.21.
//

#ifndef vas_airAbsorptionSimple_h
#define vas_airAbsorptionSimple_h

#include "vas_airAbsorption.h"
#include "vas_util.h"
#include "vas_mem.h"
//#include "vas_iir_biquad.h"
#include "vas_iir_butterworth.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_airAbsorptionSimple {
    vas_airAbsorptionFilter *w; //the "parent" struct
    float a1; //intermediate result
    float a2; //intermediate result
    float a3; //intermediate result
    //vas_iir_biquad *filter; //TODO: this should be a 1-pole instead of a 2-pole filter
    vas_iir_butterworth *filter;
} vas_airAbsorptionSimple;

vas_airAbsorptionSimple *vas_airAbsorbtionSimple_new(vas_airAbsorptionFilter *w);
void vas_airAbsorbtionSimple_free(vas_airAbsorptionSimple *x);
void vas_airAbsorbtionSimple_perform(vas_airAbsorptionSimple *x, VAS_INPUTBUFFER* in, VAS_OUTPUTBUFFER* out, int vectorSize);
void vas_airAbsorptionSimple_update_d(vas_airAbsorptionSimple *x);
void vas_airAbsorbtionSimple_update_t_h_p(vas_airAbsorptionSimple *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_airAbsorptionSimple_h */
