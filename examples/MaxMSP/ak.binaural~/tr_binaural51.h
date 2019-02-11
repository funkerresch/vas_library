#include "vas_fir_binaural_51.h"

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

typedef struct tr_binaural51
{
#ifdef MAXMSPSDK
    t_pxobject    x_obj;
    long t_size;
    char  **t_text;
    char *ptr2handle;
#endif
#ifdef PUREDATA
    
    t_object x_obj;
    t_outlet *outL;
    t_outlet *outR;
    float inputBuffer[1024];
    
    float f;
#endif
    float azimuth;
    float elevation;
    
    vas_fir_binaural_51 *binauralEngine;
    
    int fadeCounter;
    int filterSize;
    int segmentSize;
    
    float *currentHrir;
    
} tr_binaural51;


#endif /* ak_binaural_h */
