//
//  ak_utilities.c
//  ak.binaural~
//
//  Created by Admin on 30.11.17.
//

#include "vas_util.h"
#include "string.h"
#include <errno.h>
#include <limits.h>

#define GETLINE_MINSIZE 16

#if !defined(WHEREAMI_H)
#include <whereami.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
#if !defined(WAI_MALLOC) || !defined(WAI_FREE) || !defined(WAI_REALLOC)
#include <stdlib.h>
#endif
    
#if !defined(WAI_MALLOC)
#define WAI_MALLOC(size) malloc(size)
#endif
    
#if !defined(WAI_FREE)
#define WAI_FREE(p) free(p)
#endif
    
#if !defined(WAI_REALLOC)
#define WAI_REALLOC(p, size) realloc(p, size)
#endif
    
#ifndef WAI_NOINLINE
#if defined(_MSC_VER)
#define WAI_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define WAI_NOINLINE __attribute__((noinline))
#else
#error unsupported compiler
#endif
#endif
    
#if defined(_MSC_VER)
#define WAI_RETURN_ADDRESS() _ReturnAddress()
#elif defined(__GNUC__)
#define WAI_RETURN_ADDRESS() __builtin_extract_return_addr(__builtin_return_address(0))
#else
#error unsupported compiler
#endif
    
#if defined(_WIN32)
    
#define WIN32_LEAN_AND_MEAN
#if defined(_MSC_VER)
#pragma warning(push, 3)
#endif
#include <windows.h>
#include <intrin.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    
    static int WAI_PREFIX(getModulePath_)(HMODULE module, char* out, int capacity, int* dirname_length)
    {
        wchar_t buffer1[MAX_PATH];
        wchar_t buffer2[MAX_PATH];
        wchar_t* path = NULL;
        int length = -1;
        
        for (;;)
        {
            DWORD size;
            int length_, length__;
            
            size = GetModuleFileNameW(module, buffer1, sizeof(buffer1) / sizeof(buffer1[0]));
            
            if (size == 0)
                break;
            else if (size == (DWORD)(sizeof(buffer1) / sizeof(buffer1[0])))
            {
                DWORD size_ = size;
                do
                {
                    wchar_t* path_;
                    
                    path_ = (wchar_t*)WAI_REALLOC(path, sizeof(wchar_t) * size_ * 2);
                    if (!path_)
                        break;
                    size_ *= 2;
                    path = path_;
                    size = GetModuleFileNameW(module, path, size_);
                }
                while (size == size_);
                
                if (size == size_)
                    break;
            }
            else
                path = buffer1;
            
            if (!_wfullpath(buffer2, path, MAX_PATH))
                break;
            length_ = (int)wcslen(buffer2);
            length__ = WideCharToMultiByte(CP_UTF8, 0, buffer2, length_ , out, capacity, NULL, NULL);
            
            if (length__ == 0)
                length__ = WideCharToMultiByte(CP_UTF8, 0, buffer2, length_, NULL, 0, NULL, NULL);
            if (length__ == 0)
                break;
            
            if (length__ <= capacity && dirname_length)
            {
                int i;
                
                for (i = length__ - 1; i >= 0; --i)
                {
                    if (out[i] == '\\')
                    {
                        *dirname_length = i;
                        break;
                    }
                }
            }
            
            length = length__;
            
            break;
        }
        
        if (path != buffer1)
            WAI_FREE(path);
        
        return length;
    }
    
    WAI_NOINLINE WAI_FUNCSPEC
    int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
    {
        return WAI_PREFIX(getModulePath_)(NULL, out, capacity, dirname_length);
    }
    
    WAI_NOINLINE WAI_FUNCSPEC
    int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
    {
        HMODULE module;
        int length = -1;
        
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4054)
#endif
        if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)WAI_RETURN_ADDRESS(), &module))
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        {
            length = WAI_PREFIX(getModulePath_)(module, out, capacity, dirname_length);
        }
        
        return length;
    }
    
#elif defined(__linux__) || defined(__CYGWIN__) || defined(__sun)
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__linux__)
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
    
#if !defined(WAI_PROC_SELF_EXE)
#if defined(__sun)
#define WAI_PROC_SELF_EXE "/proc/self/path/a.out"
#else
#define WAI_PROC_SELF_EXE "/proc/self/exe"
#endif
#endif
    
    WAI_FUNCSPEC
    int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer[PATH_MAX];
        char* resolved = NULL;
        int length = -1;
        
        for (;;)
        {
            resolved = realpath(WAI_PROC_SELF_EXE, buffer);
            if (!resolved)
                break;
            
            length = (int)strlen(resolved);
            if (length <= capacity)
            {
                memcpy(out, resolved, length);
                
                if (dirname_length)
                {
                    int i;
                    
                    for (i = length - 1; i >= 0; --i)
                    {
                        if (out[i] == '/')
                        {
                            *dirname_length = i;
                            break;
                        }
                    }
                }
            }
            
            break;
        }
        
        return length;
    }
    
#if !defined(WAI_PROC_SELF_MAPS_RETRY)
#define WAI_PROC_SELF_MAPS_RETRY 5
#endif
    
#if !defined(WAI_PROC_SELF_MAPS)
#if defined(__sun)
#define WAI_PROC_SELF_MAPS "/proc/self/map"
#else
#define WAI_PROC_SELF_MAPS "/proc/self/maps"
#endif
#endif
    
#if defined(__ANDROID__) || defined(ANDROID)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif
    
    WAI_NOINLINE WAI_FUNCSPEC
    int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
    {
        int length = -1;
        FILE* maps = NULL;
        
        for (int r = 0; r < WAI_PROC_SELF_MAPS_RETRY; ++r)
        {
            maps = fopen(WAI_PROC_SELF_MAPS, "r");
            if (!maps)
                break;
            
            for (;;)
            {
                char buffer[PATH_MAX < 1024 ? 1024 : PATH_MAX];
                uint64_t low, high;
                char perms[5];
                uint64_t offset;
                uint32_t major, minor;
                char path[PATH_MAX];
                uint32_t inode;
                
                if (!fgets(buffer, sizeof(buffer), maps))
                    break;
                
                if (sscanf(buffer, "%" PRIx64 "-%" PRIx64 " %s %" PRIx64 " %x:%x %u %s\n", &low, &high, perms, &offset, &major, &minor, &inode, path) == 8)
                {
                    uint64_t addr = (uintptr_t)WAI_RETURN_ADDRESS();
                    if (low <= addr && addr <= high)
                    {
                        char* resolved;
                        
                        resolved = realpath(path, buffer);
                        if (!resolved)
                            break;
                        
                        length = (int)strlen(resolved);
#if defined(__ANDROID__) || defined(ANDROID)
                        if (length > 4
                            &&buffer[length - 1] == 'k'
                            &&buffer[length - 2] == 'p'
                            &&buffer[length - 3] == 'a'
                            &&buffer[length - 4] == '.')
                        {
                            int fd = open(path, O_RDONLY);
                            char* begin;
                            char* p;
                            
                            begin = (char*)mmap(0, offset, PROT_READ, MAP_SHARED, fd, 0);
                            p = begin + offset;
                            
                            while (p >= begin) // scan backwards
                            {
                                if (*((uint32_t*)p) == 0x04034b50UL) // local file header found
                                {
                                    uint16_t length_ = *((uint16_t*)(p + 26));
                                    
                                    if (length + 2 + length_ < (int)sizeof(buffer))
                                    {
                                        memcpy(&buffer[length], "!/", 2);
                                        memcpy(&buffer[length + 2], p + 30, length_);
                                        length += 2 + length_;
                                    }
                                    
                                    break;
                                }
                                
                                p -= 4;
                            }
                            
                            munmap(begin, offset);
                            close(fd);
                        }
#endif
                        if (length <= capacity)
                        {
                            memcpy(out, resolved, length);
                            
                            if (dirname_length)
                            {
                                int i;
                                
                                for (i = length - 1; i >= 0; --i)
                                {
                                    if (out[i] == '/')
                                    {
                                        *dirname_length = i;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        break;
                    }
                }
            }
            
            fclose(maps);
            maps = NULL;
            
            if (length != -1)
                break;
        }
        
        if (maps)
            fclose(maps);
        
        return length;
    }
    
#elif defined(__APPLE__)
    
#define _DARWIN_BETTER_REALPATH
#include <mach-o/dyld.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
    
    WAI_FUNCSPEC
    int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer1[PATH_MAX];
        char buffer2[PATH_MAX];
        char* path = buffer1;
        char* resolved = NULL;
        int length = -1;
        
        for (;;)
        {
            uint32_t size = (uint32_t)sizeof(buffer1);
            if (_NSGetExecutablePath(path, &size) == -1)
            {
                path = (char*)WAI_MALLOC(size);
                if (!_NSGetExecutablePath(path, &size))
                    break;
            }
            
            resolved = realpath(path, buffer2);
            if (!resolved)
                break;
            
            length = (int)strlen(resolved);
            if (length <= capacity)
            {
                memcpy(out, resolved, length);
                
                if (dirname_length)
                {
                    int i;
                    
                    for (i = length - 1; i >= 0; --i)
                    {
                        if (out[i] == '/')
                        {
                            *dirname_length = i;
                            break;
                        }
                    }
                }
            }
            
            break;
        }
        
        if (path != buffer1)
            WAI_FREE(path);
        
        return length;
    }
    
    WAI_NOINLINE WAI_FUNCSPEC
    int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer[PATH_MAX];
        char* resolved = NULL;
        int length = -1;
        
        for(;;)
        {
            Dl_info info;
            
            if (dladdr(WAI_RETURN_ADDRESS(), &info))
            {
                resolved = realpath(info.dli_fname, buffer);
                if (!resolved)
                    break;
                
                length = (int)strlen(resolved);
                if (length <= capacity)
                {
                    memcpy(out, resolved, length);
                    
                    if (dirname_length)
                    {
                        int i;
                        
                        for (i = length - 1; i >= 0; --i)
                        {
                            if (out[i] == '/')
                            {
                                *dirname_length = i;
                                break;
                            }
                        }
                    }
                }
            }
            
            break;
        }
        
        return length;
    }
    
#elif defined(__QNXNTO__)
    
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
    
#if !defined(WAI_PROC_SELF_EXE)
#define WAI_PROC_SELF_EXE "/proc/self/exefile"
#endif
    
    WAI_FUNCSPEC
    int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer1[PATH_MAX];
        char buffer2[PATH_MAX];
        char* resolved = NULL;
        FILE* self_exe = NULL;
        int length = -1;
        
        for (;;)
        {
            self_exe = fopen(WAI_PROC_SELF_EXE, "r");
            if (!self_exe)
                break;
            
            if (!fgets(buffer1, sizeof(buffer1), self_exe))
                break;
            
            resolved = realpath(buffer1, buffer2);
            if (!resolved)
                break;
            
            length = (int)strlen(resolved);
            if (length <= capacity)
            {
                memcpy(out, resolved, length);
                
                if (dirname_length)
                {
                    int i;
                    
                    for (i = length - 1; i >= 0; --i)
                    {
                        if (out[i] == '/')
                        {
                            *dirname_length = i;
                            break;
                        }
                    }
                }
            }
            
            break;
        }
        
        fclose(self_exe);
        
        return length;
    }
    
    WAI_FUNCSPEC
    int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer[PATH_MAX];
        char* resolved = NULL;
        int length = -1;
        
        for(;;)
        {
            Dl_info info;
            
            if (dladdr(WAI_RETURN_ADDRESS(), &info))
            {
                resolved = realpath(info.dli_fname, buffer);
                if (!resolved)
                    break;
                
                length = (int)strlen(resolved);
                if (length <= capacity)
                {
                    memcpy(out, resolved, length);
                    
                    if (dirname_length)
                    {
                        int i;
                        
                        for (i = length - 1; i >= 0; --i)
                        {
                            if (out[i] == '/')
                            {
                                *dirname_length = i;
                                break;
                            }
                        }
                    }
                }
            }
            
            break;
        }
        
        return length;
    }
    
#elif defined(__DragonFly__) || defined(__FreeBSD__) || \
defined(__FreeBSD_kernel__) || defined(__NetBSD__)
    
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <dlfcn.h>
    
    WAI_FUNCSPEC
    int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer1[PATH_MAX];
        char buffer2[PATH_MAX];
        char* path = buffer1;
        char* resolved = NULL;
        int length = -1;
        
        for (;;)
        {
            int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
            size_t size = sizeof(buffer1);
            
            if (sysctl(mib, (u_int)(sizeof(mib) / sizeof(mib[0])), path, &size, NULL, 0) != 0)
                break;
            
            resolved = realpath(path, buffer2);
            if (!resolved)
                break;
            
            length = (int)strlen(resolved);
            if (length <= capacity)
            {
                memcpy(out, resolved, length);
                
                if (dirname_length)
                {
                    int i;
                    
                    for (i = length - 1; i >= 0; --i)
                    {
                        if (out[i] == '/')
                        {
                            *dirname_length = i;
                            break;
                        }
                    }
                }
            }
            
            break;
        }
        
        if (path != buffer1)
            WAI_FREE(path);
        
        return length;
    }
    
    WAI_NOINLINE WAI_FUNCSPEC
    int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length)
    {
        char buffer[PATH_MAX];
        char* resolved = NULL;
        int length = -1;
        
        for(;;)
        {
            Dl_info info;
            
            if (dladdr(WAI_RETURN_ADDRESS(), &info))
            {
                resolved = realpath(info.dli_fname, buffer);
                if (!resolved)
                    break;
                
                length = (int)strlen(resolved);
                if (length <= capacity)
                {
                    memcpy(out, resolved, length);
                    
                    if (dirname_length)
                    {
                        int i;
                        
                        for (i = length - 1; i >= 0; --i)
                        {
                            if (out[i] == '/')
                            {
                                *dirname_length = i;
                                break;
                            }
                        }
                    }
                }
            }
            
            break;
        }
        
        return length;
    }
    
#else
    
#error unsupported platform
    
#endif
    
#ifdef __cplusplus
}
#endif

int vas_getline(char **lineptr, size_t *n, FILE *fp) {
    int ch;
    int i = 0;
    char free_on_err = 0;
    char *p;
    
    errno = 0;
    if (lineptr == NULL || n == NULL || fp == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (*lineptr == NULL) {
        *n = GETLINE_MINSIZE;
        *lineptr = (char *)malloc( sizeof(char) * (*n));
        if (*lineptr == NULL) {
            errno = ENOMEM;
            return -1;
        }
        free_on_err = 1;
    }
    
    for (i=0; ; i++) {
        ch = fgetc(fp);
        while (i >= (*n) - 2) {
            *n *= 2;
            p = realloc(*lineptr, sizeof(char) * (*n));
            if (p == NULL) {
                if (free_on_err)
                    free(*lineptr);
                errno = ENOMEM;
                return -1;
            }
            *lineptr = p;
        }
        if (ch == EOF) {
            if (i == 0) {
                if (free_on_err)
                    free(*lineptr);
                return -1;
            }
            (*lineptr)[i] = '\0';
            *n = i;
            return i;
        }
        
        if (ch == '\n') {
            (*lineptr)[i] = '\n';
            (*lineptr)[i+1] = '\0';
            *n = i+1;
            return i+1;
        }
        (*lineptr)[i] = (char)ch;
    }
}

char* vas_strsep(char** stringp, const char* delim)
{
    char* start = *stringp;
    char* p;
    
    p = (start != NULL) ? strpbrk(start, delim) : NULL;
    
    if (p == NULL)
    {
        *stringp = NULL;
    }
    else
    {
        *p = '\0';
        *stringp = p + 1;
    }
    
    return start;
}



void vas_util_fadd(float *input1, float *input2, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    vDSP_vadd(input1, 1, input2, 1, dest, 1, length);
#endif
    
#ifdef VAS_USE_KISSFFT
#ifdef VAS_USE_AVX
    
    int n = length;
    while(n)
    {
        __m256 buffer1 = _mm256_load_ps(input1);
        __m256 buffer2 = _mm256_load_ps(input2);
        __m256 result;
        
        result = _mm256_add_ps(buffer1, buffer2);
        _mm256_store_ps(dest, result);
        n-=8;
        input1+=8;
        input2+=8;
        dest+=8;
    }
     
#else
    int n = length;
    while (n--)
    {
        *dest++ = *input1++ + *input2++;
    }
#endif
#endif
}

void vas_util_fmultiply(float *input1, float *input2, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    vDSP_vmul(input1, 1, input2, 1, dest, 1, length);
#else
    int n = length;
    while (n--)
    {
        *dest++ = *input1++ * *input2++;
    }
#endif
}

void vas_util_fcopy(float *source, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    cblas_scopy(length, source, 1, dest, 1);
#endif
    
#ifdef VAS_USE_KISSFFT
#ifdef VAS_USE_AVX
    
    int n = length;
    while(n)
    {
        __m256 buffer = _mm256_load_ps(source);
        _mm256_store_ps(dest, buffer);
        n-=8;
        source+=8;
        dest+=8;
    }

#else
    int n = length;
    while (n--)
    {
        *dest++ = *source++;
    }

#endif
#endif
}

void vas_util_fcopy_noavx(float *source, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    cblas_scopy(length, source, 1, dest, 1);
#endif
    
#ifdef VAS_USE_KISSFFT
    int n = length;
    while (n--)
    {
        *dest++ = *source++;
    }

#endif
}

void vas_util_fscale(float *dest, float scale,  int length)
{
#ifdef VAS_USE_VDSP
    vDSP_vsmul(dest, 1, &scale, dest, 1, length);
#endif
    
#ifdef VAS_USE_KISSFFT
    int n = length;
    while (n--)
    {
        float tmp = *dest; // prevents gcc generating segmentation fault with avx optimization
        tmp *= scale;
        *dest++ = tmp;
    }
#endif
    
}

void vas_util_fmulitplyScalar(float *source, float scale, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    vDSP_vsmul(source, 1, &scale, dest, 1, length);
#endif
    
#ifdef VAS_USE_KISSFFT
    int n = length;
    while (n--)
    {
        float tmp = *source; // prevents gcc generating segmentation fault with avx optimization
        tmp *= scale;
        *dest++ = tmp;
    }
#endif
    
}

void vas_util_fgenerateUnitImpulse(float *dest, int length)
{
    
    
}

void vas_util_complexCopy(VAS_COMPLEX *source, VAS_COMPLEX *dest, int length)
{
    int n = length;
#ifdef VAS_USE_VDSP
    float *destReal = dest->realp;
    float *destImag = dest->imagp;
    float *sourceReal = source->realp;
    float *sourceImag = source->imagp;
    
    while (n--)
    {
        *destReal++ = *sourceReal++;
        *destImag++ = *sourceImag++;
    }
#endif
    
#ifdef VAS_USE_KISSFFT
    while (n--)
    {
        dest->r = source->r;
        dest->i = source->i;
        dest++;source++;
    }
#endif
}

void vas_util_complexScale(VAS_COMPLEX *dest, float scale,  int length)
{
    int n = length;
#ifdef VAS_USE_VDSP
    float *destReal = dest->realp;
    float *destImag = dest->imagp;
    while (n--)
    {
        *destReal = (*destReal) * scale;
        *destImag = (*destImag) * scale;
        destReal++;destImag++;
    }
#endif
    
#ifdef VAS_USE_KISSFFT
    while (n--)
    {
        dest->r *= scale;
        dest->i *= scale;
        dest++;
    }
#endif
}

void vas_util_deinterleaveComplexArray2(VAS_COMPLEX *input, float *realArray, float *imagArray, int length)
{
    float *real = realArray;
    float *imag = imagArray;
    VAS_COMPLEX *inputPtr = input;
    int imagIndex = length * 0.5;
    
#ifdef VAS_USE_KISSFFT
#ifdef VAS_USE_AVX
    __m256 ab0145;
    __m256 ab2367;
    __m256 destReal;
    __m256 destImag;

    int n = length;
    double * p;

    while (n)
    {
        p = (double*)inputPtr;
        ab0145 = _mm256_castpd_ps(_mm256_setr_pd(p[0], p[1], p[4], p[5]));
        ab2367 = _mm256_castpd_ps(_mm256_setr_pd(p[2], p[3], p[6], p[7]));
        destReal = _mm256_shuffle_ps(ab0145, ab2367, 0x88);
        destImag = _mm256_shuffle_ps(ab0145, ab2367, 0xDD);
        _mm256_store_ps(real, destReal);
        _mm256_store_ps(imag, destImag);
        inputPtr+=8;
        real+=8;
        imag+=8;
        n-=8;
    }

    n = length;
    real = realArray;
    imag = imagArray;
    inputPtr = input;
    float *writeBackReal = (float *)input;
    float *writeBackImag = (float *)(&(input[imagIndex]));

    while (n)
    {
        destReal = _mm256_load_ps(real);
        destImag = _mm256_load_ps(imag);
        _mm256_store_ps(writeBackReal, destReal);
        _mm256_store_ps(writeBackImag, destImag);
        n-=8;
        real+=8;
        imag+=8;
        writeBackReal+=8;
        writeBackImag+=8;
    }
#else
    int n = length;
    while (n--)
    {
        *real++  = inputPtr->r;
        *imag++ = inputPtr->i;
        inputPtr++;
    }
    
    n = length;
    real = realArray;
    imag = imagArray;
    inputPtr = input;

    float *writeBackReal = (float *)input;
    float *writeBackImag = (float *)(&(input[imagIndex]));
    
    while (n--)
    {
        *writeBackReal++ = *real++;
        *writeBackImag++ = *imag++;
    }
#endif
#endif
}

void vas_util_complexMultiplyAdd2(VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest, int length)                //complex-multiplication
{
#ifdef VAS_USE_VDSP
    float preserveIRNyq = filter->imagp[0];
    filter->imagp[0] = 0;
    float preserveSigNyq = signalIn->imagp[0];
    signalIn->imagp[0] = 0;
    float preservePrevResult = dest->imagp[0];
    dest->imagp[0] = 0;
    
    vDSP_zvma(signalIn, 1, filter, 1, dest, 1, dest, 1, length);
    dest->imagp[0] = preserveIRNyq * preserveSigNyq + preservePrevResult;
    filter->imagp[0] = preserveIRNyq;
    signalIn->imagp[0] = preserveSigNyq;
#endif
    
#ifdef VAS_USE_KISSFFT
#ifdef VAS_USE_AVX
    
        double * p;
        int n = length;
    
        while (n > 0)
        {
            p = (double*)dest;
            
            __m256 ab0145 = _mm256_castpd_ps(_mm256_setr_pd(p[0], p[1], p[4], p[5]));
            __m256 ab2367 = _mm256_castpd_ps(_mm256_setr_pd(p[2], p[3], p[6], p[7]));
            __m256 destReal = _mm256_shuffle_ps(ab0145, ab2367, 0x88);
            __m256 destImag = _mm256_shuffle_ps(ab0145, ab2367, 0xDD);
           
            p = (double*)signalIn;

            ab0145 = _mm256_castpd_ps(_mm256_setr_pd(p[0], p[1], p[4], p[5]));
            ab2367 = _mm256_castpd_ps(_mm256_setr_pd(p[2], p[3], p[6], p[7]));
            __m256 signalInReal = _mm256_shuffle_ps(ab0145, ab2367, 0x88);
            __m256 signalInImag = _mm256_shuffle_ps(ab0145, ab2367, 0xDD);
            
            p = (double*)filter;
            
            ab0145 = _mm256_castpd_ps(_mm256_setr_pd(p[0], p[1], p[4], p[5]));
            ab2367 = _mm256_castpd_ps(_mm256_setr_pd(p[2], p[3], p[6], p[7]));
            __m256 filterReal = _mm256_shuffle_ps(ab0145, ab2367, 0x88);
            __m256 filterImag = _mm256_shuffle_ps(ab0145, ab2367, 0xDD);
            
              
            //pDstR[i] = fmsub(fSrc1R, fSrc2R, fmadd(fSrc1I, fSrc2I, -pDstR[i]));
            //pDstI[i] = fmadd(fSrc1R, fSrc2I, fmadd(fSrc2R, fSrc1I, pDstI[i]));
            destReal = _mm256_add_ps(destReal, _mm256_sub_ps(_mm256_mul_ps(signalInReal, filterReal), _mm256_mul_ps(signalInImag, filterImag)));
            destImag = _mm256_add_ps(destImag, _mm256_add_ps(_mm256_mul_ps(signalInReal, filterImag), _mm256_mul_ps(signalInImag, filterReal)));
            
            //destImag = _mm256_fmadd_ps(signalInReal, filterImag, _mm256_fmadd_ps(filterReal, signalInImag, destImag)); //AVX2

          /*  float* resReal = (float*)&destReal;
            float* resImag = (float*)&destImag;
        
            (dest)->r = resReal[0]; (dest+1)->r = resReal[1]; (dest+2)->r = resReal[2]; (dest+3)->r = resReal[3];
            (dest+4)->r = resReal[4]; (dest+5)->r = resReal[5]; (dest+6)->r = resReal[6]; (dest+7)->r = resReal[7];
            
            (dest)->i = resImag[0]; (dest+1)->i = resImag[1]; (dest+2)->i = resImag[2]; (dest+3)->i = resImag[3];
            (dest+4)->i = resImag[4]; (dest+5)->i = resImag[5]; (dest+6)->i = resImag[6]; (dest+7)->i = resImag[7];*/
            

            ab0145 = _mm256_unpacklo_ps(destReal, destImag);
            ab2367 = _mm256_unpackhi_ps(destReal, destImag);
            __m256 destVec = _mm256_permute2f128_ps(ab0145,ab2367,0x20);
            _mm256_store_ps((float *)dest, destVec);
            destVec = _mm256_permute2f128_ps(ab0145,ab2367,0x31) ;
            _mm256_store_ps((float *)(dest+4), destVec);
    
            n-=8;
            signalIn+=8;
            filter+=8;
            dest+=8;

        }
    
        signalIn-=7;
        filter-=7;
        dest-=7;
    
        dest->r += filter->r * signalIn->r - filter->i * signalIn->i;
        dest->i += filter->r * signalIn->i + filter->i * signalIn->r;

    
#else
        int n = length+1;
        while (n--)
        {
            dest->r += filter->r * signalIn->r - filter->i * signalIn->i;
            dest->i += filter->r * signalIn->i + filter->i * signalIn->r;
            dest++;filter++;signalIn++;
        }
#endif
    
#endif
}

void vas_util_complexMultiplyAdd(VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest, int length)                //complex-multiplication
{
#ifdef VAS_USE_VDSP
    float preserveIRNyq = filter->imagp[0];
    filter->imagp[0] = 0;
    float preserveSigNyq = signalIn->imagp[0];
    signalIn->imagp[0] = 0;
    float preservePrevResult = dest->imagp[0];
    dest->imagp[0] = 0;
    
    vDSP_zvma(signalIn, 1, filter, 1, dest, 1, dest, 1, length);
    dest->imagp[0] = preserveIRNyq * preserveSigNyq + preservePrevResult;
    filter->imagp[0] = preserveIRNyq;
    signalIn->imagp[0] = preserveSigNyq;
#endif
    
#ifdef VAS_USE_KISSFFT
#ifdef VAS_USE_AVX
    
        double * p;
   
        int n = length;
        int imagIndex = floor(length/2.);
        float *filterRealFloat = (float *)filter;
        float *filterImagFloat = (float *)(&(filter[imagIndex]));
        float *signalInRealFloat = (float *)signalIn;
        float *signalInImagFloat = (float *)(&(signalIn[imagIndex]));
    
        while (n > 0)
        {
            p = (double*)dest;
            
            __m256 ab0145 = _mm256_castpd_ps(_mm256_setr_pd(p[0], p[1], p[4], p[5]));
            __m256 ab2367 = _mm256_castpd_ps(_mm256_setr_pd(p[2], p[3], p[6], p[7]));
            __m256 destReal = _mm256_shuffle_ps(ab0145, ab2367, 0x88);
            __m256 destImag = _mm256_shuffle_ps(ab0145, ab2367, 0xDD);
              
            __m256 signalInReal = _mm256_load_ps(signalInRealFloat);
            __m256 signalInImag = _mm256_load_ps(signalInImagFloat);
             
            __m256 filterReal = _mm256_load_ps(filterRealFloat);
            __m256 filterImag = _mm256_load_ps(filterImagFloat);
              
            //pDstR[i] = fmsub(fSrc1R, fSrc2R, fmadd(fSrc1I, fSrc2I, -pDstR[i]));
            //pDstI[i] = fmadd(fSrc1R, fSrc2I, fmadd(fSrc2R, fSrc1I, pDstI[i]));
            
            destReal = _mm256_add_ps(destReal, _mm256_sub_ps(_mm256_mul_ps(signalInReal, filterReal), _mm256_mul_ps(signalInImag, filterImag)));
            destImag = _mm256_add_ps(destImag, _mm256_add_ps(_mm256_mul_ps(signalInReal, filterImag), _mm256_mul_ps(signalInImag, filterReal)));
           // destReal = _mm256_fmadd_ps(signalInReal, filterReal, _mm256_fmadd_ps(signalInImag, filterImag, -destReal)); //AVX2
           // destImag = _mm256_fmadd_ps(signalInReal, filterImag, _mm256_fmadd_ps(filterReal, signalInImag, destImag)); //AVX2
     
            ab0145 = _mm256_unpacklo_ps(destReal, destImag);
            ab2367 = _mm256_unpackhi_ps(destReal, destImag);
            __m256 destVec = _mm256_permute2f128_ps(ab0145,ab2367,0x20);
            _mm256_store_ps((float *)dest, destVec);
            destVec = _mm256_permute2f128_ps(ab0145,ab2367,0x31) ;
            _mm256_store_ps((float *)(dest+4), destVec);
    
            n-=8;
            dest+=8;
            filterRealFloat+=8;
            filterImagFloat+=8;
            signalInRealFloat+=8;
            signalInImagFloat+=8;
        }
    
#else
        int n = length;
        while (n--)
        {
            dest->r += filter->r * signalIn->r - filter->i * signalIn->i;
            dest->i += filter->r * signalIn->i + filter->i * signalIn->r;
            dest++;filter++;signalIn++;
        }
#endif
#endif
}

void vas_util_complexMultiply(int length, VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest)
{
#ifdef VAS_USE_VDSP
    float preserveIRNyq = filter->imagp[0];
    filter->imagp[0] = 0;
    float preserveSigNyq = signalIn->imagp[0];
    signalIn->imagp[0] = 0;
    
    vDSP_zvmul(signalIn, 1, filter, 1, dest, 1, length, 1);
    dest->imagp[0] = 0;//preserveIRNyq * preserveSigNyq;
    filter->imagp[0] = preserveIRNyq;
    signalIn->imagp[0] = preserveSigNyq;
#endif
    
#ifdef VAS_USE_KISSFFT
    int n = length;
    while (n--)
    {
        dest->r = filter->r * signalIn->r - filter->i * signalIn->i;
        dest->i = filter->r * signalIn->i + filter->i * signalIn->r;
        dest++;filter++;signalIn++;
    }
#endif
}

void vas_util_complexWriteZeros(VAS_COMPLEX *dest, int length)
{
    int n = length;
#ifdef VAS_USE_VDSP
    float *destReal = dest->realp;
    float *destImag = dest->imagp;
    while (n--)
    {
        *destReal = 0;
        *destImag = 0;
        destReal++;destImag++;
    }
#endif
    
#ifdef VAS_USE_KISSFFT
#ifdef VAS_USE_AVX
    n = length *2;
    float *floatPtr = (float *)dest;
    while(n)
    {
        __m256 zero = _mm256_setzero_ps();
        _mm256_store_ps(floatPtr, zero);
        floatPtr+=8;
        n-=8;
    }
    
#else
    
    while (n--)
    {
        dest->r = 0;
        dest->i = 0;
        dest++;
    }
#endif
#endif
}

float vas_utilities_degrees2radians(float degrees)
{
    return degrees * (M_PI/180);
}

float vas_utilities_radians2degrees(float radians)
{
    return radians * (180/M_PI);
}

const char *vas_util_getFileExtension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";
    return dot + 1;
}

bool vas_utilities_isValidSegmentSize(int segmentSize)
{
    return  (segmentSize >= 16) && (segmentSize <= 65536)  && ((segmentSize & (segmentSize - 1)) == 0);
}

int vas_utilities_roundUp2NextPowerOf2(int value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}



/*void ak_vaTools_zmultiply_SSE(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest)                //complex-multiplication
{
    vFloat *destReal = (vFloat *)dest.realp;
    vFloat *destImag = (vFloat *)dest.imagp;
    vFloat *signalInReal = (vFloat *)signalIn.realp;
    vFloat *signalInImag = (vFloat *)signalIn.imagp;
    vFloat *filterReal = (vFloat *)filter.realp;
    vFloat *filterImag = (vFloat *)filter.imagp;
    int n = length;
    
    while (n--)
    {
        *destReal = _mm_sub_ps(_mm_mul_ps(*signalInReal, *filterReal), _mm_mul_ps(*signalInImag, *filterImag));
        *destImag = _mm_add_ps(_mm_mul_ps(*signalInReal, *filterImag), _mm_mul_ps(*signalInImag, *filterReal));
        signalInReal++; signalInImag++;filterReal++; filterImag++; destReal++; destImag++;
    }
}
void ak_vaTools_zmultiplyAdd_SSE(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest)                //complex-multiplication
{
    vFloat *destReal = (vFloat *)dest.realp;
    vFloat *destImag = (vFloat *)dest.imagp;
    vFloat *signalInReal = (vFloat *)signalIn.realp;
    vFloat *signalInImag = (vFloat *)signalIn.imagp;
    vFloat *filterReal = (vFloat *)filter.realp;
    vFloat *filterImag = (vFloat *)filter.imagp;
    int n = length;
    
    while (n--)
    {
        *destReal = _mm_add_ps(*destReal, _mm_sub_ps(_mm_mul_ps(*signalInReal, *filterReal), _mm_mul_ps(*signalInImag, *filterImag)));
        *destImag = _mm_add_ps(*destImag, _mm_add_ps(_mm_mul_ps(*signalInReal, *filterImag), _mm_mul_ps(*signalInImag, *filterReal)));
        signalInReal++; signalInImag++;filterReal++; filterImag++; destReal++; destImag++;
    }
}
 
void ak_vaTools_fadd_SSE(int n, float *signalIn, float *filter, float *dest)
{
    
    vFloat *in1 = (vFloat *) signalIn;
    vFloat *in2 = (vFloat *) filter;
    vFloat *out = (vFloat *) dest;
    
    while (n--)
        *out++ = _mm_add_ps(*in1++, *in2++);
}
void vas_utilities_fcopy_SSE(int n, float *source,  float *dest)
{
    
    vFloat *in = (vFloat *) source;
    vFloat *out = (vFloat *) dest;
    while (n--)
        *out++ = *in++;
}*/

void vas_utilities_writeZeros1(int length, VAS_INPUTBUFFER *dest)
{
    int n = length;
    while(n--)
        *dest++ = 0;
}

void vas_utilities_writeZeros(int length, float *dest)
{
    int n = length;
    while(n--)
        *dest++ = 0;
}

float vas_utilities_fadeOut(float n, float length)
{
    float k = 1 -(n/length);
    float volume = (0.5  + 0.5 * cos(M_PI * k));
    return volume;
}

float vas_utilities_fadeIn(float n, float length)
{
    float k = 1 -(n/length);
    float volume = (0.5  - 0.5 * cos(M_PI * k));
    return volume;
}

void vas_utilities_writeFadeOutArray(float length, float *dest)
{
    float n = length;
    float *destPtr = dest;
    while(n--)
    {
        float k = 1-n/length;
        float volume = (0.5  + 0.5 * cos(M_PI * k));
        *destPtr++ = volume;        
    }
}

void vas_utilities_writeFadeInArray(float length, float *dest)
{
    float n = length;
    float *destPtr = dest;
    while(n--)
    {
        float k = 1-n/length;
        float volume = (0.5  - 0.5 * cos(M_PI * k));
        *destPtr++ = volume;
    }
}

void vas_utilities_writeFadeOutArray1(float length, float *dest)
{
    float n = length;
    float *destPtr = dest;
    while(n--)
    {
        float k = 1-n/length;
        float volume = cos(M_PI*0.5 * k);
        *destPtr++ = volume;
    }
}

void vas_utilities_writeFadeInArray1(float length, float *dest)
{
    float n = length;
    float *destPtr = dest;
    while(n--)
    {
        float k = 1-n/length;
        float volume = sin(M_PI*0.5 * k);
        *destPtr++ = volume;
    }
}

void vas_utilities_copyFloatArray(int length, float *arr1, float *arr2)
{
    int n = length;
    float *in = arr1;
    float *out = arr2;
    while (n--)
        *out++ = *in++;
}

#ifdef VAS_USE_VDSP

void vas_utilities_writeZerosComplex_vDSP(int n, COMPLEX_SPLIT dest)
{
    float *destReal = dest.realp;
    float *destImag = dest.imagp;
    while (n--)
    {
        *destReal = 0;
        *destImag = 0;
        destReal++;destImag++;
    }
}

void vas_utilities_scaleComplex_vDSP(int n, float scale, COMPLEX_SPLIT dest)
{
    float *destReal = dest.realp;
    float *destImag = dest.imagp;
    while (n--)
    {
        *destReal = (*destReal) * scale;
        *destImag = (*destImag) * scale;
        destReal++;destImag++;
    }
}

void vas_utilities_fcopy_vDSP(int length, float *source, float *dest)
{
    cblas_scopy(length, source, 1, dest, 1);
}

void vas_utilities_copy_vDSP(int length, double *source, double *dest)
{
    cblas_dcopy(length, source, 1, dest, 1);
}

void vas_utilities_zcopy_vDSP(int length, COMPLEX_SPLIT source, COMPLEX_SPLIT dest)
{
    float *destReal = dest.realp;
    float *destImag = dest.imagp;
    float *sourceReal = source.realp;
    float *sourceImag = source.imagp;
    
    int n = length;
    while (n--)
    {
        *destReal++ = *sourceReal++;
        *destImag++ = *sourceImag++;
    }
}

void vas_utilities_fadd_vDSP(int length, float *signalIn, float *filter, float *dest)
{
    vDSP_vadd(signalIn, 1, filter, 1, dest, 1, length);
}

void vas_utilities_fmultiply_vDSP(int length, float *signalIn, float *filter, float *dest)
{
    vDSP_vmul(signalIn, 1, filter, 1, dest, 1, length);
}

void vas_utilities_fscale_vDSP(int length, float *signal, float scale)
{
    vDSP_vsmul(signal, 1, &scale, signal, 1, length);
}

void vas_utilities_scale_vDSP(int length, double *signal, double scale)
{
    vDSP_vsmulD(signal, 1, &scale, signal, 1, length);
}

void vas_utilities_zadd_vDSP(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest)
{
    vDSP_zvadd(&signalIn, 1, &filter, 1, &dest, 1, length);
}

void vas_utilities_zmultiply_vDSP(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest)
{
    float preserveIRNyq = filter.imagp[0];
    filter.imagp[0] = 0;
    float preserveSigNyq = signalIn.imagp[0];
    signalIn.imagp[0] = 0;
    
    vDSP_zvmul(&signalIn, 1, &filter, 1, &dest, 1, length, 1);
    dest.imagp[0] = 0;//preserveIRNyq * preserveSigNyq;
    filter.imagp[0] = preserveIRNyq;
    signalIn.imagp[0] = preserveSigNyq;
}

void vas_utilities_zmultiplyAdd_vDSP(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest)
{
    float preserveIRNyq = filter.imagp[0];
    filter.imagp[0] = 0;
    float preserveSigNyq = signalIn.imagp[0];
    signalIn.imagp[0] = 0;
    float preservePrevResult = dest.imagp[0];
    dest.imagp[0] = 0;
    
    vDSP_zvma(&signalIn, 1, &filter, 1, &dest, 1, &dest, 1, length);
    dest.imagp[0] = preserveIRNyq * preserveSigNyq + preservePrevResult;
    filter.imagp[0] = preserveIRNyq;
    signalIn.imagp[0] = preserveSigNyq;
}

#endif
