//
//  vas_delayTap_crossfade.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 10.06.19.
//

#ifndef vas_delayTap_crossfade_h
#define vas_delayTap_crossfade_h

#include <stdio.h>
#include "vas_delayTap.h"


#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * @struct vas_delayTap_crossfade
 * @brief A structure for a delayTap object <br>
 * @var vas_delayTap::buffer The buffer we save the incoming signal in <br>
 * @var vas_delayTap::delayTap_in_samples The parameter value for adjusting the <br>
 * delayTap of the incoming signal
 * @var vas_delayTap::buffer_size The size of the delayTap buffer <br>
 * @var vas_delayTap::circular_pointer Circular pointer to the delayTap buffer <br>
 * @var vas_delayTap::delayTap_sample The current sample from the delayTap buffer <br>
 */
typedef struct vas_delayTap_crossfade
{
    float *fadeOut;
    float *fadeIn;
    float *outputCurrent;
    float *outputTarget;
    int fadeLength;
    int delayTimeTmp;
    vas_delayTap *current;
    vas_delayTap *target;
    int fadeCounter;
    int startCrossfade;
    int numberOfFramesForCrossfade;
    int maxdelayTapTime;
} vas_delayTap_crossfade;
    
vas_delayTap_crossfade *vas_delayTap_crossfade_new(vas_ringBuffer *ringBuffer);

void vas_delayTap_crossfade_setRingBuffer(vas_delayTap_crossfade *x, vas_ringBuffer *ringBuffer);
    
void vas_delayTap_crossfade_setDelayTime(vas_delayTap_crossfade *x, float delayTime);
    
void vas_delayTap_crossfade_process(vas_delayTap_crossfade *x, float *out, int vectorSize);

void vas_delayTap_crossfade_clear(vas_delayTap_crossfade *x);
    
void vas_delayTap_crossfade_free(vas_delayTap_crossfade *x);
    
#ifdef __cplusplus
    }
#endif

#endif /* vas_delayTap_crossfade_h */
