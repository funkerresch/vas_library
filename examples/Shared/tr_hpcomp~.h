
#include "vas_fir_headphoneCompensation.h"

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

typedef struct tr_hpcomp
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
    float inputBuffer[1024];
    
    float f;
#endif
    
    vas_filter_headphoneCompensation *filter;
    
    int filterSize;
    int segmentSize;
    float *currentHrir;
    
} tr_hpcomp;


#endif /* ak_binaural_h */
