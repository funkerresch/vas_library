#include "vas_fir_binaural.h"

void vas_fir_binaural_process(vas_fir_binaural *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, 0);
}

void vas_fir_binaural_processOutputInPlace(vas_fir_binaural *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
}

void vas_fir_binaural_resetInput(vas_fir_binaural *x)
{
    vas_dynamicFirChannel_setAllInputSegments2Zero(x->left);
    vas_dynamicFirChannel_setAllInputSegments2Zero(x->right);
}

void vas_fir_binaural_setAzimuth(vas_fir_binaural *x, int azimuth)
{
    vas_dynamicFirChannel_setAzimuth(x->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(x->right, azimuth);
}

void vas_fir_binaural_setElevation(vas_fir_binaural *x, int elevation)
{
    vas_dynamicFirChannel_setElevation(x->left, elevation);
    vas_dynamicFirChannel_setElevation(x->right, elevation);
}

vas_fir_binaural *vas_fir_binaural_new(int flags, int segmentSize, vas_dynamicFirChannel_config *firConfig)
{
    vas_fir_binaural *x = ( vas_fir_binaural * )vas_mem_alloc(sizeof(vas_fir_binaural));
    vas_filter_metaData_init(&x->description);
    int leftSetup = flags;
    int rightSetup = flags;
    
    if(flags & VAS_GLOBALFILTER)
    {
        leftSetup |= VAS_GLOBALFILTER_LEFT;
        rightSetup |= VAS_GLOBALFILTER_RIGHT;
    }

    x->left = vas_dynamicFirChannel_new(leftSetup, segmentSize, firConfig);
    x->right = vas_dynamicFirChannel_new(rightSetup, segmentSize, firConfig);
    
    return x;
}

void vas_fir_binaural_free(vas_fir_binaural *x)
{
    vas_dynamicFirChannel_free(x->left);
    vas_dynamicFirChannel_free(x->right);
}

void vas_fir_binaural_shareInput(vas_fir_binaural *x, vas_fir_binaural *sharedInput)
{
    vas_dynamicFirChannel_shareInputWith(x->left, sharedInput->left);
    vas_dynamicFirChannel_shareInputWith(x->right, sharedInput->right);
}

void vas_fir_binaural_shareFilter(vas_fir_binaural *x, vas_fir_binaural *sharedFilter)
{
    vas_dynamicFirChannel_shareFilterWith(x->left, sharedFilter->left);
    vas_dynamicFirChannel_shareInputWith(x->right, sharedFilter->right);
}







