/**
 * @file vas_fir_headphoneCompensation.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Convenience Class for Headphone Compensation <br>
 *
 * A Stereo Headphone Compensation Filter with 2048 samples length <br>
 * Performs a left-to-left and right-to-right channel convolution
 * with the loaded filter
 *
 */

#ifndef vas_filter_headphoneCompensation_h
#define vas_filter_headphoneCompensation_h

#include "vas_fir.h"

typedef struct vas_fir_headphoneCompensation
{
    vas_fir_metaData description;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;
    
} vas_filter_headphoneCompensation;

vas_filter_headphoneCompensation *vas_filter_headphoneCompensation_new(int setup, int segmentSize);

void vas_filter_headphoneCompensation_process(vas_filter_headphoneCompensation *x, AK_INPUTVECTOR *inLeft, AK_INPUTVECTOR *inRight, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize);

void vas_filter_headphoneCompensation_free(vas_filter_headphoneCompensation *x);

#endif /* vas_filter_headphoneCompensation_h */
