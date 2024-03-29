/**
* @file vas_hpcomp~.h
* @author Thomas Resch
* @date 4 Jan 2018
* @brief C - ~Pure Data/MaxMSP Headphone Compensation <br>
*
* A Stereo Headphone Compensation Filter <br>
* Performs a left-to-left and right-to-right channel convolution
* with the loaded filter
*
*/

#ifndef _rwa_hpcomp_
#define _rwa_hpcomp_

#include "vas_pdmaxobject.h"
#include "vas_fir_headphoneCompensation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_hpcomp
{
#ifdef MAXMSPSDK
    VAS_MAX_OBJECT
#endif
#ifdef PUREDATA
    VAS_PD_OBJECT
    void *in2;
    float inputBuffer2[VAS_MAXVECTORSIZE];
#endif
    
} vas_hpcomp;

#ifdef PUREDATA
void vas_hpcomp_tilde_setup(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
