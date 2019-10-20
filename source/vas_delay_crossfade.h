//
//  vas_delay_crossfade.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#ifndef vas_delay_crossfade_h
#define vas_delay_crossfade_h

#include <stdio.h>
#include "vas_delay.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * @struct vas_delay_crossfade
 * @brief A structure for a delay object <br>
 * @var vas_delay::buffer The buffer we save the incoming signal in <br>
 * @var vas_delay::delay_in_samples The parameter value for adjusting the <br>
 * delay of the incoming signal
 * @var vas_delay::buffer_size The size of the delay buffer <br>
 * @var vas_delay::circular_pointer Circular pointer to the delay buffer <br>
 * @var vas_delay::delay_sample The current sample from the delay buffer <br>
 */
typedef struct vas_delay_crossfade
{
    float *fadeOut;
    float *fadeIn;
    float *outputCurrent;
    float *outputTarget;
    int fadeLength;
    int delayTimeTmp;
    vas_delay *current;
    vas_delay *target;
    int fadeCounter;
    int numberOfFramesForCrossfade;
} vas_delay_crossfade;
    
vas_delay_crossfade *vas_delay_crossfade_new(long maxDelayTime);
    
void vas_delay_crossfade_setDelayTime(vas_delay_crossfade *x, float delayTime);
    
void vas_delay_crossfade_process(vas_delay_crossfade *x, float *in, float *out, int vectorSize);
    
void vas_delay_crossfade_free(vas_delay_crossfade *x);
    
#ifdef __cplusplus
    }
#endif

#endif /* vas_delay_crossfade_h */
