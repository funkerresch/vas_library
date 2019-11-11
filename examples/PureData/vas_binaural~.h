#include "vas_firobject.h"

#ifndef _vas_binaural_
#define _vas_binaural_

typedef struct vas_binaural
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
    t_inlet *azi;
    t_inlet *ele;
#endif
  
} vas_binaural;

#ifdef PUREDATA
void vas_binaural_tilde_setup(void);
#endif

#endif
