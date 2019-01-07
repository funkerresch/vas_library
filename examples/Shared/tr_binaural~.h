
#include "vas_fir_binaural.h"

#ifdef USE_LIBMYSOFA
#include "mysofa.h"
#endif
#ifdef MAXMSPSDK
#include "z_dsp.h"
#endif
#ifdef PUREDATA
#include "m_pd.h"
#endif


#ifndef ak_binaural_h
#define ak_binaural_h

typedef struct tr_binaural
{
#ifdef MAXMSPSDK
    t_pxobject	x_obj;
    long t_size;
    char  **t_text;
    char *ptr2handle;
#endif
#ifdef PUREDATA

    t_object x_obj;
    t_outlet *outL;
    t_outlet *outR;
    t_inlet *azi;
    t_inlet *ele;
    
    
    float f;
#endif
    float azimuth;
    float elevation;
    double inputBuffer[2048];
    
    vas_fir_binaural *binauralEngine;
    
    int fadeCounter;
    int filterSize;
    int segmentSize;
    int useGlobalFilter;
    
    float *currentHrir;
    
} tr_binaural;


#endif /* ak_binaural_h */
