#include "vas_fir_binaural_6reflections.h"

void vas_fir_binaural_6reflections_process(vas_fir_binaural_6reflections *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, 0);
    
    vas_iir_biquad_process(x->groundFilterHp, in, x->tmp, vectorSize);
    vas_interpolDelay_perform1(x->groundDelay, x->tmp, x->tmp, vectorSize);
    vas_dynamicFirChannel_process(x->groundReflectionLeft, x->tmp, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->groundReflectionRight, x->tmp, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_iir_biquad_process(x->ceilingFilterHp, in, x->tmp, vectorSize);
    vas_interpolDelay_perform1(x->ceilingDelay, x->tmp, x->tmp, vectorSize);
    vas_dynamicFirChannel_process(x->ceilingReflectionLeft, x->tmp, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->ceilingReflectionRight, x->tmp, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_iir_biquad_process(x->backFilterHp, in, x->tmp, vectorSize);
    vas_interpolDelay_perform1(x->backDelay, x->tmp, x->tmp, vectorSize);
    vas_dynamicFirChannel_process(x->backReflectionLeft, x->tmp, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->backReflectionRight, x->tmp, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_iir_biquad_process(x->frontFilterHp, in, x->tmp, vectorSize);
    vas_interpolDelay_perform1(x->frontDelay, x->tmp, x->tmp, vectorSize);
    vas_dynamicFirChannel_process(x->frontReflectionLeft, x->tmp, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->frontReflectionRight, x->tmp, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_iir_biquad_process(x->leftFilterHp, in, x->tmp, vectorSize);
    vas_interpolDelay_perform1(x->leftDelay, x->tmp, x->tmp, vectorSize);
    vas_dynamicFirChannel_process(x->leftReflectionLeft, x->tmp, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->leftReflectionRight, x->tmp, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    
    vas_iir_biquad_process(x->rightFilterHp, in, x->tmp, vectorSize);
    vas_interpolDelay_perform1(x->rightDelay, x->tmp, x->tmp, vectorSize);
    vas_dynamicFirChannel_process(x->rightReflectionLeft, x->tmp, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->rightReflectionRight, x->tmp, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
}

vas_fir_binaural_6reflections *vas_fir_binaural_6reflections_new(int setup, int segmentSize, vas_dynamicFirChannel_config *firConfig)
{
    vas_fir_binaural_6reflections *x = vas_mem_alloc(sizeof(vas_fir_binaural_6reflections));
    vas_filter_metaData_init(&x->description);
    int leftSetup = setup;
    int rightSetup = setup;
    
    if(setup & VAS_GLOBALFILTER)
    {
        leftSetup |= VAS_GLOBALFILTER_LEFT;
        rightSetup |= VAS_GLOBALFILTER_RIGHT;
    }
    
    x->directLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->directRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    
    x->groundReflectionLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->groundReflectionRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->groundReflectionLeft, x->directLeft);
    vas_dynamicFirChannel_shareFilterWith(x->groundReflectionRight, x->directRight);
    
    x->groundDelay = vas_interpolDelay_new(12000);
    vas_interpolDelay_setDelayTime(x->groundDelay, 3000);
    x->groundFilterHp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 500, 10);
    
    x->ceilingReflectionLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->ceilingReflectionRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->ceilingReflectionLeft, x->directLeft);
    vas_dynamicFirChannel_shareFilterWith(x->ceilingReflectionRight, x->directRight);
    
    x->ceilingDelay = vas_interpolDelay_new(12000);
    vas_interpolDelay_setDelayTime(x->ceilingDelay, 2000);
    x->ceilingFilterHp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 700, 10);
    
    x->backReflectionLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->backReflectionRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->backReflectionLeft, x->directLeft);
    vas_dynamicFirChannel_shareFilterWith(x->backReflectionRight, x->directRight);
    
    x->backDelay = vas_interpolDelay_new(10000);
    vas_interpolDelay_setDelayTime(x->backDelay, 4100);
    x->backFilterHp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 1000, 10);
    
    x->frontReflectionLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->frontReflectionRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->frontReflectionLeft, x->directLeft);
    vas_dynamicFirChannel_shareFilterWith(x->frontReflectionRight, x->directRight);
    
    x->frontDelay = vas_interpolDelay_new(20000);
    vas_interpolDelay_setDelayTime(x->frontDelay, 7000);
    x->frontFilterHp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 1100, 10);
    
    x->leftReflectionLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->leftReflectionRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->leftReflectionLeft, x->directLeft);
    vas_dynamicFirChannel_shareFilterWith(x->leftReflectionRight, x->directRight);
    
    x->leftDelay = vas_interpolDelay_new(20000);
    vas_interpolDelay_setDelayTime(x->leftDelay, 4900);
    x->leftFilterHp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 700, 10);
    
    x->rightReflectionLeft = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->rightReflectionRight = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    vas_dynamicFirChannel_shareFilterWith(x->rightReflectionLeft, x->directLeft);
    vas_dynamicFirChannel_shareFilterWith(x->rightReflectionRight, x->directRight);
    
    x->rightDelay = vas_interpolDelay_new(20000);
    vas_interpolDelay_setDelayTime(x->rightDelay, 5400);
    x->rightFilterHp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 800, 10);
    
    return x;
}

void vas_fir_binaural_6reflections_free(vas_fir_binaural_6reflections *x)
{
    vas_dynamicFirChannel_free(x->left);
    vas_dynamicFirChannel_free(x->right);
}






