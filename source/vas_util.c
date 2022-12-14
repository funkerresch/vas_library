//
//  ak_utilities.c
//  ak.binaural~
//
//  Created by Thomas Resch on 30.11.17.
//

#if defined(MAXMSPSDK) || defined(PUREDATA)
#include "m_pd.h"
#endif

#include "vas_util.h"
#include "string.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#ifdef __APPLE__
#include <sys/sysctl.h> //TODO: this is not cross-compatible
#endif

#if defined(__ANDROID__) || defined(ANDROID)
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#define GETLINE_MINSIZE 16

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

void vas_util_debug(char *fmt, ...)
{
    char dest[1024 * 16];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf(dest, fmt, argptr);
    va_end(argptr);
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post(dest);
#elif defined(IAGS_UNITY_SPATIALIZER)
    Debug(dest);
#else
    printf(dest);
#endif
}

float vas_util_faverageSumOfmagnitudes(float *in, int length)
{
    float result = 0;
#ifdef VAS_USE_VDSP
    vDSP_svemg(in, 1, &result, length);
#else
    int n = length;
    while(n--)
        result+=fabsf(*in++);
#endif
    result = result/length;
    return result;
}

float vas_util_fgain2db(float in)
{
    return -20.0f * log10f(in);
}


float vas_util_dB_to_lin(float dB)
{
    return powf(10, dB/20.0);
}

void vas_util_single2DoublePrecision(float *in, double *out, int length)
{
    #ifdef VAS_USE_VDSP
    vDSP_vspdp(in, 1, out, 1, length);
    #else
    int n = length;
    while(n--)
    {
        *out++ = *in++;
    }
    #endif
}

void vas_util_double2SinglePrecision(double *in, float *out, int length)
{
    #ifdef VAS_USE_VDSP
        vDSP_vdpsp(in, 1, out, 1, length);
    #else
    int n = length;
    while(n--)
    {
        *out++ = *in++;
    }
    #endif
}

void vas_util_fadd(float *input1, float *input2,  float *dest, int length)
{
#ifdef VAS_USE_VDSP
    vDSP_vadd(input1, 1, input2, 1, dest, 1, length);
#endif
    
#if defined(VAS_USE_PFFFT)

#ifdef PFFFT_ENABLE_NEON
    int n = length;
    while(n)
    {
        float32x4_t v1 = vld1q_f32(input1);
        float32x4_t v2 = vld1q_f32(input2);
        float32x4_t res = vaddq_f32(v1, v2);
        vst1q_f32(dest, res);
        n-=4;
        input1+=4;
        input2+=4;
        dest+=4;
    }
#elif defined (VAS_USE_AVX)

    int n = length;
    __m256 buffer1;
    __m256 buffer2;
    __m256 result;
    
    while(n)
    {
        buffer1 = _mm256_load_ps(input1);
        buffer2 = _mm256_load_ps(input2);
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
#endif
#if defined(VAS_USE_PFFFT)
    
#ifdef PFFFT_ENABLE_NEON
    int n = length;
    while(n)
    {
        float32x4_t v1 = vld1q_f32(input1);
        float32x4_t v2 = vld1q_f32(input2);
        float32x4_t prod = vmulq_f32(v1, v2);
        vst1q_f32(dest, prod);
        n-=4;
        input1+=4;
        input2+=4;
        dest+=4;
    }
#elif defined(VAS_USE_AVX)
    int n = length;
    __m256 buffer1;
    __m256 buffer2;
    __m256 result;
    
    while(n)
    {
        buffer1 = _mm256_load_ps(input1);
        buffer2 = _mm256_load_ps(input2);
        result = _mm256_mul_ps(buffer1, buffer2);
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
        *dest++ = *input1++ * *input2++;
    }
#endif
#endif
}


void vas_util_fcopyUnalignedSource(float *source, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    cblas_scopy(length, source, 1, dest, 1);
#endif
    
#if defined(VAS_USE_PFFFT)
#ifdef VAS_USE_AVX

    int n = length;

    while(n)
    {
        _mm256_store_ps(dest, _mm256_loadu_ps(source));
        n-=8;
        source+=8;
        dest+=8;
    }
#elif defined(PFFFT_ENABLE_NEON)

    int n = length;
    while(n)
    {
        float32x4_t v1 = vld1q_f32(source);
        vst1q_f32(dest, v1);
        n-=4;
        source+=4;
        dest+=4;
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

void vas_util_fcopy(float *source, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    cblas_scopy(length, source, 1, dest, 1);
#endif
    
#if defined(VAS_USE_PFFFT)
#ifdef VAS_USE_AVX

    int n = length;

    while(n)
    {
        _mm256_store_ps(dest, _mm256_load_ps(source));
        n-=8;
        source+=8;
        dest+=8;
    }
#elif defined(PFFFT_ENABLE_NEON)

    int n = length;
    while(n)
    {
        float32x4_t v1 = vld1q_f32(source);
        vst1q_f32(dest, v1);
        n-=4;
        source+=4;
        dest+=4;
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
#else
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
    
#if defined(VAS_USE_PFFFT)
#ifdef PFFFT_ENABLE_NEON
    int n = length;
    while(n)
    {
        float32x4_t v1 = vld1q_f32(dest);
        v1 = vmulq_n_f32(v1, scale);
        vst1q_f32(dest, v1);
        n-=4;
        dest+=4;
    }
#elif defined(VAS_USE_AVX)
    int n = length;
    __m256 buffer;
    const __m256 scalar = _mm256_set1_ps(scale);
    __m256 result;
    
    while(n)
    {
        buffer = _mm256_load_ps(dest);
        result = _mm256_mul_ps(buffer, scalar);
        _mm256_store_ps(dest, result);
        n-=8;
        dest+=8;
    }
#else
    int n = length;
    while (n--)
    {
        float tmp = *dest; // prevents gcc generating segmentation fault with avx optimization
        tmp *= scale;
        *dest++ = tmp;
    }
#endif
#endif
    
}

void vas_util_fmulitplyScalar(float *source, float scale, float *dest, int length)
{
#ifdef VAS_USE_VDSP
    vDSP_vsmul(source, 1, &scale, dest, 1, length);
#endif
    
#if defined(VAS_USE_PFFFT)
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
    
#if defined(VAS_USE_KISSFFT)|| defined(VAS_USE_PFFFT)
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
    
#if defined(VAS_USE_KISSFFT)|| defined(VAS_USE_PFFFT)
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
    
#if defined(VAS_USE_KISSFFT)|| defined(VAS_USE_PFFFT)
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
    
#if defined(VAS_USE_PFFFT)
#ifdef VAS_USE_AVX
    n = length *2;
    float *floatPtr = (float *)dest;
    __m256 zero = _mm256_setzero_ps();

    while(n)
    {
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

float vas_util_degrees2radians(float degrees)
{
    return degrees * (M_PI/180);
}

float vas_util_radians2degrees(float radians)
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

bool vas_util_isValidSegmentSize(int segmentSize)
{
    return  (segmentSize >= 16) && (segmentSize <= 131072)  && ((segmentSize & (segmentSize - 1)) == 0);
}

int vas_util_roundUp2NextPowerOf2(int value)
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

void vas_util_writeZeros1(int length, VAS_INPUTBUFFER *dest)
{
    int n = length;
    while(n--)
        *dest++ = 0;
}

void vas_util_writeZeros(int length, float *dest)
{
    int n = length;
    while(n--)
        *dest++ = 0;
}

float vas_util_fadeOut(float n, float length)
{
    float k = 1 -(n/length);
    float volume = (0.5  + 0.5 * cos(M_PI * k));
    return volume;
}

float vas_util_fadeIn(float n, float length)
{
    float k = 1 -(n/length);
    float volume = (0.5  - 0.5 * cos(M_PI * k));
    return volume;
}

void vas_util_writeFadeOutArray(float length, float *dest)
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

void vas_util_writeFadeInArray(float length, float *dest)
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

void vas_util_writeFadeOutArray1(float length, float *dest)
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

void vas_util_writeFadeInArray1(float length, float *dest)
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

void vas_util_copyFloatArray(int length, float *arr1, float *arr2)
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

/*note: it is faster to use the vas_window_blackman lookup tables than to calculate the window each time.*/
void vas_utilities_apply_blackman_window(VAS_INPUTBUFFER *x, int n) {
    int M = (n % 2 == 0) ? n / 2 : (n + 1) / 2;
    float win_factor = 0;
    for (int i = 0; i < M; ++i) {
        x[i] *= win_factor;
        x[n - 1 - i] *= win_factor;
        win_factor = 0.42f - 0.5f * cos(2 * M_PI * i / (n - 1)) + 0.08f * cos(4 * M_PI * i / (n - 1));
    }
}

void vas_utilities_apply_window(float* window, float*data, int n) {
    for (int i = 0; i < n/2; i++) {
        data[i] *= window[i];
        data[n-i] *= window[i];
    }
}

int vas_utilities_next_power_of_2(int n) {
    uint32_t K = n;
    K--;
    K |= K >> 1;
    K |= K >> 2;
    K |= K >> 4;
    K |= K >> 8;
    K |= K >> 16;
    K++;
    return K;
}

float vas_utilities_dB_to_lin(float dB) {
    return powf(10, dB/20.0);
}

float vas_utilities_lin_to_dB(float lin) {
    return 20 * log10(lin);
}

double vas_util_getWallTime(void) {
#ifdef __APPLE__
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ((double) (1000000000 * ts.tv_sec + ts.tv_nsec)) / 1000000000;
#else
    return 0
#endif
        /*struct timeval tv;
        struct timezone tz;
        gettimeofday(&tv, &tz);
        return ((double) (1000000 * tv.tv_sec + tv.tv_usec)) / 1000000;*/
}

double vas_util_getCPUTime(void) {
#ifdef __APPLE__
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return ((double) (1000000 * r.ru_utime.tv_sec + r.ru_utime.tv_usec)) / 1000000;
    #else
        return 0
    #endif
    /*struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return ((double) (1000000000 * ts.tv_sec + ts.tv_nsec)) / 1000000000;*/
}

unsigned int vas_util_getNumPhysicalCores(void)
{
  size_t len;
  unsigned int ncpu = 0;
#ifdef __APPLE__
  len = sizeof(ncpu);
  sysctlbyname ("hw.physicalcpu",&ncpu,&len,NULL,0);
#endif

  return ncpu;
}

unsigned int vas_util_getNumLogicalCores(void)
{
  size_t len;
  unsigned int ncpu = 8;
    
#ifdef __APPLE__
  len = sizeof(ncpu);
  sysctlbyname ("hw.logicalcpu",&ncpu,&len,NULL,0);
#endif

  return ncpu;
}

#endif
