#include "vas_firobject.h"

#ifndef rwa_reverb_h
#define rwa_reverb_h

typedef struct vas_dynconv
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
#endif
    
} vas_dynconv;

#ifdef PUREDATA
void vas_dynconv_tilde_setup(void);
#endif

#endif
