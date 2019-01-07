//
//  akconvolve~.h
//  ak.binaural~
//
//  Created by Admin on 08.02.18.
//

#ifndef akconvolve__h
#define akconvolve__h

#include "vas_fir_staticFir_m2s.h"
#include "vas_fir_staticFir_s2s.h"
#include "vas_maxObjectUtil.h"

#ifdef MAXMSPSDK
#include "z_dsp.h"
#endif

typedef struct tr_convolve
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
    
    vas_fir_static_s2s *convolutionEngine;
    vas_dynamicFirChannel *currentChannel2read;
    
    int filterSize;
    int segmentSize;
    int currentHrtfIndex2read;
    int getChFromTxt;
    float *currentIr;
    
} tr_convolve;




#endif /* akconvolve__h */
