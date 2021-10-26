
#ifndef vas_memory_c
#define vas_memory_c

#include "vas_mem.h"

// pffft's aligned memory allocation crashes on osx
// posix memalign is used on posix systems
// on windows
//
// #define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)
//
// is defined in vas_mem.h
// there is no difference anymore between vas_mem_alloc & vas_mem_resize

void *vas_mem_alloc(long size)
{
    void *tmp;
    posix_memalign(&tmp, 64, size);
    long long *setMem = tmp;
    memset(setMem, 0, size);
    return tmp;
}

void *vas_mem_resize(void *ptr, long size)
{
    void *tmp;
#ifdef _WIN32
    _aligned_free(ptr);
#else
    if(ptr)
        free(ptr);
#endif
    
    posix_memalign(&tmp, 64, size);
    long long *setMem = tmp;
    memset(setMem, 0, size);
    return tmp;
}

void vas_mem_free(void *ptr)
{
#ifdef _WIN32
    _aligned_free(ptr);
#else
    if(ptr)
        free(ptr);
#endif
}

#endif
