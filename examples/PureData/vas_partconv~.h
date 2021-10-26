#ifndef rwa_partconv_h
#define rwa_partconv_h

#include "vas_pdmaxobject.h"
#include "vas_fir_partitioned.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_partconv
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
#endif
    vas_fir_partitioned *partConvEngine;
    
} vas_partconv;

#ifdef PUREDATA
void vas_partconv_tilde_setup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
