//
//  thpool_noMalloc.c
//
// This is not in use anymore, because it is not safe in a real-time audio context.
// I only left it here for measuring purposes.
//
//  Variation of thpool.ch by Johan Hanssen Seferidis without mem alloc
//  Argument for thpool_add_work_noHeap should subclass vas_threadedArg and pass the job with it as first argument
//  Created by Thomas Resch on 23.04.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.

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

#if defined (VAS_USE_MULTITHREADCONVOLUTION) || defined (VAS_USE_MULTITHREADREFLECTION)

#ifdef __cplusplus
extern "C" {
#endif

#include "thpool.c"
#include <sched.h>
#include "vas_fir_binauralReflection1.h"

#define VAS_MAXNUMBEROFTHREADS 256 // no machine running unity or pd with more than 256 logical cores

int jobCounter = 0;
    
typedef struct vas_threadedArg
{
    vas_job *job;
} vas_threadedArg;

/* Add work to the thread pool */
int thpool_add_work_noHeap(thpool_* thpool_p, void (*function_p)(void*), void* arg_p){
    
    job* newjob = (job *)  ( (vas_threadedArg *)arg_p)->job;

    /* add function and argument */
    newjob->function=function_p;
    newjob->arg=arg_p;

    /* add job to queue */
    jobqueue_push(&thpool_p->jobqueue, newjob);

    return 0;
}

static void* thread_do_noHeap(struct thread* thread_p){

    /* Set thread name for profiling and debuging */
    char thread_name[32] = {0};
    snprintf(thread_name, 32, "thread-pool-%d", thread_p->id);

#if defined(__linux__)
    /* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
    prctl(PR_SET_NAME, thread_name);
#elif defined(__APPLE__) && defined(__MACH__)
    pthread_setname_np(thread_name);
#else
    err("thread_do(): pthread_setname_np is not supported on this system");
#endif

    /* Assure all threads have been created before starting serving */
    thpool_* thpool_p = thread_p->thpool_p;

    /* Register signal handler */
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = thread_hold;
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        err("thread_do(): cannot handle SIGUSR1");
    }

    /* Mark thread as alive (initialized) */
    pthread_mutex_lock(&thpool_p->thcount_lock);
    thpool_p->num_threads_alive += 1;
    pthread_mutex_unlock(&thpool_p->thcount_lock);

    while(threads_keepalive){

        bsem_wait(thpool_p->jobqueue.has_jobs);

        if (threads_keepalive){

            pthread_mutex_lock(&thpool_p->thcount_lock);
            thpool_p->num_threads_working++;
            pthread_mutex_unlock(&thpool_p->thcount_lock);

            /* Read job from queue and execute it */
            void (*func_buff)(void*);
            void*  arg_buff;
            job* job_p = jobqueue_pull(&thpool_p->jobqueue);
            if (job_p) {
                func_buff = job_p->function;
                arg_buff  = job_p->arg;
                func_buff(arg_buff);
            }

            pthread_mutex_lock(&thpool_p->thcount_lock);
            thpool_p->num_threads_working--;
            if (!thpool_p->num_threads_working) {
                pthread_cond_signal(&thpool_p->threads_all_idle);
            }
            pthread_mutex_unlock(&thpool_p->thcount_lock);
        }
    }
    pthread_mutex_lock(&thpool_p->thcount_lock);
    thpool_p->num_threads_alive --;
    pthread_mutex_unlock(&thpool_p->thcount_lock);

    return NULL;
}
    
/* Destroy the threadpool */
void thpool_destroy_noHeap(thpool_* thpool_p){
    /* No need to destory if it's NULL */
    if (thpool_p == NULL) return ;

    volatile int threads_total = thpool_p->num_threads_alive;

    /* End each thread 's infinite loop */
    threads_keepalive = 0;

    /* Give one second to kill idle threads */
    double TIMEOUT = 1.0;
    time_t start, end;
    double tpassed = 0.0;
    time (&start);
    while (tpassed < TIMEOUT && thpool_p->num_threads_alive){
        bsem_post_all(thpool_p->jobqueue.has_jobs);
        time (&end);
        tpassed = difftime(end,start);
    }

    /* Poll remaining threads */
    while (thpool_p->num_threads_alive){
        bsem_post_all(thpool_p->jobqueue.has_jobs);
        sleep(1);
    }

    int n;
    for (n=0; n < threads_total; n++){
        thread_destroy(thpool_p->threads[n]);
    }
    free(thpool_p->threads);
    free(thpool_p);
}

static int thread_init_noHeap (thpool_* thpool_p, struct thread** thread_p, int id){

    *thread_p = (struct thread*)malloc(sizeof(struct thread));
    if (*thread_p == NULL){
        err("thread_init(): Could not allocate memory for thread\n");
        return -1;
    }

    (*thread_p)->thpool_p = thpool_p;
    (*thread_p)->id       = id;
    
    pthread_attr_t tattr;
    int ret;
   
    struct sched_param oldparam;
    struct sched_param param;
    
    int policy;

    /* get scheduling policy of thread */
    ret = pthread_attr_getschedpolicy (&tattr, &policy);
    
    int newprio = sched_get_priority_max(policy);
    
    printf("%d %d\n", policy, newprio);
   // int policy = sched_getscheduler(0);

    /* initialized with default attributes */
    ret = pthread_attr_init (&tattr);

    /* safe to get existing scheduling param */
    ret = pthread_attr_getschedparam (&tattr, &param);

    /* set the priority; others are unchanged */
    param.sched_priority = newprio;

    /* setting the new scheduling param */
    ret = pthread_attr_setschedparam (&tattr, &param);

    pthread_create(&(*thread_p)->pthread, &tattr, (void * (*)(void *)) thread_do_noHeap, (*thread_p));
    pthread_detach((*thread_p)->pthread);
   
    return 0;
}

/* Initialise thread pool */
struct thpool_* thpool_init_noHeap(int num_threads){

    threads_on_hold   = 0;
    threads_keepalive = 1;

    if (num_threads < 0){
        num_threads = 0;
    }

    /* Make new thread pool */
    thpool_* thpool_p;
    thpool_p = (struct thpool_*)malloc(sizeof(struct thpool_));
    if (thpool_p == NULL){
        err("thpool_init(): Could not allocate memory for thread pool\n");
        return NULL;
    }
    thpool_p->num_threads_alive   = 0;
    thpool_p->num_threads_working = 0;

    /* Initialise the job queue */
    if (jobqueue_init(&thpool_p->jobqueue) == -1){
        err("thpool_init(): Could not allocate memory for job queue\n");
        free(thpool_p);
        return NULL;
    }

    /* Make threads in pool */
    thpool_p->threads = (struct thread**)malloc(num_threads * sizeof(struct thread *));
    if (thpool_p->threads == NULL){
        err("thpool_init(): Could not allocate memory for threads\n");
        jobqueue_destroy(&thpool_p->jobqueue);
        free(thpool_p);
        return NULL;
    }

    pthread_mutex_init(&(thpool_p->thcount_lock), NULL);
    pthread_cond_init(&thpool_p->threads_all_idle, NULL);

    /* Thread init */
    int n;
    for (n=0; n<num_threads; n++){
        thread_init_noHeap(thpool_p, &thpool_p->threads[n], n);
#if THPOOL_DEBUG
            printf("THPOOL_DEBUG: Created thread %d in pool \n", n);
#endif
    }

    /* Wait for threads to initialize */
    while (thpool_p->num_threads_alive != num_threads) {}

    return thpool_p;
}


#ifdef __cplusplus
}
#endif

#endif


