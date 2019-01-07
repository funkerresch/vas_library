/**
 * @file vas_interpolDelay.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - Interpolated Delay <br>
 *
 * Allows for changing delay time during playback without clicks <br>
 * Ported from c++ from standford stk "delayL.c"<br>
 * Added delaytime interpolation over several frames
 * https://ccrma.stanford.edu/software/stk/
 */

#ifndef vas_interpolDelay_h
#define vas_interpolDelay_h

#include <stdio.h>
#include "vas_mem.h"
#include "vas_util.h"typedef struct vas_interpolDelay{     long maxDelay;     long delayTime;        // in Samples
     long targetDelayTime;     float *delayBuffer;     long inPtr;     long outPtr;     float alpha;     float omAlpha;     int doNextOut;     float nextOutput;     float output;} vas_interpolDelay;vas_interpolDelay *vas_interpolDelay_new(long maxDelay);

void vas_interpolDelay_setDelayTime(vas_interpolDelay *x, long delayTime);void vas_interpolDelay_approachNewDelayTime(vas_interpolDelay *x);float vas_interpolDelay_nextOut(vas_interpolDelay *x);void vas_interpolDelay_free(vas_interpolDelay *x);

void vas_interpolDelay_perform1(vas_interpolDelay *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize);float vas_interpolDelay_lastValue(vas_interpolDelay *x);void vas_interpolDelay_resize(vas_interpolDelay *x, long maxDelay);

#endif		 