#include "rwa_firobject.h"

#ifndef _rwa_hpcomp_
#define _rwa_hpcomp_

typedef struct rwa_hpcomp
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
    void *in2;
    float inputBuffer2[2048];
#endif
    
} rwa_hpcomp;

#ifdef PUREDATA
void rwa_hpcomp_tilde_setup(void);
#endif

#endif
