//
//  vas_pdmaxobject.h
//  vas_binaural~
//
//  Created by Thomas Resch on 18.02.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#ifndef vas_pdmaxobject_h
#define vas_pdmaxobject_h

#include "vas_fir_binaural.h"
#include "vas_fir_read.h"

#ifdef VAS_USE_LIBMYSOFA
#include "mysofa.h"
#endif
#ifdef MAXMSPSDK
#include "z_dsp.h"
#include "vas_maxObjectUtil.h"
#endif
#ifdef PUREDATA
#include "m_pd.h"
#endif

#define VAS_PDMAX_OBJECT \
    void *convolutionEngine; \
    float inputBuffer[VAS_MAXVECTORSIZE]; \
    float azimuth; \
    float elevation; \
\
    char fullpath[512]; \
    char canvasDirectory[512]; \
\
    int fadeCounter; \
    int filterSize; \
    int segmentSize; \
    int aziDirection; \
    int eleDirection; \
\
    float *currentHrir;

#define VAS_PD_OBJECT \
    t_object x_obj; \
    t_outlet *outL; \
    t_outlet *outR; \
    float f; \
    t_word *leftArray; \
    t_word *rightArray; \
    int leftArrayLength; \
    int rightArrayLength; \
\
    VAS_PDMAX_OBJECT

#define VAS_MAX_OBJECT \
    t_pxobject x_obj; \
    long t_size; \
    char  **t_text; \
    char *ptr2handle; \
    float outL[VAS_MAXVECTORSIZE]; \
    float outR[VAS_MAXVECTORSIZE]; \
\
    VAS_PDMAX_OBJECT


typedef struct vas_pdmaxobject
{
#ifdef PUREDATA
    VAS_PD_OBJECT
#endif
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
    
} vas_pdmaxobject;

void vas_pdmaxobject_read(vas_pdmaxobject *x, t_symbol *s, float segmentSize, float offset, float end);

void vas_pdmaxobject_set1(vas_pdmaxobject *x, t_symbol *left, t_symbol *right, float segmentSize, float offset, float end);

#ifdef PUREDATA
void vas_pdmaxobject_getFloatArrayAndLength(t_symbol *arrayname, t_word **array, int *length);
#endif

#endif /* vas_pdmaxobject_h */
