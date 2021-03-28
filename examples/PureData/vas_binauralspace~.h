
#include "vas_ringBuffer.h"
#include "vas_delayTap_crossfade.h"
#include "vas_pdmaxobject.h"

#ifndef _vas_binauralspace_
#define _vas_binauralspace_

#define VAS_REFLECTIONS_MAXSIZE 6

typedef struct vas_binauralspace_reflection
{
    bool active;
    vas_fir *convolutionEngine;
    vas_delayTap_crossfade *tap;
    
    float delayTime;
    float azimuth;
    float elevation;
    float damping;
    
} vas_binauralspace_reflection;

vas_binauralspace_reflection *vas_binauralspace_reflection_new(vas_fir *sharedFilter, vas_ringBuffer *ringBuffer);

typedef struct vas_binauralspace
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
    t_inlet *azi;
    t_inlet *ele;
#endif
    
    vas_fir *binauralReflectionEngine[VAS_REFLECTIONS_MAXSIZE];
    vas_ringBuffer *ringbuffer;
    vas_delayTap_crossfade *tap[VAS_REFLECTIONS_MAXSIZE];
  
} vas_binauralspace;

#ifdef PUREDATA
void vas_binauralspace_tilde_setup(void);
#endif

#endif
