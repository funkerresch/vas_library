//
//  vas_thpool_noMalloc.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 19.05.21.
//

#ifndef vas_thpool_noMalloc_h
#define vas_thpool_noMalloc_h

#ifdef __cplusplus
extern "C" {
#endif
#include "thpool.h"

// we do not want any memory allocation within the DSP loop
// it costs time and what if it really fails ?

int thpool_add_work_noHeap(struct thpool_* thpool_p, void (*function_p)(void*), void* arg_p);
void thpool_destroy_noHeap(struct thpool_* thpool_p);
struct thpool_* thpool_init_noHeap(int num_threads);

typedef struct vas_job { // same as thpools job which is not in the header but in source file
    struct vas_job*  prev;
    void   (*function)(void* arg);
    void*  arg;
} vas_job;

#ifdef __cplusplus
}
#endif


#endif /* vas_thpool_noMalloc_h */
