/**
 * @file vas_mem.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based virtual acoustics (mainly dynamic binaural synthesis) <br>
 * <br>
 * @brief Utilties for dynamic memory allocation <br>
 * <br>
 * Wrapper for memory allocation
 * Max/MSP SDK suggests using the Max/MSP "sysmem_" - routines
 * instead of malloc/calloc/free
 * So for Max/MSP define the Preprocessor macro "MAXMSPSDK"
 */

#ifndef vas_memory_h
#define vas_memory_h

#include "pffft.h"
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

/**
 * \brief Tries to allocate the desired amount of memory. The allocated memory will be 64-byte aligned.
 *
 * @param size The amount of memory to allocate.
 *
 * @return A pointer to the allocated chunk of 64-byte aligned memory, or NULL if allocation failed.
 */
void *vas_mem_alloc(long size);

/**
 * \brief Tries to re-allocate memory of the desired amount to the given pointer. The allocated memory will be 64-byte aligned.
 *
 * @param ptr The pointer whose memory should be resized.
 * @param size The amount of memory to allocate.
 *
 * @return A pointer to the allocated chunk of 64-byte aligned memory, or NULL if re-allocation failed.
 */
void *vas_mem_resize(void *ptr, long size);

/**
 * \brief Frees a pointer.
 *
 * @param ptr A pointer previously allocated by vas_mem_alloc(1).
*/
void vas_mem_free(void *ptr);

#ifdef __cplusplus
}
#endif


#endif
