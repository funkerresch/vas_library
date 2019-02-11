//
//  vas_filter_headphoneCompensation.c
//  ak.binaural~
//
//  Created by Admin on 12.03.18.
//

#include "vas_fir_headphoneCompensation.h"

void vas_fir_headphoneCompensation_process(vas_fir_headphoneCompensation *x, AK_INPUTVECTOR *inLeft, AK_INPUTVECTOR *inRight, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, inLeft, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, inRight, outRight, vectorSize, 0);
}

vas_fir_headphoneCompensation *vas_fir_headphoneCompensation_new(int setup, int segmentSize)
{
    vas_fir_headphoneCompensation *x = vas_mem_alloc(sizeof(vas_fir_headphoneCompensation));
    vas_filter_metaData_init(&x->description);
    
    int leftSetup = setup;
    int rightSetup = setup;
    leftSetup |= VAS_STATICFILTER;
    rightSetup |= VAS_STATICFILTER;
    
    if(setup & VAS_GLOBALFILTER)
    {
        leftSetup |= VAS_GLOBALFILTER_LEFT;
        rightSetup |= VAS_GLOBALFILTER_RIGHT;
    }
    
    x->left = vas_dynamicFirChannel_new(leftSetup, segmentSize, NULL);
    x->right = vas_dynamicFirChannel_new(rightSetup, segmentSize, NULL);
    
    return x;
}

void vas_fir_headphoneCompensation_free(vas_fir_headphoneCompensation *x)
{
    vas_dynamicFirChannel_free(x->left);
    vas_dynamicFirChannel_free(x->right);
}

