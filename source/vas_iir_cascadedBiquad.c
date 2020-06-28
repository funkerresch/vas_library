//
//  vas_iir_biquad_crossfade.c
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 16.11.19.
//

#include "vas_iir_cascadedBiquad.h"

vas_iir_cascadedBiquad *vas_iir_cascadedBiquad_new()
{
    vas_iir_cascadedBiquad *x = (vas_iir_cascadedBiquad *)vas_mem_alloc(sizeof(vas_iir_cascadedBiquad));
    x->ls = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWSHELF, 100, 0.707);
    x->pk = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWSHELF, 2000, 0.707);
    x->hs = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWSHELF, 10000, 0.707);
    return x;
}
