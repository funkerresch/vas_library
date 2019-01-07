/**
 * @file vas_fir_binaraural.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Dynamic Mono Source to Binaural Filter <br>
 *
 * A Dynamic Mono Source to Binaural Filter Class <br>
 * Performs an mono-input-to-left and mono-input-to-right convolution for <br>
 * binaural synthesis. Convolution is performed (for now) with an equal partition <br>
 * size.
 *
 */


#ifndef vas_filter_binaural_h
#define vas_filter_binaural_h

#include "math.h"
#include "vas_fir.h"

#ifdef __cplusplus
extern "C" {
#endif
  
typedef struct vas_fir_binaural
{
    vas_fir_metaData description;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;    
    
} vas_fir_binaural;

vas_fir_binaural *vas_fir_binaural_new(int flags, int segmentSize, vas_dynamicFirChannel_config *firConfig);
void vas_fir_binaural_free(vas_fir_binaural *x);
void vas_fir_binaural_process(vas_fir_binaural *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize);
void vas_fir_binaural_resetInput(vas_fir_binaural *x);
void vas_fir_binaural_setAzimuth(vas_fir_binaural *x, int azimuth);
void vas_fir_binaural_setElevation(vas_fir_binaural *x, int elevation);
    
#ifdef __cplusplus
}
#endif

#endif
