#include "vas_delayTap_crossfade.h"
#include "vas_ringBuffer.h"

#ifndef vas_del_h
#define vas_del_h

typedef struct vas_del
{
    t_object x_obj;
    t_outlet *outL;
    float f;
    vas_delayTap_crossfade *delayEngine;
    vas_ringBuffer *buffer;
} vas_del;

#ifdef PUREDATA
void vas_del_tilde_setup(void);
#endif

#endif
