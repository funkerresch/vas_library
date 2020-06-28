//
//  vas_iir_biquad_crossfade.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 16.11.19.
//

#ifndef vas_iir_cascadedBiquad_h
#define vas_iir_cascadedBiquad_h

#include <stdio.h>
#include "vas_iir_biquad.h"

typedef struct vas_iir_cascadedBiquad
{
    vas_iir_biquad *ls;
    vas_iir_biquad *pk;
    vas_iir_biquad *hs;
    
} vas_iir_cascadedBiquad;

#endif /* vas_iir_cascadedBiquad_h */
