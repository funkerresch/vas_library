/**
 * @file vas_fir_headphoneCompensation.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Convenience Class for Headphone Compensation <br>
 *
 * A Stereo Headphone Compensation Filter <br>
 * Performs a left-to-left and right-to-right channel convolution
 * with the loaded filter
 *
 */

#ifndef vas_filter_headphoneCompensation_h
#define vas_filter_headphoneCompensation_h

#include "vas_fir.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_fir_headphoneCompensation
{
    vas_fir_metaData metaData;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;
    
} vas_fir_headphoneCompensation;

void vas_fir_headphoneCompensation_process(vas_fir_headphoneCompensation *x, VAS_INPUTBUFFER *inLeft, VAS_INPUTBUFFER *inRight, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize);

#ifdef __cplusplus
}
#endif

#endif /* vas_filter_headphoneCompensation_h */
