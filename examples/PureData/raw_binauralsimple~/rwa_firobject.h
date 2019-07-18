#include "vas_fir_binaural.h"
#include "vas_fir_headphoneCompensation.h"

#define VAS_USE_LIBMYSOFA

#ifdef VAS_USE_LIBMYSOFA
#include "mysofa.h"
#endif
#ifdef MAXMSPSDK
#include "z_dsp.h"
#endif
#ifdef PUREDATA
#include "m_pd.h"
#endif

#ifndef _rwa_firobject_
#define _rwa_firobject_

#define RWA_FIROBJECT \
    void *convolutionEngine; \
    float azimuth; \
    float elevation; \
\
    char fullpath[512]; \
    char canvasDirectory[512]; \
\
    int fadeCounter; \
    int filterSize; \
    int segmentSize; \
    int useGlobalFilter; \
\
    float *currentHrir;

#define RWA_FIROBJECT_PD \
    t_object x_obj; \
    t_outlet *outL; \
    t_outlet *outR; \
    float inputBuffer[2048]; \
    float f; \
\
    RWA_FIROBJECT

#define RWA_FIROBJECT_MAX \
    t_pxobject x_obj; \
    long t_size; \
    char  **t_text; \
    char *ptr2handle; \
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

void rwa_firobject_read(rwa_firobject *x, t_symbol *s);

#endif
