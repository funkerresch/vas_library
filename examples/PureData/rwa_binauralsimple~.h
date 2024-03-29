/*
 Leftover for compatibility with RWA, is going to be replaced with vas_binaural~
 */

#ifndef _binauralsimple_
#define _binauralsimple_

#include "vas_pdmaxobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rwa_binauralsimple
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
    t_inlet *azi;
    t_inlet *ele;
#endif
    
} rwa_binauralsimple;

#ifdef PUREDATA
void rwa_binauralsimple_tilde_setup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ak_binaural_h */
