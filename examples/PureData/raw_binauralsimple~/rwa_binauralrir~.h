#include "rwa_firobject.h"

#ifndef _rwa_binauralrir_
#define _rwa_binauralrir_

typedef struct rwa_binauralrir
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
    t_inlet *azi;
    t_inlet *ele;
#endif
  
} rwa_binauralrir;

#endif
