#include "rwa_firobject.h"

#ifndef _binauralsimple_
#define _binauralsimple_

typedef struct rwa_binauralsimple
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
    t_inlet *azi;
    t_inlet *ele;
#endif
    
} rwa_binauralsimple;

#ifdef PUREDATA
void rwa_binauralsimple_tilde_setup(void);
#endif

#endif /* ak_binaural_h */
