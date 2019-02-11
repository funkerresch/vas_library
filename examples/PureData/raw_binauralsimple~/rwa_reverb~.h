#include "rwa_firobject.h"

#ifndef rwa_reverb_h
#define rwa_reverb_h

typedef struct rwa_reverb
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
#endif
    
} rwa_reverb;

#endif
