/**
 * @file vas_mem.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based virtual acoustics (mainly dynamic binaural synthesis) <br>
 * <br>
 * @brief Utilties for dynamic memory allocation <br>
 * <br>
 * Wrapper for memory allocation <br>
 * Max/MSP SDK suggests using the Max/MSP "sysmem_" - routines <br>
 * instead of malloc/calloc/free but it returns errors for very large <br>
 * memory allocations (or at least it has several years ago). Also pffft needs aligned memory so vas_mem uses <br>
 * posix_memalign and _aligned_malloc on WIN for everthing. <br>
 */

#ifndef vas_memory_h
#define vas_memory_h

#ifdef VAS_USE_PFFFT
#include "pffft.h"
#endif
#include <stdlib.h>
#include <string.h> /* memset */
//#include <unistd.h> /* close */
#ifdef PUREDATA
#include "m_pd.h"
#endif

#ifdef MAXMSPSDK
#include "ext.h"
#include "ext_obex.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)
#endif

void *vas_mem_alloc(long size);

void *vas_mem_resize(void *ptr, long size);

void vas_mem_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
