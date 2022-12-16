//
//  vas_pRefl.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 10.02.21.
//

#include "vas_util.h"
#ifdef VAS_COMPILE_WITH_MULTITHREADING

#ifndef vas_pRefl_h
#define vas_pRefl_h


//disable for best performance, enable for debugging/benchmarking
//#define VAS_ENABLE_MULTITHREAD_PROFILING

#ifndef vas_fir_binauralReflection1_h
#include "vas_fir_binauralReflection1.h"
#endif

#include <pthread.h>
#include <stdio.h>


#define VAS_PREF_MAX_SIZE 10000 ///< maximum audio buffer size (needed for temporary thread buffers)
#define VAS_PREF_MAX_TASKS 10000 ///< maximum number of tasks per thread.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_pConv_tempBuffer {
    VAS_OUTPUTBUFFER **outL;
    VAS_OUTPUTBUFFER **outR;
    int nBuffers;
    int nSamples;
} vas_pConv_tempBuffer;

typedef struct vas_threadLog {
    int numJobs;
    int numThreads;
    int numJobsPerThread;
    double wallTime;
    double cpuTime;
    double idleTime;
} vas_threadLog;

typedef struct vas_pConv_firJob {
    int start;
    int end;
    int tid;
    VAS_INPUTBUFFER *inBuf;
    VAS_OUTPUTBUFFER *outL;
    VAS_OUTPUTBUFFER *outR;
    int length;
} vas_pConv_firJob;

/**
* Puts a reflection to the queue to process it later.
* @param[x] the reflection to process later
* @param[tid] thread id. You can get the number of threads by vas_pRefl_num_threads.
*/
void vas_pRefl_global_enqueue(vas_fir_binauralReflection1 *x, int tid);
/**
* Allocates buffers and checks for the optimal number of threads. You msut call this exactly once before executing vas_pRefl_global_enqueue or vas_pRefl_global_execute (ideally on program startup).
* @param[length] the length of the buffers you are going to process. Note: Changing buffer size at runtime is not yet supported.
*/
vas_fir_binauralReflection1 ***vas_pRefl_tasks_init(int length);
/**
* Stops the worker thread and frees memory. You must call this exactly once before any resources get deallocated, otherwise your program will probably crash or have memory leaks.
*/
void vas_pRefl_global_deinit(void);
/**
* threadpool implementation ov vas_pRefl_global_deinit. Does the exact same thing, but using the thpool.h library.
*/
void vas_pRefl_global_deinit_tp(void);
/**
* Executes all tasks in the queue. Execution is distributed on vas_pRefl_num_threads threads, which usually corresponds to the number of CPU cores of a system.
* @param[outInterleaved] the output audio buffer to finally write to.
* @param[length] The length of the audio buffers to process. Note: Neither different buffer lengths inside one execution, nor changing the buffer length between executions are supported right now.
*/
vas_threadLog vas_pRefl_global_execute(VAS_OUTPUTBUFFER *outInterleaved, int length);
/**
* threadpool implementation ov vas_pRefl_global_execute. Does the exact same thing, but using the thpool.h library.
*/
vas_threadLog vas_pRefl_global_execute_tp(VAS_OUTPUTBUFFER *outInterleaved, int length);
/**
* Multidimensional array [numThreads][maxNumTasks] that holds pointers to the tasks to execute in the future.
*/
extern vas_fir_binauralReflection1 ***vas_pRefl_tasks;
/**
* The number of threads that is created in each call of vas_pRefl_global_execute. Note that this is always 1 when vas_pRefl_tasks_init has not been called yet.
*/
extern int vas_pRefl_num_threads;
/**
* Set this flag at runtime to switch between single-threaded and multi-threaded execution. Can be handy for direct performance comparison.
*/
extern int vas_enable_multithreading;
/**
* Set this flag at runtime to switch between thpool.h library and "hand-made" pthread implementation.
*/
extern int vas_enable_threadpool;

#ifdef __cplusplus
}
#endif

#endif /* vas_pRefl_h */

#endif /* VAS_COMPILE_WITH_MULTITHREADING */
