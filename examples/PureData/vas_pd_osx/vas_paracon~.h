#ifndef rwa_partconv_h
#define rwa_partconv_h

#include "vas_pdmaxobject.h"
#include "vas_fir_parallel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_paracon
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
#endif
    vas_fir_parallel *partConvEngine;
    
} vas_paracon;

#ifdef PUREDATA
void vas_paracon_tilde_setup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
