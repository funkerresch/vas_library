#ifndef rwa_reverb_h
#define rwa_reverb_h

#include "vas_pdmaxobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_reverb
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
#endif
} vas_reverb;

#ifdef PUREDATA
void vas_reverb_tilde_setup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
