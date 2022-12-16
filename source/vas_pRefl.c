//
//  vas_pRefl.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 10.02.21.
//

#include "vas_pRefl.h"
#ifdef vas_pRefl_h

#include "vas_mem.h"
#include <sys/param.h>
#include "thpool.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    int vas_enable_multithreading = false;
    int vas_enable_threadpool = false;
    vas_fir_binauralReflection1 ***vas_pRefl_tasks = NULL;
    vas_pConv_tempBuffer *vas_pRefl_tempBuffer = NULL;
    int *vas_pRefl_tasks_ptr = NULL;
    int vas_pRefl_num_threads = 1;
    
    /* pthread implementation */
    pthread_t *vas_pRefl_threads = NULL;
    
    /* threadpool implementation */
    threadpool tpool = NULL;
    
    /* functions valid for both implementations */
    
    void vas_pRefl_global_enqueue(vas_fir_binauralReflection1 *x, int tid) {
        if(vas_pRefl_tasks_ptr[tid] < VAS_PREF_MAX_TASKS) {
            vas_pRefl_tasks[tid][vas_pRefl_tasks_ptr[tid]] = x;
            x->thread = tid;
            x->index = vas_pRefl_tasks_ptr[tid];
            vas_pRefl_tasks_ptr[tid]++;
        }
    }
    
    vas_fir_binauralReflection1 ***vas_pRefl_tasks_init(int length) {
        vas_pRefl_num_threads = 8;
        vas_pRefl_tasks_ptr = (int*) vas_mem_alloc(vas_pRefl_num_threads * sizeof(int));
        vas_fir_binauralReflection1 ***x = (vas_fir_binauralReflection1 ***) vas_mem_alloc(vas_pRefl_num_threads * sizeof(vas_fir_binauralReflection1**));
        for(int i = 0; i < vas_pRefl_num_threads; i++) {
            vas_pRefl_tasks_ptr[i] = 0;
            x[i] = (vas_fir_binauralReflection1**) vas_mem_alloc(length * sizeof(vas_fir_binauralReflection1*));
        }
        return x;
    }
    
    /* pthread implementation */
    
    void vas_pRefl_global_deinit(void) {
        
        /* Free memory */
        
        if(vas_pRefl_threads) {
            for (int i = 0; i < vas_pRefl_num_threads; i++) {
                if(vas_pRefl_threads[i]) {
//                    pthread_cancel(vas_pRefl_threads[i]);
                    pthread_join(vas_pRefl_threads[i], NULL);
                }
            }
        }
        
        if(vas_pRefl_tasks) {
            for(int i = 0; i < vas_pRefl_num_threads; i++) {
                vas_mem_free(vas_pRefl_tasks[i]);
            }
            vas_mem_free(vas_pRefl_tasks);
        }
        
        if(vas_pRefl_tempBuffer != NULL) {
            for(int i = 0; i < vas_pRefl_num_threads; i++) {
                vas_mem_free(vas_pRefl_tempBuffer->outL[i]);
                vas_mem_free(vas_pRefl_tempBuffer->outR[i]);
            }
            vas_mem_free(vas_pRefl_tempBuffer->outL);
            vas_mem_free(vas_pRefl_tempBuffer->outR);
            vas_mem_free(vas_pRefl_tempBuffer);
        }
        
        if(tpool) thpool_destroy(tpool);
        
        
        vas_mem_free(vas_pRefl_tasks_ptr);
        
        /* Set pointers to NULL */
        
        vas_pRefl_tempBuffer = NULL;
        vas_pRefl_threads = NULL;
        vas_pRefl_tasks = NULL;
        tpool = NULL;
        vas_pRefl_tasks_ptr = NULL;
    }
    
    void *vas_pRefl_executeJob_pthread(void *arg) {
        //pthread_setname_np("ReflectionWorker");
        vas_pConv_firJob *j = (vas_pConv_firJob*)arg;
        int tid = j->tid;
        for(int i = j->start; i < j->end; i++) {
            vas_fir_binauralReflection1* r = vas_pRefl_tasks[tid][i];
            if(r != NULL) {
                /* Another thread may attempt to delete r while we are accessing it, so we have to protect it by acquiring the lock. Note that the lock only protects r from being deleted, NOT from being modified! */
                if(pthread_mutex_trylock(&r->lock) == 0) {
                    vas_fir_binauralReflection1_process(r, j->outL, j->outR, j->length);
                    r->thread = -1;
                    r->index = -1;
                    pthread_mutex_unlock(&r->lock);
                }
            }
        }
        pthread_exit(&tid);
    }
    
    vas_threadLog vas_pRefl_global_execute(VAS_OUTPUTBUFFER *outInterleaved, int length) {
        /*prepare log and temporary output buffers for threads*/
        if(vas_pRefl_tempBuffer == NULL) {
            vas_pRefl_tempBuffer = vas_mem_alloc(sizeof(vas_pConv_tempBuffer));
            vas_pRefl_tempBuffer->nBuffers = vas_pRefl_num_threads;
            vas_pRefl_tempBuffer->nSamples = length;
            vas_pRefl_tempBuffer->outL = vas_mem_alloc(vas_pRefl_num_threads * sizeof(VAS_OUTPUTBUFFER*));
            vas_pRefl_tempBuffer->outR = vas_mem_alloc(vas_pRefl_num_threads * sizeof(VAS_OUTPUTBUFFER*));
            for(int i = 0; i < vas_pRefl_num_threads; i++) {
                vas_pRefl_tempBuffer->outL[i] = vas_mem_alloc(length * sizeof(VAS_OUTPUTBUFFER));
                vas_pRefl_tempBuffer->outR[i] = vas_mem_alloc(length * sizeof(VAS_OUTPUTBUFFER));
            }
        }
        else if(vas_pRefl_tempBuffer->nBuffers != vas_pRefl_num_threads || vas_pRefl_tempBuffer->nSamples != length) {
            //TODO:
        }
        vas_threadLog log;
        log.numJobs = 0;
        for(int i = 0; i < vas_pRefl_num_threads; i++) {
            log.numJobs += vas_pRefl_tasks_ptr[i];
            vas_utilities_writeZeros(length, vas_pRefl_tempBuffer->outL[i]);
            vas_utilities_writeZeros(length, vas_pRefl_tempBuffer->outR[i]);
        }
        log.numThreads = vas_pRefl_num_threads;
        log.numJobsPerThread = log.numJobs / vas_pRefl_num_threads;
        
        
        /*create worker threads*/
        int tid = 0;
        vas_pConv_firJob threadArgs[vas_pRefl_num_threads];
        pthread_t threads[vas_pRefl_num_threads];
        vas_pRefl_threads = threads;
#ifdef VAS_ENABLE_MULTITHREAD_PROFILING
        double wt = vas_util_getWallTime();
        double ct = vas_util_getCPUTime();
#endif
        while(tid < vas_pRefl_num_threads) {
            threadArgs[tid] = (vas_pConv_firJob){0, vas_pRefl_tasks_ptr[tid], tid, NULL, vas_pRefl_tempBuffer->outL[tid], vas_pRefl_tempBuffer->outR[tid], length };
            pthread_create(&threads[tid], NULL, vas_pRefl_executeJob_pthread, &threadArgs[tid]);
            tid++;
        }
        //TODO: Find out why it is apparently more cpu-friendly to NOT do anything in the main thread?!
        for(int i = 0; i < vas_pRefl_num_threads; i++) {
            pthread_join(threads[i], NULL);
            vas_pRefl_tasks_ptr[i] = 0;
        }
#ifdef VAS_ENABLE_MULTITHREAD_PROFILING
        wt = vas_util_getWallTime() - wt;
        ct = vas_util_getCPUTime() - ct;
        log.wallTime = wt * 1000;
        log.cpuTime = ct * 1000;
        log.idleTime = (wt - ct/vas_pRefl_num_threads) * 1000;
        printf("pthread log: wt=%f, cput=%f, idle=%f, @ %d threads.\n", log.wallTime, log.cpuTime, log.idleTime, vas_pRefl_num_threads);
#endif
        
        /*clean up and write samples to output buffers*/
        
        vas_pRefl_threads = NULL;
        for(int i = 0; i < vas_pRefl_num_threads; i++) {
            for(unsigned int n = 0; n < length; n++)
            {
                outInterleaved[n * 2] += vas_pRefl_tempBuffer->outL[i][n];
                outInterleaved[n * 2 + 1] += vas_pRefl_tempBuffer->outR[i][n];
            }
        }

        return log;
    }
    
    /* threadpool implementation */
    
    void vas_pRefl_executeJob_tp(void *arg) {
        vas_pConv_firJob *j = (vas_pConv_firJob*)arg;
        int tid = j->tid;
        for(int i = j->start; i < j->end; i++) {
            vas_fir_binauralReflection1* r = vas_pRefl_tasks[tid][i];
            /* r is NULL when a vas_fir_binauralReflection1 struct has been deleted after it was pushed into vas_pRefl_tasks, so we have to check. */
            if(r != NULL) {
                /* Another thread may attempt to delete r while we are accessing it, so we have to protect it by acquiring the lock. Note that the lock only protects r from being deleted, NOT from being modified! */
                if(pthread_mutex_trylock(&r->lock) == 0) {
                    vas_fir_binauralReflection1_process(r, j->outL, j->outR, j->length);
                    r->thread = -1;
                    r->index = -1;
                    pthread_mutex_unlock(&r->lock);
                }
            }
        }
    }
    
    vas_threadLog vas_pRefl_global_execute_tp(VAS_OUTPUTBUFFER *outInterleaved, int length) {
            /*prepare log and temporary output buffers for threads*/
            if(vas_pRefl_tempBuffer == NULL) {
                vas_pRefl_tempBuffer = vas_mem_alloc(sizeof(vas_pConv_tempBuffer));
                vas_pRefl_tempBuffer->nBuffers = vas_pRefl_num_threads;
                vas_pRefl_tempBuffer->nSamples = length;
                vas_pRefl_tempBuffer->outL = vas_mem_alloc(vas_pRefl_num_threads * sizeof(VAS_OUTPUTBUFFER*));
                vas_pRefl_tempBuffer->outR = vas_mem_alloc(vas_pRefl_num_threads * sizeof(VAS_OUTPUTBUFFER*));
                for(int i = 0; i < vas_pRefl_num_threads; i++) {
                    vas_pRefl_tempBuffer->outL[i] = vas_mem_alloc(length * sizeof(VAS_OUTPUTBUFFER));
                    vas_pRefl_tempBuffer->outR[i] = vas_mem_alloc(length * sizeof(VAS_OUTPUTBUFFER));
                }
            }
            else if(vas_pRefl_tempBuffer->nBuffers != vas_pRefl_num_threads || vas_pRefl_tempBuffer->nSamples != length) {
                //TODO:
            }
            vas_threadLog log;
            log.numJobs = 0;
            for(int i = 0; i < vas_pRefl_num_threads; i++) {
                log.numJobs += vas_pRefl_tasks_ptr[i];
                vas_utilities_writeZeros(length, vas_pRefl_tempBuffer->outL[i]);
                vas_utilities_writeZeros(length, vas_pRefl_tempBuffer->outR[i]);
            }
            log.numThreads = vas_pRefl_num_threads;
            log.numJobsPerThread = log.numJobs / vas_pRefl_num_threads;
            
            
            /*create worker threads*/
            int tid = 0;
            vas_pConv_firJob threadArgs[vas_pRefl_num_threads];
            if(tpool == NULL) {
                tpool = thpool_init(vas_pRefl_num_threads);
            }
    #ifdef VAS_ENABLE_MULTITHREAD_PROFILING
            double wt = vas_util_getWallTime();
            double ct = vas_util_getCPUTime();
    #endif
            while(tid < vas_pRefl_num_threads) {
                threadArgs[tid] = (vas_pConv_firJob){0, vas_pRefl_tasks_ptr[tid], tid, NULL, vas_pRefl_tempBuffer->outL[tid], vas_pRefl_tempBuffer->outR[tid], length };
                thpool_add_work(tpool, vas_pRefl_executeJob_tp, &threadArgs[tid]);
                tid++;
            }
            thpool_wait(tpool);
            for(int i = 0; i < vas_pRefl_num_threads; i++) {
                vas_pRefl_tasks_ptr[i] = 0;
            }
    #ifdef VAS_ENABLE_MULTITHREAD_PROFILING
            wt = vas_util_getWallTime() - wt;
            ct = vas_util_getCPUTime() - ct;
            log.wallTime = wt * 1000;
            log.cpuTime = ct * 1000;
            log.idleTime = (wt - ct/vas_pRefl_num_threads) * 1000;
            printf("threadpool log: wt=%f, cput=%f, idle=%f @ %d threads.\n", log.wallTime, log.cpuTime, log.idleTime, vas_pRefl_num_threads);
    #endif
            
            /*clean up and write samples to output buffers*/
            
            for(int i = 0; i < vas_pRefl_num_threads; i++) {
                for(unsigned int n = 0; n < length; n++)
                {
                    outInterleaved[n * 2] += vas_pRefl_tempBuffer->outL[i][n];
                    outInterleaved[n * 2 + 1] += vas_pRefl_tempBuffer->outR[i][n];
                }
            }

            return log;
        }
        
#ifdef __cplusplus
}
#endif

#endif /* vas_pRefl_h */
