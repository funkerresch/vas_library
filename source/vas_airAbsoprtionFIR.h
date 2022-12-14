//
//  vas_airAbsoprtionFIR.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 19.01.21.
//

#ifndef vas_airAbsoprtionFIR_h
#define vas_airAbsoprtionFIR_h

#include <math.h>
#include "vas_mem.h"
#include "vas_util.h"
#include "vas_airAbsorption.h"

#define VAS_AIRABS_USE_LOOKUP
#ifdef VAS_AIRABS_USE_LOOKUP
#include "vas_window_blackman.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_airAbsorptionFIR {
    vas_airAbsorptionFilter *w;
    VAS_COMPLEX *ir;
    int irLength;
} vas_airAbsorptionFIR;

vas_airAbsorptionFIR *vas_airAbsorptionFIR_new(vas_airAbsorptionFilter *w);
void vas_airAbsorptionFIR_free(vas_airAbsorptionFIR *x);
void vas_airAbsorptionFIR_perform(vas_airAbsorptionFIR *x, VAS_INPUTBUFFER* in, VAS_OUTPUTBUFFER* out, int vectorSize);
void vas_airAbsorbtionFIR_update_t_h_p(vas_airAbsorptionFIR *x);
void vas_airAbsorptionFIR_update_d(vas_airAbsorptionFIR *x);


#ifdef __cplusplus
}
#endif

#endif /* vas_airAbsoprtionFIR_h */
