#include "vas_firobject.h"
#include "vas_ringBuffer.h"
#include "vas_delayTap_crossfade.h"

#ifndef _vas_binauralspace_
#define _vas_binauralspace_

typedef struct vas_binauralspace
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
    t_inlet *azi;
    t_inlet *ele;
    vas_ringBuffer *ringbuffer;
    vas_delayTap_crossfade *tap;
#endif
  
} vas_binauralspace;

#ifdef PUREDATA
void vas_binauralspace_tilde_setup(void);
#endif

#endif
