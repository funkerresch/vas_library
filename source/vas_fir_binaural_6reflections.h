/**
 * @file vas_fir_binaraural.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Dynamic Mono Source to Binaural Filter <br>
 *
 * A Dynamic Mono Source to Binaural Filter Class with 6 binaural calculated early reflections. <br>
 * Performs an mono-input-to-left and mono-input-to-right convolution for <br>
 * binaural synthesis. Convolution is performed (for now) with equal-sized partitions <br>
 * size.
 *
 */

#ifndef vas_filter_binaural_6reflections_h
#define vas_filter_binaural_6reflections_h

#include "math.h"
#include "vas_fir.h"
#include "vas_interpolDelay.h"
#include "vas_iir_2highpass2lowpass.h"
#include "vas_iir_biquad.h"

#define directLeft left
#define directRight right

typedef struct vas_fir_binaural_6reflections
{
    vas_fir_metaData description;
    vas_dynamicFirChannel *directLeft;
    vas_dynamicFirChannel *directRight;
    
    vas_dynamicFirChannel *groundReflectionLeft;
    vas_dynamicFirChannel *groundReflectionRight;
    vas_interpolDelay *groundDelay;
    vas_iir_biquad *groundFilterHp;
    vas_iir_biquad *groundFilterLp;
    
    vas_dynamicFirChannel *ceilingReflectionLeft;
    vas_dynamicFirChannel *ceilingReflectionRight;
    vas_interpolDelay *ceilingDelay;
    vas_iir_biquad *ceilingFilterHp;
    vas_iir_biquad *ceilingFilterLp;

    vas_dynamicFirChannel *frontReflectionLeft;
    vas_dynamicFirChannel *frontReflectionRight;
    vas_interpolDelay *frontDelay;
    vas_iir_biquad *frontFilterHp;
    vas_iir_biquad *frontFilterLp;
    
    vas_dynamicFirChannel *backReflectionLeft;
    vas_dynamicFirChannel *backReflectionRight;
    vas_interpolDelay *backDelay;
    vas_iir_biquad *backFilterHp;
    vas_iir_biquad *backFilterLp;
    
    vas_dynamicFirChannel *leftReflectionLeft;
    vas_dynamicFirChannel *leftReflectionRight;
    vas_interpolDelay *leftDelay;
    vas_iir_biquad *leftFilterHp;
    vas_iir_biquad *leftFilterLp;
    
    vas_dynamicFirChannel *rightReflectionLeft;
    vas_dynamicFirChannel *rightReflectionRight;
    vas_interpolDelay *rightDelay;
    vas_iir_biquad *rightFilterHp;
    vas_iir_biquad *rightFilterLp;
    
    AK_OUTPUTVECTOR tmp[1024];
    
} vas_fir_binaural_6reflections;

vas_fir_binaural_6reflections *vas_fir_binaural_6reflections_new(int setup, int segmentSize, vas_dynamicFirChannel_config *firConfig);
void vas_fir_binaural_6reflections_free(vas_fir_binaural_6reflections *x);
void vas_fir_binaural_6reflections_process(vas_fir_binaural_6reflections *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *outLeft, AK_OUTPUTVECTOR *outRight, int vectorSize);

#endif

