/**
 * @file vas_threads.h
 * @author Thomas Resch <br>
 * Audio Communication Group, TU-Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * <br>
 * @brief A (in principle) lock- and heapfree pthreads-based threadpool<br>
 * <br>
 * Created by Thomas Resch on 05.01.22. <br>
 * Based on Johan Hanssen Seferides' Threadpool implementation (https://github.com/Pithikos/C-Thread-Pool),<br>
 * and Taymindis lock-free queue (https://github.com/Taymindis/lfqueue).<br>
 * Removed Real-Time critical parts (locks and heap allocation),<br>
 * and replaced the job queue with an atomic, lock-free queue.<br>
 * Every "job-group" can pass a shared queue counter, the VAS_THREADS_WAIT_FOR_EMPTY_QUEUE<br>
 * macro maybe used for sync purposes. It waits until the corresponding<br>
 * job counter is set to zero.<br>
 * Example usage is shown vas_fir_partitioned (non-equal-segmented parallel convolution)<br>
 * <br>
 */

/*
    https://github.com/Pithikos/C-Thread-Pool
 
    The MIT License (MIT)

    Copyright (c) 2016 Johan Hanssen Seferidis

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/


//  Job queue has been replaced with a lock free FIFO queue, based
//  on Taymindis lock-free queue:

/*
    https://github.com/Taymindis/lfqueue
   
    BSD 2-Clause License

    Copyright (c) 2018, Taymindis Woon
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//  All following platform independent macros for atomic operations below are copied also from Taymindis's lock-free queue
//  Thanks very much for those

#ifndef vas_thpool_h
#define vas_thpool_h

#define VAS_MAX_QUEUE_LENGTH 256

#    if defined(__APPLE__)
#        include <AvailabilityMacros.h>
#    else
#        ifndef _POSIX_C_SOURCE
#            define _POSIX_C_SOURCE 200809L
#        endif
#    endif
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#if defined(__linux__)
#include <sys/prctl.h>

#endif

#if defined __GNUC__ || defined __CYGWIN__ || defined __MINGW32__ || defined __APPLE__

#include <stdatomic.h>
#define ATOMIC_TEST_SET_AND_CLEAR_DEFINED
#define __LFQ_VAL_COMPARE_AND_SWAP __sync_val_compare_and_swap
#define __LFQ_BOOL_COMPARE_AND_SWAP __sync_bool_compare_and_swap
#define __LFQ_FETCH_AND_ADD __sync_fetch_and_add
#define __LFQ_ADD_AND_FETCH __sync_add_and_fetch
#define __LFQ_YIELD_THREAD sched_yield
#define __LFQ_SYNC_MEMORY __sync_synchronize

#else

#include <Windows.h>
#include <stdbool.h>
#include <synchapi.h>
#include <time.h>
#ifdef _WIN64
inline BOOL __SYNC_BOOL_CAS(LONG64 volatile *dest, LONG64 input, LONG64 comparand) {
    return InterlockedCompareExchangeNoFence64(dest, input, comparand) == comparand;
}
#define __LFQ_VAL_COMPARE_AND_SWAP(dest, comparand, input) \
    InterlockedCompareExchangeNoFence64((LONG64 volatile *)dest, (LONG64)input, (LONG64)comparand)
#define __LFQ_BOOL_COMPARE_AND_SWAP(dest, comparand, input) \
    __SYNC_BOOL_CAS((LONG64 volatile *)dest, (LONG64)input, (LONG64)comparand)
#define __LFQ_FETCH_AND_ADD InterlockedExchangeAddNoFence64
#define __LFQ_ADD_AND_FETCH InterlockedAddNoFence64
#define __LFQ_SYNC_MEMORY MemoryBarrier

#else
#ifndef asm
#define asm __asm
#endif
inline BOOL __SYNC_BOOL_CAS(LONG volatile *dest, LONG input, LONG comparand) {
    return InterlockedCompareExchangeNoFence(dest, input, comparand) == comparand;
}
#define __LFQ_VAL_COMPARE_AND_SWAP(dest, comparand, input) \
    InterlockedCompareExchangeNoFence((LONG volatile *)dest, (LONG)input, (LONG)comparand)
#define __LFQ_BOOL_COMPARE_AND_SWAP(dest, comparand, input) \
    __SYNC_BOOL_CAS((LONG volatile *)dest, (LONG)input, (LONG)comparand)
#define __LFQ_FETCH_AND_ADD InterlockedExchangeAddNoFence
#define __LFQ_ADD_AND_FETCH InterlockedAddNoFence
#define __LFQ_SYNC_MEMORY() asm mfence

#endif
#include <windows.h>
#define __LFQ_YIELD_THREAD SwitchToThread
#endif

#if defined __GNUC__ || defined __CYGWIN__ || defined __MINGW32__ || defined __APPLE__
#define lfq_time_t long
#define lfq_get_curr_time(_time_sec) \
struct timeval _time_; \
gettimeofday(&_time_, NULL); \
*_time_sec = _time_.tv_sec
#define lfq_diff_time(_etime_, _stime_) _etime_ - _stime_
#else
#define lfq_time_t time_t
#define lfq_get_curr_time(_time_sec) time(_time_sec)
#define lfq_diff_time(_etime_, _stime_) difftime(_etime_, _stime_)
#endif

#if defined __GNUC__ || defined __CYGWIN__ || defined __MINGW32__ || defined __APPLE__
#define lfq_bool_t int
#else
#ifdef _WIN64
#define lfq_bool_t int64_t
#else
#define lfq_bool_t int
#endif
#endif

#ifndef ATOMIC_TEST_SET_AND_CLEAR_DEFINED

typedef enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
} memory_order;

// Windows implementation of atomic_flag

typedef struct atomic_flag { bool _Value; } atomic_flag;

bool atomic_flag_test_and_set(volatile atomic_flag *flag);

bool atomic_flag_test_and_set_explicit(volatile atomic_flag* object, memory_order order);

void atomic_flag_clear(volatile atomic_flag *flag);

void atomic_flag_clear_explicit(volatile atomic_flag* object, memory_order order);

#endif

// I changed this to arm yield and x86_64 _mm_pause and renamed the old ones
// to ..SLEEP_AND_WAIT.. The sleep version sends (if called from there) the whole main audio thread
// sleeping. This might be useful for special cases where just a single, very intense
// DSP algorithm is calculated. But not when I want to have several instances of the same
// plugin. An interessting extension could be to count the instances that are currently waiting
// with another lock-free queue. If all are waiting for the worker threads, we can send the main audio
// thread to sleep and do a "real" yield.

#if TARGET_CPU_ARM64
#define VAS_THREADS_WAIT_FOR_EMPTY_QUEUE(A) while(!__LFQ_BOOL_COMPARE_AND_SWAP(A, 0, 0)) \
                                                asm volatile("yield");
#elif TARGET_CPU_X86_64
#define VAS_THREADS_WAIT_FOR_EMPTY_QUEUE(A) while(!__LFQ_BOOL_COMPARE_AND_SWAP(A, 0, 0)) \
                                                _mm_pause();
#endif

//#ifdef __arm__
//#define VAS_THREADS_WAIT_FOR_EMPTY_QUEUE(A) while(!__LFQ_BOOL_COMPARE_AND_SWAP(A, 0, 0)) \
//                                                asm volatile("yield");
//#else
//#define VAS_THREADS_WAIT_FOR_EMPTY_QUEUE(A) while(!__LFQ_BOOL_COMPARE_AND_SWAP(A, 0, 0)) \
//                                                _mm_pause();
//#endif

// different sources say different things about nanosleep in this context (RT Audio Thread)
// it works but maybe there is a better alternative
// WINDOWS is in ms, should be fast enough too, but clearly depends on the application.

#if defined __GNUC__ || defined __CYGWIN__ || defined __MINGW32__ || defined __APPLE__
#define VAS_THREADS_SLEEP_AND_WAIT_FOR_EMPTY_QUEUE(A) while(!__LFQ_BOOL_COMPARE_AND_SWAP(A, 0, 0)) \
                                                          nanosleep((const struct timespec[]){{0, 1}}, NULL);
#else
#define VAS_THREADS_SLEEP_AND_WAIT_FOR_EMPTY_QUEUE(A) while(!__LFQ_BOOL_COMPARE_AND_SWAP(A, 0, 0)) \
                                                          Sleep(1);
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* I started with spinlocks which are supposed to wait in the worker threads until there is something to do. This was the only thing I could find in the popular blogs of
the audio developer community about lock-free synchronisation mechanisms. They are (probably, I haven't measured yet) a little faster than my
implementation, but due to their energy consumption not applicable for mobile applications. I left them
here for measuring purposes.
*/

typedef struct vas_spinMutex
{
    atomic_flag flag;
} vas_spinMutex;

vas_spinMutex *vas_spinMutex_new(void);


void vas_spinMutex_tryLock(vas_spinMutex *x);


void vas_spinMutex_unLock(vas_spinMutex *x);

/**
 * @brief Struct vas_threads_job. <br>
 * vas_threads_job contains function and arguments <br>
 * passed from the main thread. <br>
 */

typedef struct vas_threads_job
{
    void   (*function)(void* arg);
    void*  arg;
    atomic_flag available;
} vas_threads_job;

/**
 * @brief vas_threads_lfnode. <br>
 * vas_threads_lfnode is a list node <br>
 * containing a vas_thread_job as value. <br>
 * On dequeuing the job is returned.
 *
 */

typedef struct vas_threads_lfnode
{
    void * value;
    struct vas_threads_lfnode *next;
} vas_threads_lfnode;

/**
 * @brief vas_threads_atomicBsem <br>
 * The binary semaphore is used to wait <br>
 * until a job is pushed to the thread's queue <br>
 * The mutex is not accessed by any <br>
 * other thread, v is set using <br>
 * the atomic compare and swap operation <br>
 */

typedef struct vas_threads_atomicBsem
{
    pthread_mutex_t mutex;
    pthread_cond_t   cond;
    int v;
} vas_threads_atomicBsem;

/**
 * @brief vas_threads_jobNode. <br>
 * JobNodes are pre-allocated with max number  = VAS_MAX_QUEUE_LENGTH <br>
 * for each thread. Every Vas_Unity_Spatializer adds one job/thread each <br>
 * audio frame. As long as the CPU is fast enough this makes it possible <br>
 * to calculate VAS_MAX_QUEUE_LENGTH audio sources at the same time. <br>
 */

typedef struct vas_threads_jobNode
{
    vas_threads_job job;
    vas_threads_lfnode node;
} vas_threads_jobNode;

/**
 * @brief vas_threads_lfqueue. <br>
 * vas_threads_lfqueue contains the jobs of
 * thread.
 */

typedef struct vas_threads_lfqueue
{
    vas_threads_lfnode *head, *tail, *root_free, *move_free;
    volatile size_t size;
    vas_threads_atomicBsem *has_jobs;
} vas_threads_lfqueue;

/**
 * @brief vas_thread.<br>
 * vas_thread contains the pthread, <br>
 * the queue and a prealloated number of nodes so they <br>
 * don't have to be allocated on the heap somewhere. If <br>
 * no jobNodes are available, the job is simply discarded. <br>
 */

typedef struct vas_thread
{
    int       id;                        /* friendly id               */
    pthread_t pthread;                   /* pointer to actual thread  */
    struct vas_threads* thpool_p;        /* access to thpool          */
    vas_threads_lfqueue jobQueue;
    vas_threads_jobNode jobNodes[VAS_MAX_QUEUE_LENGTH];
    int counter;
} vas_thread;

/**
 * @brief vas_threads.<br>
 * vas_threads is not a "real" threadpool.
 * It is meant for an evenly distriburted work load <br>
 * in each thread. <br>
 * The calling thread chooses the worker thread explicitly. <br>
 */


typedef struct vas_threads
{
    vas_thread**   threads;               /* pointer to threads        */
    volatile int num_threads_alive;      /* threads currently alive   */
    int globalSize;
} vas_threads;

/**
 * @brief  Initialize vas_threads
 *
 * Initializes a threadpool. This function will not return until all
 * threads have initialized successfully.
 *
 * @example
 *
 *    ..
 *    vas_threads *x;                     //First we declare a threadpool
 *    x = vas_threads_init(4);               //then we initialize it to 4 threads
 *    ..
 *
 * @param  numberOfThreads   number of threads to be created in the threadpool
 * @return vas_threads    created threadpool on success,
 *                       NULL on error
 */
vas_threads *vas_threads_init(int numberOfThreads);

/**
 * @brief Add work to the job queue
 *
 * Takes an action and its argument and adds it to the threadpool's job queue.
 * If you want to add to work a function with more than one arguments then
 * a way to implement this is by passing a pointer to a structure.
 *
 * NOTICE: You have to cast both the function and argument to not get warnings.
 *
 * @example
 *
 *    void print_num(int num){
 *       printf("%d\n", num);
 *    }
 *
 *    int main() {
 *       ..
 *       int a = 10;
 *       vas_threads_add_work(thpool, 0,  (void*)print_num, (void*)a);
 *       ..
 *    }
 *
 * @param  x    vas_threads to which the work will be added
 * @param  threadNumber  The  threadnumber to which the work will be added
 * @param  function_p    pointer to function to add as work
 * @param  arg_p         pointer to an argument
 * @return 0 on success, -1 otherwise.
 */

int vas_threads_addWork(vas_threads* x, int threadNumber, void (*function_p)(void*), void* arg_p);

/**
 * @brief Prepare destroying the threadpool
 *
 * This will wait for the currently active threads to finish the last job.
 * Since the main audio thread may wait using  vas_threads_waitForEmptyQueue()
 *
 * @param x     the threadpool to prepare
 * @return nothing
 */

void vas_threads_destroy1(vas_threads* x, int* queue2Clear);

#ifdef __cplusplus
}
#endif


#endif /* vas_thpool_h */
