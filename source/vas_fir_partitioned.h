/**
 * @file vas_fir_partitioned.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * <br>
 * @brief Implementation of a non-equal-segmented convolution algorithm, and<br>
 * usage example for the heap- and lock-free pthreads-implementation vas_threads. <br>
 * pd example usage is in vas_partconv~.
 * Partition Scheme is close to Gardners proposition, but a little different in the beginning:
 *
 * 1st: 8 * minSegmentSize
 * 2nd: 2 * (minSegmentSize * 4)
 * 3rd: 2 * (lastSegmentSize * 2)
 * 4th: 2 * (lastSegmentSize * 2)
 * ...
 *
 * For example:
 *
 *
 * |256            |1024     |2048             |4096                             |8192           <- Partition Sizes
 * |               |         |                 |                                 |
 * |-|-|-|-|-|-|-|-|----|----|--------|--------|----------------|----------------|--------------------------------|--------------------------------|
 * |0              |2048     |4096             |8192                             |16384          <- Time
 * |               |         |                 |                                 |
 *
 * Using a maximum segment size could also make sense, but would require a much more complicated scheduling scheme and many delay buffers.
 * This way, after writing 4 min segment size frames we can start calculating the first 1024 fft convolution with the first 4 min segment size frames.
 * After reading the next 4 min size frames at time 1024 the first 1024 frame should be calculated for output, we switch read and write buffers.
 * At the same time (2048) we have collected enough samples to start convolution with the first 2048 filter frame..
 * <br>
 * <br>
 */

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
    int *writeJobQueueLeft;
    int *writeJobQueueRight;
    int *readJobQueueLeft;
    int *readJobQueueRight;
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
