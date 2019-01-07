//
//  ak_firFilter.c
//  ak.binaural~
//
//  Created by Admin on 08.02.18.
//

#include "vas_fir_staticFir_s2s.h"

vas_fir_static_s2s *vas_fir_static_s2s_new(int setup, int segmentSize)
{
    vas_fir_static_s2s *x = vas_mem_alloc(sizeof(vas_fir_static_s2s));
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
    x->left2right = vas_dynamicFirChannel_new(leftSetup, segmentSize, NULL);
    x->right = vas_dynamicFirChannel_new(rightSetup, segmentSize, NULL);
    x->right2left = vas_dynamicFirChannel_new(rightSetup, segmentSize, NULL);
    
    vas_dynamicFirChannel_shareInputWith(x->left2right, x->left);
    vas_dynamicFirChannel_shareInputWith(x->right2left, x->right);
    
    vas_dynamicFirChannel_shareFilterWith(x->right2left, x->left);
    vas_dynamicFirChannel_shareFilterWith(x->left2right, x->right);
    
    return x;
}

void vas_fir_static_s2s_process(vas_fir_static_s2s *x, AK_INPUTVECTOR *inLeft, AK_INPUTVECTOR *inRight,AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, inLeft, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, inRight, outRight, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right2left, inRight, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->left2right, inLeft, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
}

void vas_fir_static_s2s_free(vas_fir_static_s2s *x)
{
    vas_dynamicFirChannel_free(x->left);
    vas_dynamicFirChannel_free(x->right);
}
