#include "vas_delay_crossfade.h"

#ifndef rwa_reverb_h
#define rwa_reverb_h

typedef struct vas_del
{
    t_object x_obj;
    t_outlet *outL;
    float f;
    vas_delay_crossfade *delayEngine;
    float lastDelayTime;
    float targetDelayTime;
} vas_del;

#ifdef PUREDATA
void vas_del_tilde_setup(void);
#endif

#endif
