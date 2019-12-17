#include "vas_firobject.h"

#ifndef rwa_reverb_h
#define rwa_reverb_h

typedef struct vas_reverb
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
#endif
    void *lateConvolutionEngine;
    
} vas_reverb;

#ifdef PUREDATA
void vas_reverb_tilde_setup(void);
#endif

#endif
