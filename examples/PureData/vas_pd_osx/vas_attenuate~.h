#include "vas_attenuation.h"
#ifndef vas_attenuate_h
#define vas_attenuate_h

typedef struct vas_attenuate
{
    t_object x_obj;
    t_outlet *outL;
    vas_attenuation *attenuate;
    float f;
} vas_attenuate;

#ifdef PUREDATA
void vas_attenuate_tilde_setup(void);
#endif

#endif
