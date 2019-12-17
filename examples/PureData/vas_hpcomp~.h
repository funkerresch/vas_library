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

#include "vas_firobject.h"

#ifndef _rwa_hpcomp_
#define _rwa_hpcomp_

typedef struct vas_hpcomp
{
#ifdef MAXMSPSDK
    RWA_FIROBJECT_MAX
#endif
#ifdef PUREDATA
    RWA_FIROBJECT_PD
    void *in2;
    float inputBuffer2[VAS_MAXVECTORSIZE];
#endif
    
} vas_hpcomp;

#ifdef PUREDATA
void vas_hpcomp_tilde_setup(void);
#endif

#endif
