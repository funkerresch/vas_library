/**
 * @file vas_fir_binaraural.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Convenience Class for dynamic 5.1 binaural synthesis <br>
 *
 * A Dynamic 5.1 Source to Binaural Filter Class <br>
 * Performs an 5.1-input-to-left and 5.1-input-to-right convolution for <br>
 * binaural synthesis. Convolution is performed (for now) with an equal partition <br>
 * size.
 *
 */

#ifndef vas_filter_binaural_51h
#define vas_filter_binaural_51h

#include "math.h"
#include "vas_fir.h"

#define left2left left
#define left2right right

typedef struct vas_fir_binaural_51
{
    vas_fir_metaData description;
    vas_dynamicFirChannel *left2left;
    vas_dynamicFirChannel *left2right;
    vas_dynamicFirChannel *center2left;
    vas_dynamicFirChannel *center2right;
    vas_dynamicFirChannel *right2left;
    vas_dynamicFirChannel *right2right;
    vas_dynamicFirChannel *surroundLeft2left;
    vas_dynamicFirChannel *surroundLeft2right;
    vas_dynamicFirChannel *surroundRight2left;
    vas_dynamicFirChannel *surroundRight2right;
    
} vas_fir_binaural_51;

vas_fir_binaural_51 *vas_fir_binaural_51_new(int setup, int segmentSize, vas_dynamicFirChannel_config *firConfig);
void vas_fir_binaural_51_free(vas_fir_binaural_51 *x);
void vas_fir_binaural_51_process(vas_fir_binaural_51 *x, AK_INPUTVECTOR *in[6], AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize);

#endif
