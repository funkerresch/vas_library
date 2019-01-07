/**
 * @file vas_fir_staticFir_m2s.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Convenience Class for mono-2-stereo static convolution (most likely a reverb)<br>
 *
 */

#ifndef ak_firFilter_h
#define ak_firFilter_h

#include "vas_dynamicFirChannel.h"
#include "vas_fir.h"

typedef struct vas_fir_static_m2s
{
    vas_fir_metaData description;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;
} vas_fir_static_m2s;

vas_fir_static_m2s *vas_filter_staticFir_m2s_new(int setup, int segmentSize);

void vas_fir_static_m2s_process(vas_fir_static_m2s *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize);

void vas_fir_static_m2s_free(vas_fir_static_m2s *x);

#endif /* ak_firFilter_h */
