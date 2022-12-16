/**
 * @file vas_util.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technische Universität Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based stuff.<br>
 * Many thanks to digilus for readline for windows <br>
 * https://github.com/digilus/getline <br>
 * <br>
 * @brief Utilty functions and all #defines for the VAS library <br>
 * <br>
 * All kinds of utility functions, mostly vector math
 */

#ifndef vas_util_h
#define vas_util_h

#if defined (__APPLE__)
#include <TargetConditionals.h>
#endif

//#define VAS_USE_KISSFFT
//#define VAS_USE_PFFFT
//#define VAS_USE_AVX

#if !defined(VAS_USE_VDSP) && !defined(VAS_USE_KISSFFT) && !defined(VAS_USE_PFFFT)

#ifdef __APPLE__
#pragma message ( "TARGET __APPLE__IS CHOSEN AUTOMATICALLY" )
#include <math.h>
#define VAS_EXPORT
#define VAS_USE_VDSP

#else

#ifdef _WIN32
#define VAS_EXPORT __declspec(dllexport)
#include <windows.h>
#include <intrin.h>
#include <stdint.h>
#else
#define VAS_EXPORT
#endif
#define _USE_MATH_DEFINES
#include <math.h>
#define VAS_USE_PFFFT

#endif
#endif

#if !defined(VAS_EXPORT) // MANUAL FFT AND OPTIMIZATION SELECTION

#pragma message ( "VAS_EXPORT NOT YET DEFINED" )
#pragma message ( "FFT AND OPTIMIZATION SETTINGS ARE CHOSEN MANUALLY" )

#ifdef _WIN32
#define VAS_EXPORT __declspec(dllexport)
#include <windows.h>
#include <intrin.h>
#include <stdint.h>
#define _USE_MATH_DEFINES
#include <math.h>
#else
#define VAS_EXPORT
#ifdef VAS_USE_AVX
#if TARGET_OS_IPHONE
#undef VAS_USE_AVX
#pragma message ( "AVX DEACTIVATED, TARGET IS IOS" )
#else
#include <x86intrin.h>
#endif
#endif
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
#pragma message ( "VDSP IS ACTIVE" )
#endif

#ifdef VAS_USE_KISSFFT
#include "kiss_fftr.h"
typedef kiss_fft_cpx VAS_COMPLEX;
#pragma message ( "KISSFFT IS ACTIVE" )
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
#pragma message ( "NEON IS ACTIVE" )
#endif
#include "pffft.h"
#pragma message ( "PFFFT IS ACTIVE" )
#endif

#ifdef VAS_USE_AVX
#pragma message ( "AVX IS ACTIVE" )
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
#define VAS_AZIMUTH_ANGLES_MAX 180
#define VAS_ELEVATIONZERO 30
#define VAS_AZIMUTH_STRIDE_MIN 2
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
    
void vas_util_complexMultiplyAddWithOne(VAS_COMPLEX *signalIn, VAS_COMPLEX *dest, int length) ;
    
int vas_getline(char **lineptr, size_t *n, FILE *stream);
    
char* vas_strsep(char** stringp, const char* delim);

void vas_util_debug(char *fmt, ...);
    
void vas_util_postSomeValue(VAS_COMPLEX *dest, int length);

float vas_util_fgain2db(float in);

float vas_util_faverageSumOfmagnitudes(float *in, int length);

void vas_util_single2DoublePrecision(float *in, double *out, int length);

void vas_util_double2SinglePrecision(double *in, float *out, int length);
    
void vas_util_fadd(float *input1, float *input2, float *dest, int length);
    
void vas_util_fmultiply(float *input1, float *input2, float *dest, int length);
    
void vas_util_fcopy(float *source, float *dest, int length);

void vas_util_fcopyUnalignedSource(float *source, float *dest, int length);

void vas_util_fcopy_noavx(float *source, float *dest, int length);
    
void vas_util_fscale(float *dest, float scale,  int length);

void vas_util_fmulitplyScalar(float *source, float scale, float *dest, int length);
    
void vas_util_complexScale(VAS_COMPLEX *dest, float scale,  int length);

void vas_util_complexCopy(VAS_COMPLEX *source, VAS_COMPLEX *dest, int length);

void vas_util_complexWriteZeros(VAS_COMPLEX *dest, int n);

void vas_util_complexMultiply(int length, VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest);

void vas_util_complexMultiplyAdd(VAS_COMPLEX *signalIn, VAS_COMPLEX *filter, VAS_COMPLEX *dest, int length);

void vas_util_deinterleaveComplexArray2(VAS_COMPLEX *input, float *realArray, float *imagArray, int length);

const char *vas_util_getFileExtension(const char *filename);

void vas_util_getFileExtension1(const char *filename, char *fileExtension);

float vas_util_degrees2radians(float degrees);

float vas_util_radians2degrees(float radians);

bool vas_util_isValidSegmentSize(int segmentSize);

int vas_util_roundUp2NextPowerOf2(int value);

void vas_util_writeZeros(int length, float *dest);

void vas_util_writeZeros1(int length, VAS_INPUTBUFFER *dest);

float vas_util_fadeOut(float n, float length);

float vas_util_fadeIn(float n, float length);

void vas_util_writeFadeOutArray(float length, float *dest);

void vas_util_writeFadeInArray(float length, float *dest);

void vas_util_writeFadeOutArray1(float length, float *dest);

void vas_util_writeFadeInArray1(float length, float *dest);

void vas_util_copyFloatArray(int length, float *arr1, float *arr2);

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
