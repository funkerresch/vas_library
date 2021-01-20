//
//  vas_combinedTypes.c
//  tr.binaural~
//
//  Created by Admin on 24.06.18.
//

#include "vas_3dSource.h"

void vas_indexMap_setMapItem(vas_indexMap *x, int index, void *vasObject)
{
    x->index = index;
    x->vasObject = vasObject;
}

vas_directivityFilter *vas_directivityFilter_new()
{
    vas_directivityFilter *x = vas_mem_alloc(sizeof(vas_directivityFilter));
    x->damping = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWPASS, 10000, 5);
    x->directivity = VAS_DIRECTIVITY_OMNI;
    return x;
}

void vas_directivityFilter_free(vas_directivityFilter *x)
{
    vas_mem_free(x->damping);
    vas_mem_free(x);
}

vas_source *vas_source_new(vas_source *mother, vas_coordinates position, int maxDelay, int flags, int segmentSize, vas_dynamicFirChannel_config *firConfig)
{
    vas_source *x = vas_mem_alloc(sizeof(vas_source));
    x->parent = mother;
    x->position = position;
    x->binauralEngine = vas_fir_binaural_new(flags, segmentSize, firConfig);
    x->delay = vas_interpolDelay_new(maxDelay);
    x->damping = vas_directivityFilter_new();
    return x;
}

void vas_anySource_free(vas_source *x)
{
    vas_mem_free(x->binauralEngine);
    vas_mem_free(x->delay);
    vas_mem_free(x->damping);
    vas_mem_free(x);
}

vas_3dSource *vas_3dSource_new(vas_coordinates position, int maxDelay, int flags, int segmentSize, vas_dynamicFirChannel_config *firConfig)
{
    vas_3dSource *x = vas_mem_alloc(sizeof(vas_3dSource));
    x->main = vas_source_new(NULL, position, maxDelay, flags, segmentSize, firConfig);
    x->diffuseReverb = vas_filter_staticFir_m2s_new(VAS_VDSP | VAS_LOCALFILTER, segmentSize);
    x->mirrorSourceCounter = 0;
    x->maxDelay = maxDelay;
    x->flags = flags;
    x->segmentSize = segmentSize;
    x->firConfig = firConfig;
    x->receiverPosition = position;
    x->receiverAzimuth = 0;
    x->receiverElevation = 0;
    
    return x;
}

void vas_3dSource_addMirrorSource(vas_3dSource *x, vas_coordinates position)
{
    if(x->mirrorSourceCounter < VAS_SOURCE_MAXNUMBEROFMIRRORS)
        x->mirrors[x->mirrorSourceCounter] = vas_source_new(x->main, position, x->maxDelay, x->flags, x->segmentSize, x->firConfig);
}

void vas_3dSource_setReceiverPosition(vas_3dSource *x, vas_coordinates receiverPosition)
{
    x->receiverPosition = receiverPosition;
    double azimuth = vas_gps_util_calculateBearingWithHeadOrientation(receiverPosition, x->main->position, x->receiverAzimuth);
    vas_fir_binaural_setAzimuth(x->main->binauralEngine, azimuth);
}

void vas_3dSource_setReceiverAzimuth(vas_3dSource *x, int receiverAzimuth)
{
    x->receiverAzimuth = receiverAzimuth;
    double azimuth = vas_gps_util_calculateBearingWithHeadOrientation(x->receiverPosition, x->main->position, receiverAzimuth);
    vas_fir_binaural_setAzimuth(x->main->binauralEngine, azimuth);
    
    //mirrors are missing // damping is missing // delays are missing
}

void vas_3dSource_setReceiverElevation(vas_3dSource *x, int receiverElevation)
{
    x->receiverElevation= receiverElevation;
    vas_fir_binaural_setElevation(x->main->binauralEngine, receiverElevation);
}

void vas_3dSource_calculatedAnglesAndDelays(vas_3dSource *x, vas_coordinates receiverPosition, int receiverAzi, int receiverEle)
{
    double azimuth = vas_gps_util_calculateBearingWithHeadOrientation(receiverPosition, x->main->position, receiverAzi);
    vas_fir_binaural_setAzimuth(x->main->binauralEngine, azimuth);
    vas_fir_binaural_setElevation(x->main->binauralEngine, receiverEle);
    
    
    //mirrors are missing // damping is missing // delays are missing
}

void vas_3dSource_process(vas_3dSource *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize)
{
    vas_dynamicFirChannel_process(x->main->binauralEngine->left, in, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->main->binauralEngine->right, in, outRight, vectorSize, 0);
    
    //mirrors are missing // damping is missing // delays are missing // direction independent room is missing
}

void vas_source_free(vas_3dSource *x)
{
    
}
















