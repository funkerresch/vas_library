
#ifndef vas_memory_c
#define vas_memory_c

#include "vas_mem.h"

void *vas_mem_alloc(long size)
{
    
#ifdef MAXMSPSDK
    
    //return sysmem_newptrclear(size);
    return malloc(size);

#elif defined(PUREDATA)
#ifdef VAS_USE_AVX
        _mm_malloc(32,32);
#else
    return malloc(size);
#endif
    
#else
    
    return malloc(size);
    
#endif
    
}

void *vas_mem_resize(void *ptr, long size)
{
    
#ifdef MAXMSPSDK
    
   // return sysmem_resizeptrclear(ptr, size);
    return realloc(ptr, size);
    
#elif defined(PUREDATA)
//#ifdef VAS_USE_AVX
//    _mm_malloc(32,32);
//#else
    void *tmp = realloc(ptr, size);
    memset(tmp, 0, size);
    return tmp;
//#endif
    
#else

    void *tmp = realloc(ptr, size);
    memset(tmp, 0, size);
    
    return tmp;
    
    //return realloc(ptr, size);
    
#endif
    
}

void vas_mem_free(void *ptr)
{
    
#ifdef MAXMSPSDK
    
   // sysmem_freeptr(ptr);
    free(ptr);
    
#elif defined(PUREDATA)
    if(ptr)
        free(ptr);
    
#else
    if(ptr)
        free(ptr);
    
#endif
    
}

#endif
