//
//  vas_combinedTypes.h
//  tr.binaural~
//
//  Created by Admin on 24.06.18.
//

#ifndef vas_combinedTypes_h
#define vas_combinedTypes_h

#include "vas_gps_util.h"
#include "vas_fir_binaural.h"
#include "vas_interpolDelay.h"
#include "vas_iir_biquad.h"
#include "vas_fir_staticFir_m2s.h"

#define VAS_DIRECTIVITY_OMNI 1

#define VAS_SOURCE_MAXNUMBEROFMIRRORS 128



typedef struct vas_indexMap
{
    int index;
    void *vasObject;
} vas_indexMap;

void vas_indexMap_setMapItem(vas_indexMap *x, int index, void *vasObject);

typedef struct vas_directivityFilter
{
    int directivity;
    vas_iir_biquad *damping;
    
} vas_directivityFilter;

vas_directivityFilter *vas_directivityFilter_new();

void vas_directivityFilter_free(vas_directivityFilter *x);

typedef struct vas_source
{
    vas_coordinates position;
    void *parent;
    vas_fir_binaural *binauralEngine;
    vas_interpolDelay *delay;
    vas_directivityFilter *damping;
} vas_source, vas_mirrorSource, vas_mainSource;

vas_source *vas_source_new(vas_source *mother, vas_coordinates position, int maxDelay, int flags, int segmentSize, vas_dynamicFirChannel_config *firConfig);
void vas_anySource_free(vas_source *x);

typedef struct vas_3dSource
{
    vas_mainSource *main;
    vas_mirrorSource *mirrors[VAS_SOURCE_MAXNUMBEROFMIRRORS];
    vas_fir_static_m2s *diffuseReverb;
    vas_coordinates receiverPosition;
    int receiverAzimuth;
    int receiverElevation;
    int mirrorSourceCounter;
    int maxDelay;
    int segmentSize;
    int flags;
    vas_dynamicFirChannel_config *firConfig;
} vas_3dSource;

vas_3dSource *vas_3dSource_new(vas_coordinates position, int maxDelay, int flags, int segmentSize, vas_dynamicFirChannel_config *firConfig);
void vas_3dSource_free(vas_3dSource *x);

void vas_3dSource_addMirrorSource(vas_3dSource *x, vas_coordinates position);

#endif /* vas_combinedTypes_h */
