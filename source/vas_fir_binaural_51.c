#include "vas_fir_binaural_51.h"

void vas_fir_binaural_51_process(vas_fir_binaural_51 *x, AK_INPUTVECTOR **in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left2left, in[0], outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->left2right, in[0], outRight, vectorSize, 0);
    
    vas_dynamicFirChannel_process(x->center2left, in[1], outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->center2right, in[1], outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_dynamicFirChannel_process(x->right2left, in[2], outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->right2right, in[2], outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_dynamicFirChannel_process(x->surroundLeft2left, in[3], outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->surroundLeft2right, in[3], outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_dynamicFirChannel_process(x->surroundRight2left, in[4], outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->surroundRight2right, in[4], outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    //LFE currently missing..
}

vas_fir_binaural_51 *vas_fir_binaural_51_new(int setup, int segmentSize, vas_dynamicFirChannel_config *firConfig)
{
    vas_fir_binaural_51 *x = vas_mem_alloc(sizeof(vas_fir_binaural_51));
    vas_filter_metaData_init(&x->description);
    int leftSetup = setup;
    int rightSetup = setup;
    
    if(setup & VAS_GLOBALFILTER)
    {
        leftSetup |= VAS_GLOBALFILTER_LEFT;
        rightSetup |= VAS_GLOBALFILTER_RIGHT;
    }
    
    x->left2left = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->left2right = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    
    x->center2left = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->center2right = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->center2left, x->left2left);
    vas_dynamicFirChannel_shareFilterWith(x->center2right, x->left2right);
    
    x->right2left = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->right2right = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->right2left, x->left2left);
    vas_dynamicFirChannel_shareFilterWith(x->right2right, x->left2right);
    
    x->surroundLeft2left = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->surroundLeft2right = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->surroundLeft2left, x->left2left);
    vas_dynamicFirChannel_shareFilterWith(x->surroundLeft2right, x->left2right);
    
    x->surroundRight2left = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->surroundRight2right = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->surroundRight2left, x->left2left);
    vas_dynamicFirChannel_shareFilterWith(x->surroundRight2right, x->left2right);
    
    return x;
}

void vas_fir_binaural_51_free(vas_fir_binaural_51 *x)
{
    vas_dynamicFirChannel_free(x->left2left);
    vas_dynamicFirChannel_free(x->left2right);
    
    vas_dynamicFirChannel_free(x->center2left);
    vas_dynamicFirChannel_free(x->center2right);
    
    vas_dynamicFirChannel_free(x->right2left);
    vas_dynamicFirChannel_free(x->right2right);
    
    vas_dynamicFirChannel_free(x->surroundLeft2left);
    vas_dynamicFirChannel_free(x->surroundLeft2right);
    
    vas_dynamicFirChannel_free(x->surroundRight2left);
    vas_dynamicFirChannel_free(x->surroundRight2right);
}

