//
//  vas_fir_partitioned.h
//  vas_reverb~
//
//  Created by Harvey Keitel on 26.04.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#ifdef VAS_USE_MULTITHREADCONVOLUTION

#ifndef vas_fir_partitioned_h
#define vas_fir_partitioned_h

#include <stdio.h>
#include "vas_fir_binaural.h"
#include "vas_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_fir_partitioned_data
{
    int start;
    int end;
    int segmentSize;
} vas_fir_partitioned_data;

void vas_fir_partitioned_data_set(vas_fir_partitioned_data *x, int segmentSize, int start, int end);

typedef struct vas_fir_partitioned
{
    int32_t convolverCount;
    vas_fir_binaural **convolutionEngine;
    vas_fir_partitioned_data *partitionData;
    float **writeBufferLeft;
    float **writeBufferRight;
    float **readBufferLeft;
    float **readBufferRight;
    vas_threadedConvolutionArg *threadArgLeft;
    vas_threadedConvolutionArg *threadArgRight;
    int filterEnd;
    int init;
 } vas_fir_partitioned;

vas_fir_partitioned *vas_fir_partitioned_new(int flags);
void vas_fir_partitioned_free(vas_fir_partitioned *x);
void vas_fir_partitioned_process(vas_fir_partitioned *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize);
void vas_fir_partitioned_shareFilter(vas_fir_partitioned *x, vas_fir_partitioned *sharedFilter);
void vas_fir_partitioned_setFilter(vas_fir_partitioned *x, float *left, float *right, int length);
int vas_fir_partitioned_init(vas_fir_partitioned *x, int minSegmentSize, int maxSegmentSize, int length);
void vas_fir_partitioned_deinit(vas_fir_partitioned *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_fir_partitioned_h */

#endif
