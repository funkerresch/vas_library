/**
 * @file vas_util.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based stuff.<br>
 * Many thanks to gpakosz for whereami <br>
 * https://github.com/gpakosz/whereami <br>
 * Many thanks to digilus for readline for windows <br>
 * https://github.com/digilus/getline <br>
 * <br>
 * @brief Utilty functions and all #defines for the VAS library <br>
 * <br>
 * All kinds of utility functions, mostly vector math
 */

#ifndef vas_util_h
#define vas_util_h

//#define VAS_USE_KISSFFT
//#define VAS_USE_PFFFT
//#define VAS_USE_AVX

//#define VAS_USE_MULTITHREADREFLECTION

#if !defined(VAS_USE_VDSP) && !defined(VAS_USE_KISSFFT) && !defined(VAS_USE_PFFFT)

#ifdef __APPLE__

#include <math.h>
#define VAS_EXPORT
#define VAS_USE_VDSP

#else

#ifdef _WIN32
#define VAS_EXPORT __declspec(dllexport)
#else
#define VAS_EXPORT
#endif
#define _USE_MATH_DEFINES
#include <math.h>
#define VAS_USE_PFFFT

#endif

#endif

#if !defined(VAS_EXPORT)

#ifdef _WIN32
#define VAS_EXPORT __declspec(dllexport)
#define _USE_MATH_DEFINES
#include <math.h>
#else
#define VAS_EXPORT
#include <math.h>
#endif

#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef VAS_USE_VDSP
#include <Accelerate/Accelerate.h>
typedef COMPLEX_SPLIT VAS_COMPLEX;
#endif

#ifdef VAS_USE_KISSFFT
#include "kiss_fftr.h"
typedef kiss_fft_cpx VAS_COMPLEX;
#endif

#ifdef VAS_USE_PFFFT
typedef struct kiss_fft_cpx // pffft's complex has the same format
{
    float r;
    float i;
} kiss_fft_cpx;

typedef kiss_fft_cpx VAS_COMPLEX;
#ifdef __arm__
#include <arm_neon.h>
#endif
#include "pffft.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//#ifdef VAS_INPUTVECTOR_USE_FLOAT
typedef float VAS_INPUTBUFFER;
///#else
//typedef double VAS_OUTPUTBUFFER;
//#endif

//#ifdef VAS_OUTPUTVECTOR_USE_FLOAT
typedef float VAS_OUTPUTBUFFER;
//#else
//typedef double VAS_OUTPUTBUFFER;
//#endif

#define VAS_MAXVECTORSIZE 1024

#define VAS_ELEVATION_ANGLES_MAX 60
#define VAS_AZIMUTH_ANGLES_MAX 120
#define VAS_ELEVATIONZERO 30
#define VAS_AZIMUTH_STRIDE_MIN 3
#define VAS_ELEVATION_STRIDE_MIN 3

#define VAS_OUTPUTVECTOR_ADDINPLACE 1
#define VAS_ONESEGMENT 2

#define VAS_UNDEFINED 0
#define VAS_GLOBALFILTER_LEFT 1
#define VAS_GLOBALFILTER_RIGHT 1 << 1
#define VAS_LOCALFILTER 1 << 2
#define VAS_STATICFILTER 1 << 3
#define VAS_GLOBALFILTER 1 << 4

#define VAS_VDSP 1 << 6
#define VAS_FFTW 1 << 7
#define VAS_SSE 1 << 8
#define VAS_TIMEDOMAIN 1 << 9
#define VASFREQDOMAIN 1 << 10
#define VAS_GLOBALBINAURALFILTER 1 << 11
#define VAS_BINAURALSETUP_STD 1 << 12
#define VAS_BINAURALSETUP_NOELEVATION 1 << 13

#define VAS_LEFTCHANNEL 1
#define VAS_RIGHTCHANNEL 2

#define VAS_MINCROSSFADELENGTH 512

#define VAS_IR_AUDIOFORMAT_MONO 1
#define VAS_IR_AUDIOFORMAT_STEREO 2
#define VAS_IR_AUDIOFORMAT_MULTICHANNEL_3 3
#define VAS_IR_DIRECTIONFORMAT_SINGLE 1             // direction independent data
#define VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH 2      // multiple IR's for different azimuth
#define VAS_IR_DIRECTIONFORMAT_MULTI 3              // multiple IR's for different azimuth and elvation
#define VAS_IR_LINEFORMAT_IR 1                      // one filter each line
#define VAS_IR_LINEFORMAT_VALUE 2                   // one value in each line, only valid for VAS_IR_DIRECTIONFORMAT_SINGLE

#define VAS_AZI_OFFSET_51_LEFT 300
#define VAS_AZI_OFFSET_51_CENTER 0
#define VAS_AZI_OFFSET_51_RIGHT 60
#define VAS_AZI_OFFSET_51_SURROUNDLEFT 240
#define VAS_AZI_OFFSET_51_SURROUNDRIGHT 120
    
#define VAS_NOERROR 0
#define VAS_ERROR_MEM 1
#define VAS_ERROR_READFILE 1 << 1
#define VAS_ERROR_METADATA 1 << 2


#ifndef WHEREAMI_H
#define WHEREAMI_H
        
#ifndef WAI_FUNCSPEC
#define WAI_FUNCSPEC
#endif
#ifndef WAI_PREFIX
#define WAI_PREFIX(function) wai_##function
#endif
        
        /**
         * Returns the path to the current executable.
         *
         * Usage:
         *  - first call `int length = wai_getExecutablePath(NULL, 0, NULL);` to
         *    retrieve the length of the path
         *  - allocate the destination buffer with `path = (char*)malloc(length + 1);`
         *  - call `wai_getExecutablePath(path, length, NULL)` again to retrieve the
         *    path
         *  - add a terminal NUL character with `path[length] = '\0';`
         *
         * @param out destination buffer, optional
         * @param capacity destination buffer capacity
         * @param dirname_length optional recipient for the length of the dirname part
         *   of the path.
         *
         * @return the length of the executable path on success (without a terminal NUL
         * character), otherwise `-1`
         */
        WAI_FUNCSPEC
        int WAI_PREFIX(getExecutablePath)(char* out, int capacity, int* dirname_length);
        
        /**
         * Returns the path to the current module
         *
         * Usage:
         *  - first call `int length = wai_getModulePath(NULL, 0, NULL);` to retrieve
         *    the length  of the path
         *  - allocate the destination buffer with `path = (char*)malloc(length + 1);`
         *  - call `wai_getModulePath(path, length, NULL)` again to retrieve the path
         *  - add a terminal NUL character with `path[length] = '\0';`
         *
         * @param out destination buffer, optional
         * @param capacity destination buffer capacity
         * @param dirname_length optional recipient for the length of the dirname part
         *   of the path.
         *
         * @return the length of the module path on success (without a terminal NUL
         * character), otherwise `-1`
         */
        WAI_FUNCSPEC
        int WAI_PREFIX(getModulePath)(char* out, int capacity, int* dirname_length);
        
    
#endif // #ifndef WHEREAMI_H
    
void vas_util_complexMultiplyAddWithOne(VAS_COMPLEX *signalIn, VAS_COMPLEX *dest, int length) ;
    
int vas_getline(char **lineptr, size_t *n, FILE *stream);
    
char* vas_strsep(char** stringp, const char* delim);
    
void vas_util_postSomeValue(VAS_COMPLEX *dest, int length);

float vas_util_fgain2db(float in);

float vas_util_faverageSumOfmagnitudes(float *in, int length);

void vas_util_single2DoublePrecision(float *in, double *out, int length);

void vas_util_double2SinglePrecision(double *in, float *out, int length);
    
void vas_util_fadd(float *input1, float *input2, float *dest, int length);
    
void vas_util_fmultiply(float *input1, float *input2, float *dest, int length);
    
void vas_util_fcopy(float *source, float *dest, int length);

void vas_util_fcopy_noavx(float *source, float *dest, int length);
    
void vas_util_fscale(float *dest, float scale,  int length);

void vas_util_fmulitplyScalar(float *source, float scale, float *dest, int length);
    
void vas_util_complexScale(VAS_COMPLEX *dest, float scale,  int length);

void vas_util_complexCopy(VAS_COMPLEX *source, VAS_COMPLEX *dest, int length);

void vas_util_complexWriteZeros(VAS_COMPLEX *dest, int n);

void vas_util_complexMultiply(int length, VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest);

void vas_util_complexMultiplyAdd(VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest, int length);

void vas_util_complexMultiplyAdd2(VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest, int length);

void vas_util_deinterleaveComplexArray2(VAS_COMPLEX *input, float *realArray, float *imagArray, int length);

const char *vas_util_getFileExtension(const char *filename);

void vas_util_getFileExtension1(const char *filename, char *fileExtension);

float vas_utilities_degrees2radians(float degrees);

float vas_utilities_radians2degrees(float radians);

bool vas_utilities_isValidSegmentSize(int segmentSize);

int vas_utilities_roundUp2NextPowerOf2(int value);

void ak_vaTools_zmultiply_SSE(int length, VAS_COMPLEX signalIn, VAS_COMPLEX filter, VAS_COMPLEX dest);                //complex-multiplication

void ak_vaTools_zmultiplyAdd_SSE(int length, VAS_COMPLEX signalIn, VAS_COMPLEX filter, VAS_COMPLEX dest);               //complex-multiplication

void ak_vaTools_fadd_SSE(int n, float *signalIn, float *filter, float *dest);

void vas_utilities_fcopy_SSE(int n, float *source,  float *dest);

void vas_utilities_writeZeros(int length, float *dest);

void vas_utilities_writeZeros1(int length, VAS_INPUTBUFFER *dest);

float vas_utilities_fadeOut(float n, float length);

float vas_utilities_fadeIn(float n, float length);

void vas_utilities_writeFadeOutArray(float length, float *dest);

void vas_utilities_writeFadeInArray(float length, float *dest);

void vas_utilities_writeFadeOutArray1(float length, float *dest);

void vas_utilities_writeFadeInArray1(float length, float *dest);

void vas_utilities_copyFloatArray(int length, float *arr1, float *arr2);
    
#ifdef VAS_USE_VDSP
    
void vas_utilities_scaleComplex_vDSP(int n, float scale, COMPLEX_SPLIT dest);
    
void vas_utilities_writeZerosComplex_vDSP(int n, COMPLEX_SPLIT dest);
    
void vas_utilities_fcopy_vDSP(int length, float *source, float *dest);

void vas_utilities_copy_vDSP(int length, double *source, double *dest);

void vas_utilities_zcopy_vDSP(int length, COMPLEX_SPLIT source, COMPLEX_SPLIT dest);

void vas_utilities_fadd_vDSP(int length, float *signalIn, float *filter, float *dest);

void vas_utilities_fmultiply_vDSP(int length, float *signalIn, float *filter, float *dest);

void vas_utilities_fscale_vDSP(int length, float *signal, float scale);

void vas_utilities_scale_vDSP(int length, double *signal, double scale);

void vas_utilities_zadd_vDSP(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest);

void vas_utilities_zmultiply_vDSP(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest);

void vas_utilities_zmultiplyAdd_vDSP(int length, COMPLEX_SPLIT signalIn, COMPLEX_SPLIT filter, COMPLEX_SPLIT dest);

#endif

void vas_utilities_apply_blackman_window(VAS_INPUTBUFFER *x, int n);

void vas_utilities_apply_window(float* window, float*data, int n);

int vas_utilities_next_power_of_2(int n);

float vas_utilities_dB_to_lin(float dB);

float vas_utilities_lin_to_dB(float lin);

double vas_util_getWallTime(void);

double vas_util_getCPUTime(void);

unsigned int vas_util_getNumPhysicalCores(void);

unsigned int vas_util_getNumLogicalCores(void);

#ifdef __cplusplus
}
#endif

#endif
