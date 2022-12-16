#include "vas_fir_binaural.h"

#ifdef __cplusplus
extern "C" {
#endif
extern vas_fir_list IRs;
int vas_fir_binaural_referenceCounter;
#ifdef VAS_USE_NEUMANNHEADERS
extern vas_fir_binaural *neumannHrtf_48kHz;
extern int neumannHrtfInit_48kHz;
#endif
#ifdef __cplusplus
}
#endif

void vas_fir_binaural_processLeftChannel(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, 0);
}

void vas_fir_binaural_processRightChannel(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, 0);
}

void vas_fir_binaural_process(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, 0);
}

void vas_fir_binaural_processOutputInPlace(vas_fir_binaural *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->left, in, outLeft, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
    vas_dynamicFirChannel_process(x->right, in, outRight, vectorSize, VAS_OUTPUTVECTOR_ADDINPLACE);
}

void vas_fir_binaural_setAzimuthDirection(vas_fir_binaural *x, int aziDirection)
{
    //vas_dynamicFirChannel_setAzimuthDirection(x->left, aziDirection);
   // vas_dynamicFirChannel_setAzimuthDirection(x->right, aziDirection);
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

vas_fir_binaural *vas_fir_binaural_new(int flags)
{
    vas_fir_binaural *x = ( vas_fir_binaural * )vas_mem_alloc(sizeof(vas_fir_binaural));
    vas_filter_metaData_init(&x->metaData);
    int leftSetup = flags;
    int rightSetup = flags;

    x->left = vas_dynamicFirChannel_new(leftSetup);
    x->right = vas_dynamicFirChannel_new(rightSetup);
    
    vas_fir_binaural_referenceCounter++;

#ifdef VAS_USE_NEUMANNHEADERS
    if(!neumannHrtfInit_48kHz)
    {
        neumannHrtfInit_48kHz = 1;
        vas_fir_neumannFilter_48kHz_init();
        vas_util_debug("%s: Initialized Neumann 48kHz HRTF", __FUNCTION__);
    }
#endif
    
    return x;
}

void vas_fir_binaural_free(vas_fir_binaural *x)
{
    if(x->metaData.fullPath)
        vas_mem_free(x->metaData.fullPath );
    
    vas_dynamicFirChannel_free(x->left);
    vas_dynamicFirChannel_free(x->right);
    vas_mem_free(x);
    vas_fir_binaural_referenceCounter--;
    
#ifdef VAS_USE_NEUMANNHEADERS
    if(vas_fir_binaural_referenceCounter == 1)
    {
        if(neumannHrtfInit_48kHz)
        {
            neumannHrtfInit_48kHz = 0;
            vas_fir_binaural_free(neumannHrtf_48kHz);
            vas_util_debug("%s: Freed Neumann 48kHz HRTF", __FUNCTION__);
        }
    }
#endif
}

void vas_fir_binaural_shareInput(vas_fir_binaural *x, vas_fir_binaural *sharedInput)
{
    vas_dynamicFirChannel_shareInputWith(x->left, sharedInput->left);
    vas_dynamicFirChannel_shareInputWith(x->right, sharedInput->right);
}

void vas_fir_binaural_shareFilter(vas_fir_binaural *x, vas_fir_binaural *sharedFilter)
{
    vas_dynamicFirChannel_getSharedFilterValues(x->left, sharedFilter->left);
    vas_dynamicFirChannel_getSharedFilterValues(x->right, sharedFilter->right);
}







