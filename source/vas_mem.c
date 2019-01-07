
#ifndef vas_memory_c
#define vas_memory_c

#include "vas_mem.h"

void *vas_mem_alloc(long size)
{
    
#ifdef MAXMSPSDK
    
    //return sysmem_newptrclear(size);
    return malloc(size);

#elif defined(PUREDATA)
    
    return malloc(size);
    
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
    
    return realloc(ptr, size);
    
#else
    
    return realloc(ptr, size);
    
#endif
    
}

void vas_mem_free(void *ptr)
{
    
#ifdef MAXMSPSDK
    
   // sysmem_freeptr(ptr);
    free(ptr);
    
#elif defined(PUREDATA)
    
    free(ptr);
    
#else
    
    free(ptr);
    
#endif
    
}

#endif
