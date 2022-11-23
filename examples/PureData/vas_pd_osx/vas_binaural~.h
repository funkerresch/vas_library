
#ifndef _vas_binaural_
#define _vas_binaural_

#include "vas_pdmaxobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_binaural
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
    t_inlet *azi;
    t_inlet *ele;
#endif
  
} vas_binaural;

#ifdef PUREDATA
void vas_binaural_tilde_setup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
