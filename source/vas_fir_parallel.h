/**
 * @file vas_fir_parallel.h
 * @author Thomas Resch <br>
 * Audio Communication Group, TU - Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * <br>
 * @brief Implementation of a non-equal-segmented convolution algorithm and<br>
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
 * Using a maximum segment size could also make sense, but would require a much more complicated scheduling scheme and several delay buffers.
 * This way, after writing 4 min segment size frames we can start calculating the first 1024 fft convolution with the first 4 min segment size frames.
 * After reading the next 4 min size frames at time 1024 the first 1024 frame should be calculated for output, we switch read and write buffers.
 * At the same time (2048) we have collected enough samples to start convolution with the first 2048 filter frame..
 * <br>
 */

#ifdef VAS_USE_MULTITHREADCONVOLUTION

#ifndef vas_fir_parallel_h
#define vas_fir_parallel_h

#include <stdio.h>
#include "vas_fir_binaural.h"
#include "vas_threads.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Struct vas_fir_parallel_data. <br>
 * Holds start and end time in samples and the segment size.<br>
 */

typedef struct vas_fir_parallel_data
{
    int start;
    int end;
    int segmentSize;
} vas_fir_parallel_data;

void vas_fir_parallel_data_set(vas_fir_parallel_data *x, int segmentSize, int start, int end);

/**
 * @brief Struct vas_fir_parallel. <br>
 * C-class struct holding all buffers, <br>
 * convolution engines, job-queues and argument-pointers for <br>
 * the worker threads.<br>
 */

typedef struct vas_fir_parallel
{
    int32_t convolverCount;
    vas_fir_binaural **convolutionEngine;
    vas_fir_parallel_data *partitionData;
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
 } vas_fir_parallel;

/**
 * @brief Creates a new multithreaded non-equal-partitioned convolution engine. <br>
 * @param flags An integer containing byte flags for configuration. Not in use yet. <br>
 * @return Returns a new, vas_fir_parallel object. <br>
 * The vas_fir_partioned class implements a mono-to-stereo multithreaded convolution. <br>
 * It uses (more or less) Gardner's algorithm (https://cse.hkust.edu.hk/mjg_lib/bibs/DPSu/DPSu.Files/Ga95.PDF), <br>
 * only in the beginning it uses a different partition scheme. <br>
 */

vas_fir_parallel *vas_fir_parallel_new(int numberOfThreads);

/**
 * @brief Frees a vas_fir_parallel Object <br>
 * @param x Pointer to object to be freed. <br>
 */

void vas_fir_parallel_free(vas_fir_parallel *x);

/**
 * @brief DSP Processing method which calculates the real-time convolution. <br>
 * @param x Pointer to the vas_fir_parallel object <br>
 * @param in The input vector <br>
 * @param outLeft The left output vector <br>
 * @param outRight The rightoutput vector <br>
 * @param vectorSize The vector size <br>
 * vas_fir_parallel_process calculates a mono-to-stereo real-time convolution <br>
 * with the partition scheme described above. <br>
 * It starts by passing convolution jobs to the worker threads in decreasing order, <br>
 * starting with the largest partition. Then it calculates the partitions necessary immediatly <br>
 * in the caller (main audio) thread. <br>
 * Whenever a frameCounter of a convolution engine is zero, the corresponding deadline for this <br>
 * partition is reached and we swap read and write buffers and we must wait for the engine to finish. <br>
 * Finally we add up all partitions for the output vector.
 * */

void vas_fir_parallel_process(vas_fir_parallel *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize);
/**
 * @brief Reads and calculates Impulse Response from two floating point arrays.<br>
 * @param x Pointer to the vas_fir_parallel object <br>
 * @param left The left IR. <br>
 * @param right The right IR <br>
 * @param length The (max) length of both IRs.<br>

 * The function loads a stereo IR from two floating point arrays.<br>
 * The function cuts the IRs into the different partition sizes and loads the  <br>
 * partitions into the corresponding convolution engines. <br>
 * */

void vas_fir_parallel_setFilter(vas_fir_parallel *x, float *left, float *right, int length);
int vas_fir_parallel_init(vas_fir_parallel *x, int minSegmentSize, int maxSegmentSize, int length);
void vas_fir_parallel_deinit(vas_fir_parallel *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_fir_parallel_h */

#endif
