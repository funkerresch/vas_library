/**
 * @file vas_fir_staticFir_m2s.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Convenience Class for stereo-2-stereo static convolution (most likely a reverb) <br>
 *
 */

#include "vas_dynamicFirChannel.h"
#include "vas_fir.h"

typedef struct vas_fir_static_s2s
{
    vas_fir_metaData description;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;
    vas_dynamicFirChannel *right2left;
    vas_dynamicFirChannel *left2right;
} vas_fir_static_s2s;

vas_fir_static_s2s *vas_fir_static_s2s_new(int setup, int segmentSize);

void vas_fir_static_s2s_process(vas_fir_static_s2s *x, AK_INPUTVECTOR *inLeft, AK_INPUTVECTOR *inRight, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize);

void vas_fir_static_s2s_free(vas_fir_static_s2s *x);


