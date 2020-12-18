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
#include "vas_fir_list.h"

#ifdef __cplusplus
extern "C" {
#endif
 
typedef struct vas_fir_binaural
{
    vas_fir_metaData metaData;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;
    
} vas_fir_binaural;

typedef vas_fir_binaural vas_fir_reverb;

vas_fir_binaural *vas_fir_binaural_new(int flags);
void vas_fir_binaural_free(vas_fir_binaural *x);
void vas_fir_binaural_process(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize);
void vas_fir_binaural_processLeftChannel(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, int vectorSize);
void vas_fir_binaural_processRightChannel(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outRight, int vectorSize);
void vas_fir_binaural_setAzimuth(vas_fir_binaural *x, int azimuth);
void vas_fir_binaural_setElevation(vas_fir_binaural *x, int elevation);
void vas_fir_binaural_shareInput(vas_fir_binaural *x, vas_fir_binaural *sharedInput);
void vas_fir_binaural_shareFilter(vas_fir_binaural *x, vas_fir_binaural *sharedFilter);
void vas_fir_binaural_processOutputInPlace(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize);

#define vas_fir_reverb_new vas_fir_binaural_new
#define vas_fir_reverb_free vas_fir_binaural_free
#define vas_fir_reverb_process vas_fir_binaural_process
#define vas_fir_dynconv_process vas_fir_binaural_processLeftChannel
    
#ifdef __cplusplus
}
#endif

#endif
