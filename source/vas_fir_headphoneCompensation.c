//
//  vas_filter_headphoneCompensation.c
//  ak.binaural~
//
//  Created by Admin on 12.03.18.
//

#include "vas_fir_headphoneCompensation.h"

void vas_fir_headphoneCompensation_process(vas_fir_headphoneCompensation *x, VAS_INPUTBUFFER *inLeft, VAS_INPUTBUFFER *inRight, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, inLeft, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, inRight, outRight, vectorSize, 0);
}


