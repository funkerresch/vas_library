//
//  vas_threads.c
//
//  Created by Thomas Resch on 05.01.22.
//  Based on Johan Hanssen Seferides' Threadpool implementation
//  and Taymindis lock-free queue.
//  Removed Real-Time critical parts (locks and heap allocation),
//  and replaced the job queue with an atomic, lock-free queue.
//  Every job can pass it's own queue counter, the VAS_THREADS_WAIT_FOR_EMPTY_QUEUE()
//  macro maybe used for sync purposes. It waits until the corresponding
//  job counter is set to zero.
//  Licences are in vas_threads.h

#include "vas_threads.h"

static volatile int threads_keepalive;

// Windows implementation of atomic_flag

#ifndef ATOMIC_TEST_SET_AND_CLEAR_DEFINED

bool atomic_flag_test_and_set(volatile atomic_flag *flag)
{
    return _InterlockedExchange8((volatile char*)flag, 1) == 1;
}

bool atomic_flag_test_and_set_explicit(volatile atomic_flag* object, memory_order order)
{
    return atomic_flag_test_and_set(object);
}

void atomic_flag_clear(volatile atomic_flag *flag)
{
    _InterlockedExchange8((volatile char*)flag, 0);
}

void atomic_flag_clear_explicit(volatile atomic_flag* object, memory_order order)
{
    atomic_flag_clear(object);
}

#endif

/* I started with spinlocks, as this was the only thing I could find in the popular blogs of
the audio developer community about lock-free synchronisation mechanisms. They are (probably, haven't measured yet) a little faster than my
implementation, but due to their energy consumption not applicable for mobile applications. I left them
here for measuring purposes. They are not in use anymore.
*/

vas_spinMutex *vas_spinMutex_new(void)
{
    vas_spinMutex *x = malloc(sizeof(vas_spinMutex));
    atomic_flag_clear(&x->flag);
    atomic_flag_test_and_set(&x->flag);
    return x;
}

void vas_spinMutex_tryLock(vas_spinMutex *x)
{
    for (int i = 0; i < 5; ++i)
    {
        if (!atomic_flag_test_and_set(&x->flag))
            return;
    }

    for (int i = 0; i < 10; ++i)
    {
        if (!atomic_flag_test_and_set(&x->flag))
            return;

       // _mm_pause();
    }

    while (1)
    {
        for (int i = 0; i < 3000; ++i) {
            if (!atomic_flag_test_and_set(&x->flag))
                return;

//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
//            _mm_pause();
        }
    }
 }

void vas_spinMutex_unLock(vas_spinMutex *x)
{
    atomic_flag_clear(&x->flag);
}

/* Post to at least one thread
 * Since every thread has its own queue
 * it is not necessary to protect the semaphore value with a mutex.
 */

static void vas_threads_waitForSeconds(double seconds)
{
    double TIMEOUT = seconds;
    time_t start, end;
    double tpassed = 0.0;
    time (&start);
    while (tpassed < TIMEOUT){
        time (&end);
        tpassed = difftime(end,start);
    }
}

static void vas_threads_atomicBsem_post(vas_threads_atomicBsem *x)
{
    __LFQ_BOOL_COMPARE_AND_SWAP(&x->v , 0, 1);
    pthread_cond_signal(&x->cond);
}

/* Init semaphore to 1 or 0 */
static void vas_threads_atomicBsem_init(vas_threads_atomicBsem *x, int value) {
    if (value < 0 || value > 1) {
        printf("bsem_init(): Binary semaphore can take only values 1 or 0");
        exit(1);
    }
    pthread_mutex_init(&(x->mutex), NULL);
    pthread_cond_init(&(x->cond), NULL);
    x->v = value;
}

/* Wait on semaphore until it has value 1.
 * The only race that could happen here would be in the moment
 * the job queue is empty and besm_p->v would be set to 0 at
 * the same time as the main audio thread calls addJob and sets bsem_p->v to 1.
 * This is simply not possible as the semaphore is not shared with another
 * worker thread that could set the semaphore to 0. And while the thread is waiting
 * it can't set the semaphore. Therefore it has wait until the main audio thread
 * calls bsem_post.
 */

static void vas_threads_atomicBsem_wait(vas_threads_atomicBsem* x)
{
    pthread_mutex_lock(&x->mutex);                     // we have to lock the mutex here because pthread_cond_wait unlocks it before sleeping
    while (!__LFQ_BOOL_COMPARE_AND_SWAP(&x->v , 1, 0)) // but no other thread ever locks this mutex, therefore this is actually lock free.
        pthread_cond_wait(&x->cond, &x->mutex);

    pthread_mutex_unlock(&x->mutex);
}

static void *_vas_threads_dequeue(vas_threads_lfqueue *x)
{
    vas_threads_lfnode *head, *next;
    void *val;

    for (;;)
    {
        head = x->head;
        if (__LFQ_BOOL_COMPARE_AND_SWAP(&x->head, head, head))                  // This makes sure that no one else de/queued between reading into head
        {                                                                       // and compare and swap
            next = head->next;                                                  // the actual head is always head->next
            if (__LFQ_BOOL_COMPARE_AND_SWAP(&x->tail, head, head))              // again making sure that nothing happenend
            {
                if (next == NULL)                                               // We don't want to dequeue if head == tail == first dummy element
                {
                    val = NULL;
                    goto _done;
                }                                                               // if head == tail but next != NULL, do another round in
            }                                                                   // the for loop, things have changed
            else                                                                // We only ever get here if tail != head
            {
                if (next)                                                       // and next exists
                {
                    val = next->value;                                          // get our return value
                    vas_threads_job *freeJob =  x->head->value;                 // and get the job which will be kicked out of the queue
                    if (__LFQ_BOOL_COMPARE_AND_SWAP(&x->head, head, next))      // if no one else did something replace head with next
                    {
                        atomic_flag_clear(&(freeJob->available));               // old head is no longer in use, so we can clear its atomic_flag
                        break;                                                  // so the job/node can be used in the queue again.
                    }
                }
                
                else
                {                                                               // this could only happen if another thread would also be able
                    val = NULL;                                                 // to dequeue, in our case impossible.
                    goto _done;
                }
            }
        }
    }

_done:
    // __asm volatile("" ::: "memory");
    __LFQ_SYNC_MEMORY();
    return val;
}

static int _vas_threads_enqueue(vas_threads_lfqueue *x, void* value, vas_threads_lfnode *node)
{
    vas_threads_lfnode *tail;
    node->value = value;
    node->next = NULL;
    for (;;)
    {
        __LFQ_SYNC_MEMORY();
        tail = x->tail;
        if (__LFQ_BOOL_COMPARE_AND_SWAP(&tail->next, NULL, node)) // if (tail->next == NULL) replace tail->next with node
        {
            __LFQ_BOOL_COMPARE_AND_SWAP(&x->tail, tail, node); // new tail is node
            return 0;
        }
    }
    return -1;
}

int vas_threads_lfqueue_init(vas_threads_lfqueue *x)
{
    vas_threads_lfnode *base = malloc(sizeof(vas_threads_lfnode));
    vas_threads_job *baseJob = malloc(sizeof(vas_threads_job));
    
    base->value = baseJob; // we always need a first dummy element in order to perform atomic dequeuing
    base->next = NULL;

    x->head = x->tail = base; // Not yet to be free for first node only
    x->root_free = base; // we need to remember this for freeing
    x->size = 0;
    x->has_jobs = (struct vas_threads_atomicBsem*)malloc(sizeof(struct vas_threads_atomicBsem));
    vas_threads_atomicBsem_init(x->has_jobs, 0);

    return 0;
}

void vas_threads_lfqueue_destroy(vas_threads_lfqueue *x)
{
    vas_threads_lfnode *rtfree = x->root_free;

    if (rtfree)
        free(rtfree);
    
    x->size = 0;
}

int vas_threads_lfqueue_enq(vas_threads_lfqueue *x, void *value, vas_threads_lfnode *node)
{
    if (_vas_threads_enqueue(x, value, node))
        return -1;

    __LFQ_ADD_AND_FETCH(&x->size, 1);
    vas_threads_atomicBsem_post(x->has_jobs);
    return 0;
}

void* vas_threads_lfqueue_deq(vas_threads_lfqueue *lfqueue)
{
    void *v;
    if ( (v = _vas_threads_dequeue(lfqueue)) )
    {
         __LFQ_FETCH_AND_ADD(&lfqueue->size, -1);
         return v;
    }

    return NULL;
}

/* What each thread is doing
*
* In principle this is an endless loop. The only time this loop gets interuppted is once
* thpool_destroy() is invoked or the program exits.
*
* @param  thread        thread that will run this function
* @return nothing
*/

static void* vas_threads_executeJob(struct vas_thread* x)
{
    char thread_name[32] = {0};
    snprintf(thread_name, 32, "vas_threadPool-%d", x->id);

#if defined(__linux__)
    /* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
    prctl(PR_SET_NAME, thread_name);
#elif defined(__APPLE__) && defined(__MACH__)
    pthread_setname_np(thread_name);
#else
    printf("thread_do(): pthread_setname_np is not supported on this system");
#endif

    vas_threads *thpool_p = x->thpool_p;
    __LFQ_ADD_AND_FETCH(&thpool_p->num_threads_alive, 1);
    while(threads_keepalive)
    {
        vas_threads_atomicBsem_wait(x->jobQueue.has_jobs);
        void (*func_buff)(void*);
        void*  arg_buff;
        vas_threads_job *val = vas_threads_lfqueue_deq(&x->jobQueue);
        
        if (val)
        {
            func_buff = val->function;
            arg_buff  = val->arg;
            func_buff(arg_buff);
            
            // We have to prevent the thread from going to sleep if there are
            // still jobs in the queue.
            // As the semaphore is not shared with any other worker threads,
            // we can safely set its value to 1 if there are still jobs to do.

            if(!__LFQ_BOOL_COMPARE_AND_SWAP(&x->jobQueue.size, 0, 0))
                __LFQ_BOOL_COMPARE_AND_SWAP(&x->jobQueue.has_jobs->v , 0, 1);
            
            // The semaphore value v is the only one that could end up in a race
            // However, the main audio thread changes the semaphore value to 1 only.
            // So even if main and worker thread want to set v to 1 at the same time
            // no harm is done. Using atomic compare and swap it will happen only once anyway.
        }
    }
    
    __LFQ_ADD_AND_FETCH(&thpool_p->num_threads_alive, -1);
    return NULL;
}

static int vas_threads_initThread (vas_threads* x, struct vas_thread** thread_p, int id){

    *thread_p = (struct vas_thread*)malloc(sizeof(struct vas_thread));
    if (*thread_p == NULL){
        printf("thread_init(): Could not allocate memory for thread\n");
        return -1;
    }
 
    (*thread_p)->thpool_p = x;
    (*thread_p)->id       = id;
    
    vas_threads_lfqueue_init( &(*thread_p)->jobQueue);
    (*thread_p)->counter = 0;
    
    for(int i=0; i< VAS_MAX_QUEUE_LENGTH; i++)
        atomic_flag_clear(& ((*thread_p)->jobNodes[i].job.available));
     
    // priority is inherited from creating thread ?
    pthread_create(&(*thread_p)->pthread, NULL, (void * (*)(void *)) vas_threads_executeJob, (*thread_p));
    pthread_detach((*thread_p)->pthread);
    return 0;
}

struct vas_threads* vas_threads_init(int numberOfThreads)
{
    if (numberOfThreads <= 0){
        return NULL;
    }
    
    threads_keepalive = 1;

    vas_threads *x = (struct vas_threads*)malloc(sizeof(struct vas_threads));
    x->num_threads_alive   = 0;
    x->globalSize = numberOfThreads;
    x->threads = (struct vas_thread**) malloc(numberOfThreads * sizeof(struct vas_thread *));

    for (int n=0; n<numberOfThreads; n++)
        vas_threads_initThread(x, &x->threads[n], n);

    /* Wait for threads to initialize */
#if defined __GNUC__ || defined __CYGWIN__ || defined __MINGW32__ || defined __APPLE__
    while (!__LFQ_BOOL_COMPARE_AND_SWAP(&x->num_threads_alive, numberOfThreads, numberOfThreads))
        ;       // For some reason MSVS does not like this..
#else
    vas_threads_waitForSeconds(2.5); // ..therefore we simply we wait 2.5s which should be more than enough
#endif
    
    return x;
}

static void vas_threads_destroyThread (vas_thread* x)
{
    vas_threads_lfqueue_destroy(&x->jobQueue);
    free(x);
}

void vas_threads_destroy1(vas_threads *x, int *queue2Clear)
{
    if (x == NULL)
        return ;

    volatile int threads_total = x->globalSize;
    
    threads_keepalive = 0;
    
    // Making sure that threads are not waiting at semaphore
    // As the jobqueue enqueuing checks for NULL pointer this is OK
    // even if jobqueue is empty
    
    for (int n=0; n < threads_total; n++)
        vas_threads_atomicBsem_post(x->threads[n]->jobQueue.has_jobs);

    // Give at least 0.25 seconds before calling destroy() and start killing idle threads
     
    vas_threads_waitForSeconds(0.25);
    
    while(!__LFQ_BOOL_COMPARE_AND_SWAP(queue2Clear, 0, 0)) // allow main audio thread to finish
        __LFQ_ADD_AND_FETCH(queue2Clear, -1);
    
    vas_threads_waitForSeconds(0.25); // wait until main audio thread has finished
    
    for (int n=0; n < threads_total; n++)
        vas_threads_destroyThread(x->threads[n]);

    free(x->threads);
    free(x);
}

vas_threads_jobNode *vas_threads_findFreeJobNode(vas_thread *x)
{
    for(int i=0; i< VAS_MAX_QUEUE_LENGTH; i++)
    {
        if(!atomic_flag_test_and_set(&x->jobNodes[i].job.available)) // if flag has been cleared either through init thread or after finishing a job
        {
            x->jobNodes[i].node.next = NULL;
            return &x->jobNodes[i];
        }
    }
    return NULL;
}

int vas_threads_addWork(vas_threads* x, int threadNumber, void (*function_p)(void*), void* arg_p)
{
    vas_threads_jobNode *node = vas_threads_findFreeJobNode(x->threads[threadNumber]);
    if(node == NULL)
    {
        printf("No job available");
        return -1; // no jobs available, could not add work:(
    }
    
    vas_threads_job* newjob = &node->job;
    vas_threads_lfnode *newNode = &node->node;

    newjob->function=function_p;
    newjob->arg=arg_p;

    vas_threads_lfqueue_enq(&x->threads[threadNumber]->jobQueue, newjob, newNode);
  
    return 0;
}








