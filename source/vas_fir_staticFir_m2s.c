//
//  ak_firFilter.c
//  ak.binaural~
//
//  Created by Admin on 08.02.18.
//

#include "vas_fir_staticFir_m2s.h"

vas_fir_static_m2s *vas_filter_staticFir_m2s_new(int setup, int segmentSize)
{
    vas_fir_static_m2s *x = vas_mem_alloc(sizeof(vas_fir_static_m2s));
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
    vas_dynamicFirChannel_shareInputWith(x->right, x->left);
    
    return x;
}

void vas_filter_staticFir_m2s_process(vas_fir_static_m2s *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, 0);
}

void vas_filter_staticFir_m2s_free(vas_fir_static_m2s *x)
{
    vas_dynamicFirChannel_free(x->left);
    vas_dynamicFirChannel_free(x->right);
}
