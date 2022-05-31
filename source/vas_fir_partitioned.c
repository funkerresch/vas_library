//
//  vas_fir_partitioned.c
//  vas_reverb~
//
//  Created by Thomas Resch on 26.04.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//
//  Parallel non-equal-size convolution algorithm
//  Usage example for the heap- and lock-free pthreads-implementation
//  vas_threads. The fixed size jobqueue is a little ugly, needs a little clean-up.
//  static instance counter is missing in order to free the convolution thread pool
//  when the last instance is freed
//
// Partition Scheme
//
// 1st: 8 * minSegmentSize
// 2nd: 2 * (minSegmentSize * 4)
// 3rd: 2 * (lastSegmentSize * 2)
// 4th: 2 * (lastSegmentSize * 2)
// ...
//
// For examample:
//
//
// |256            |1024     |2048             |4096                             |8192           <- Partition Sizes
// |               |         |                 |                                 |
// |-|-|-|-|-|-|-|-|----|----|--------|--------|----------------|----------------|--------------------------------|--------------------------------|
// |0              |2048     |4096             |8192                             |16384          <- Time
// |               |         |                 |                                 |
//
//
// Using a maximum segment size could also make sense, but would require a much more complicated scheduling scheme and many delay buffers.
// This way, after writing 4 min segment size frames we can start calculating the first 1024 fft convolution with the first 4 min segment size frames.
// After reading the next 4 min size frames at time 1024 the first 1024 frame should be calculated for output, we switch read and write buffers.
// At the same time (2048) we have collected enough samples to start convolution with the first 2048 filter frame..

#ifdef VAS_USE_MULTITHREADCONVOLUTION

#include "vas_fir_partitioned.h"
#include "vas_fir_read.h"

vas_threads *vas_convolutionThreadPool; // instance counter of vas_fir_partitioned is missing in order to free the thread pool

void vas_fir_partitioned_data_set(vas_fir_partitioned_data *x, int segmentSize, int start, int end)
{
    x->segmentSize = segmentSize;
    x->start = start;
    x->end = end;
}

vas_fir_partitioned *vas_fir_partitioned_new(int flags)
{
    vas_fir_partitioned *x = ( vas_fir_partitioned * )vas_mem_alloc(sizeof(vas_fir_partitioned));
    x->convolutionEngine = NULL;
    x->filterEnd = 0;
    x->init = 0;
    
    if(vas_convolutionThreadPool == NULL)
        vas_convolutionThreadPool = vas_threads_init(8);
    
    return x;
}

void vas_fir_partitioned_swapReadAndWrite(vas_fir_partitioned *x, int i)
{
    float *tmp = x->readBufferLeft[i];
    int *tmpQueueLeft = x->writeJobQueueLeft;
    int *tmpQueueRight = x->writeJobQueueRight;
    
    x->writeJobQueueLeft = x->readJobQueueLeft;
    x->writeJobQueueRight = x->readJobQueueRight;
    
    x->readJobQueueLeft = tmpQueueLeft;
    x->readJobQueueRight = tmpQueueRight;
    
    x->readBufferLeft[i] = x->writeBufferLeft[i];
    x->writeBufferLeft[i] = tmp;

    tmp = x->readBufferRight[i];
    x->readBufferRight[i] = x->writeBufferRight[i];
    x->writeBufferRight[i] = tmp;

    x->threadArgLeft[i].data = x->writeBufferLeft[i];
    x->threadArgRight[i].data = x->writeBufferRight[i];
}

void vas_fir_partitioned_process(vas_fir_partitioned *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *outLeft, VAS_OUTPUTBUFFER *outRight, int vectorSize)
{
    for(int i = x->convolverCount-1; i > 0; i--)
    {
        vas_dynamicFirChannel_process_threaded2(&x->threadArgLeft[i], i, in, vectorSize);
        vas_dynamicFirChannel_process_threaded2(&x->threadArgRight[i], i, in, vectorSize);
        
        if(!x->convolutionEngine[i]->left->frameCounter)    // if framecounter = 0, we just collected enough frames for calculating the convolution
            vas_fir_partitioned_swapReadAndWrite(x, i);     // therefore we swap read and write buffers
    }
    
    vas_dynamicFirChannel_process(x->convolutionEngine[0]->left, in, outLeft, vectorSize, 0);
    vas_dynamicFirChannel_process(x->convolutionEngine[0]->right, in, outRight, vectorSize, 0);
    
    for(int i = x->convolverCount-1; i > 0; i--)            // after exchanging read and write buffers we are making sure, that every convolution engine has
    {                                                       // finished calculating before we add the results together
        if(!x->convolutionEngine[i]->left->frameCounter)
            VAS_THREADS_WAIT_FOR_EMPTY_QUEUE(&x->readJobQueueLeft[i]);  // because we give every doubled fft size twice as much time to finish,
        if(!x->convolutionEngine[i]->right->frameCounter)               // this (usually) also works fine without waiting because fft has O(n log n)
            VAS_THREADS_WAIT_FOR_EMPTY_QUEUE(&x->readJobQueueRight[i]); // But depends highly on what else is going on your computer
    }
    
    for(int i = x->convolverCount-1; i > 0; i--)
    {
        vas_util_fadd(x->readBufferLeft[i] + x->convolutionEngine[i]->left->frameCounter * vectorSize, outLeft, outLeft, vectorSize);
        vas_util_fadd(x->readBufferRight[i] + x->convolutionEngine[i]->right->frameCounter * vectorSize, outRight, outRight, vectorSize);
    }
}

int vas_fir_partitioned_init(vas_fir_partitioned *x, int minSegmentSize, int maxSegmentSize, int length)
{
    if(x->init)
        vas_fir_partitioned_deinit(x);
    
    int currentStart = 0;
    int currentEnd = minSegmentSize * 8;
    int currentSize = minSegmentSize * 4;
    int count = 1;
    
    while(currentEnd < length)
    {
        count++;
        currentStart = currentEnd;
        currentEnd += currentSize*2;
        
        if(maxSegmentSize)
        {
            if(currentSize < maxSegmentSize)
                currentSize *=2;
        }
        else
            currentSize *=2;
    }
    
    x->convolverCount = count;
    x->filterEnd = currentEnd;
    
    x->convolutionEngine = (vas_fir_binaural **) vas_mem_alloc(sizeof(vas_fir_binaural *) * x->convolverCount);
    x->writeBufferLeft = (float **) vas_mem_alloc(sizeof(float *)* x->convolverCount);
    x->writeBufferRight = (float **) vas_mem_alloc(sizeof(float *)* x->convolverCount);
    x->readBufferLeft = (float **) vas_mem_alloc(sizeof(float *)* x->convolverCount);
    x->readBufferRight = (float **) vas_mem_alloc(sizeof(float *)* x->convolverCount);
    x->threadArgLeft = (vas_threadedConvolutionArg *) vas_mem_alloc(sizeof(vas_threadedConvolutionArg)* x->convolverCount);
    x->threadArgRight = (vas_threadedConvolutionArg *) vas_mem_alloc(sizeof(vas_threadedConvolutionArg)* x->convolverCount);
    x->partitionData = (vas_fir_partitioned_data *) vas_mem_alloc(sizeof(vas_fir_partitioned_data) * x->convolverCount);
    x->writeJobQueueLeft = (int *) vas_mem_alloc(x->convolverCount * sizeof(int));
    x->writeJobQueueRight = (int *) vas_mem_alloc(x->convolverCount * sizeof(int));
    x->readJobQueueLeft = (int *) vas_mem_alloc(x->convolverCount * sizeof(int));
    x->readJobQueueRight = (int *) vas_mem_alloc(x->convolverCount * sizeof(int));
    
    currentEnd = minSegmentSize * 8;
    vas_fir_partitioned_data_set(&x->partitionData[0], minSegmentSize, 0, currentEnd);
    
    x->writeBufferLeft[0] = NULL; // those are not needed for the min segment size
    x->writeBufferRight[0] = NULL;
    x->readBufferLeft[0] = NULL;
    x->readBufferRight[0] = NULL;
    
    currentSize = minSegmentSize * 4;
    
    for(int i = 1; i < x->convolverCount; i++)
    {
        currentStart = currentEnd;
        currentEnd += currentSize*2;
         
        vas_fir_partitioned_data_set(&x->partitionData[i], currentSize, currentStart, currentEnd);
        
        x->writeBufferLeft[i] = vas_mem_alloc(currentSize * sizeof(float));
        x->writeBufferRight[i] = vas_mem_alloc(currentSize * sizeof(float));
        x->readBufferLeft[i] = vas_mem_alloc(currentSize * sizeof(float));
        x->readBufferRight[i] = vas_mem_alloc(currentSize * sizeof(float));
        
        if(maxSegmentSize)
        {
            if(currentSize < maxSegmentSize)
                currentSize *=2;
        }
        else
            currentSize *=2;
    }
    
    for(int i = 0; i < x->convolverCount; i++)
    {
        x->convolutionEngine[i] = vas_fir_binaural_new(0);
        x->threadArgLeft[i].x = x->convolutionEngine[i]->left;
        x->threadArgLeft[i].data = x->writeBufferLeft[i];
        x->threadArgRight[i].x = x->convolutionEngine[i]->right;
        x->threadArgRight[i].data = x->writeBufferRight[i];
        x->threadArgLeft[i].job = malloc(sizeof(vas_job));
        x->threadArgRight[i].job = malloc(sizeof(vas_job));
        x->threadArgLeft[i].jobQueue = &x->writeJobQueueLeft[i];
        x->threadArgRight[i].jobQueue = &x->writeJobQueueRight[i];
    }
    
    x->init = 1;
    return x->filterEnd;
}

void vas_fir_partitioned_deinit(vas_fir_partitioned *x)
{
    if(x->init)
    {
        for(int i = 0; i < x->convolverCount; i++)
        {
            vas_fir_binaural_free(x->convolutionEngine[i]);
            vas_mem_free(x->writeBufferLeft[i]);
            vas_mem_free(x->writeBufferRight[i]);
            vas_mem_free(x->readBufferLeft[i]);
            vas_mem_free(x->readBufferRight[i]);
            free(x->threadArgLeft[i].job);
            free(x->threadArgRight[i].job);
        }
        
        vas_mem_free(x->writeJobQueueLeft);
        vas_mem_free(x->writeJobQueueRight);
        vas_mem_free(x->readJobQueueLeft);
        vas_mem_free(x->readJobQueueRight);
        vas_mem_free(x->threadArgLeft);
        vas_mem_free(x->threadArgRight);
        vas_mem_free(x->partitionData);
        vas_mem_free(x->convolutionEngine);
        vas_mem_free(x->writeBufferLeft);
        vas_mem_free(x->writeBufferRight);
        vas_mem_free(x->readBufferLeft);
        vas_mem_free(x->readBufferRight);
    }
    x->init = 0;
}

void vas_fir_partitioned_setFilter(vas_fir_partitioned *x, float *left, float *right, int length)
{
    char name[20];
    int currentSize = x->partitionData[0].segmentSize;
    int start = x->partitionData[0].start;
    int end = x->partitionData[0].end;
    
    vas_fir_read_singleImpulseFromFloatArray((vas_fir*) x->convolutionEngine[0], "MinPartitionSize", left, right, length, currentSize, 0, x->partitionData[0].end);
    
    for(int i=1; i<x->convolverCount; i++)
    {
        currentSize = x->partitionData[i].segmentSize;
        start = x->partitionData[i].start;
        end = x->partitionData[i].end;
        sprintf(name, "%dpartition", i);
        vas_fir_read_singleImpulseFromFloatArray((vas_fir*) x->convolutionEngine[i], name, left, right, length, currentSize, x->partitionData[i].start, x->partitionData[i].end);
    }
}

void vas_fir_partitioned_free(vas_fir_partitioned *x)
{
    vas_fir_partitioned_deinit(x);
    vas_mem_free(x);
}

#endif
