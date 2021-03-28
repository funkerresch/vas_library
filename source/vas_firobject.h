/**
* @file vas_fir.h
* @author Thomas Resch
* @date 4 Jan 2018
* @brief C - "Base" class for Max/MSP and Pure Data Objects <br>
*
*/

#include "vas_fir_binaural.h"

#define RWA_BINAURALENGINE 0
#define RWA_REVERBENGINE 1

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

#ifndef _rwa_firobject_
#define _rwa_firobject_

#define RWA_FIROBJECT \
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

#define RWA_FIROBJECT_PD \
    t_object x_obj; \
    t_outlet *outL; \
    t_outlet *outR; \
    float f; \
    t_word *leftArray; \
    t_word *rightArray; \
    int leftArrayLength; \
    int rightArrayLength; \
\
    RWA_FIROBJECT

#define RWA_FIROBJECT_MAX \
    t_pxobject x_obj; \
    long t_size; \
    char  **t_text; \
    char *ptr2handle; \
    float outL[VAS_MAXVECTORSIZE]; \
    float outR[VAS_MAXVECTORSIZE]; \
\
    RWA_FIROBJECT


typedef struct rwa_firobject
{
#ifdef PUREDATA
    RWA_FIROBJECT_PD
#endif
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
    
} rwa_firobject;

void vas_firobject_set1(rwa_firobject *x, t_symbol *left, t_symbol *right, float segmentSize, float offset, float end);

void rwa_firobject_read2(rwa_firobject *x, t_symbol *s, float segmentSize, float offset, float end);

void vas_firobject_prepareForInterpolatedIrs(rwa_firobject *x, float segmentSize, float maxLength, float numberOfIrs);

void vas_firobject_loadIr2ArrayIndex(rwa_firobject *x, t_symbol *left, float index);

void vas_pdmaxobject_read(rwa_firobject *x, t_symbol *s, float segmentSize, float offset, float end);

void vas_firobject_setAndInterpolateBetweenIndexes(rwa_firobject *x, t_symbol *start, t_symbol *end, float startIndex, float endIndex, float mode);

void vas_firobject_setAndInterpolateBetweenIndexes1(rwa_firobject *x, t_symbol *s, int argc, t_atom *argv);

void vas_firobject_set_mono_simple(rwa_firobject *x, t_symbol *left, float segmentSize);

#endif
