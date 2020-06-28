/**
 * @file dynamicFirChannel.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based virtual acoustics (mainly dynamic binaural synthesis) <br>
 * <br>
 * @brief Class for convolution-based filtering.<br>
 * <br>
 * vas_dynamicFirChannel allows for convolution-based FIR filtering. <br>
 * The main class in this header is the vas_dynamicFirChannel. <br>
 * A Fir-Channel in this context contains an input, a filter and an output. <br>
 * In order to exchange the filter in realtime, the output contains two <br>
 * output arrays: one for the current azimuth/elevation and one for the target <br>
 * azimuth/elevation; they are named "current" and "next". <br>
 * In the moment of changing azimuth and/or elevation, the convolution of the input is <br>
 * calculated for both, the current and the next angle. <br>
 * To avoid discontinuities we must crossfade between current and next <br>
 * for at least 512 samples (at 44.1kHz). 256 is sufficient most of the time but in <br>
 * my experience not always. <br>
 * <br>
 * Static filters are also realized with vas_dynamicFirChannel; they have only one <br>
 * direction. <br>
 * <br>
 * Many thanks to Mark Borgerding for his KissFFT <br>
 * https://github.com/mborgerding/kissfft <br>
 * which - for any low latency partition size - is almost as fast as apple's vDSP FFT. <br>
 */

#ifndef vas_dynamicFirChannel_h
#define vas_dynamicFirChannel_h

#include <stdio.h>
#include "vas_mem.h"
#include "vas_util.h"
//#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * @brief Struct vas_fir_metaData. <br>
 * Format Meta data of a vas fir filter <br>
 */

typedef struct vas_fir_metaData
{
    char *fullPath;
    int filterLength;
    int filterLengthMinusOffset;
    int filterOffset;
    int filterEnd;
    int segmentSize;
    int directionFormat;
    int audioFormat;
    int lineFormat;
    int azimuthStride;
    int elevationStride;
    int eleZero;
    int eleRange;
    int eleMin;
    int eleMax;
    int aziRange;
} vas_fir_metaData;

void vas_filter_metaData_init(vas_fir_metaData *x);

/**
 * @brief Struct vas_dynamicFirChannel_filter. <br>
 * Holds the partitioned frequency responses of the filter <br>
 */

typedef struct vas_dynamicFirChannel_filter
{
    VAS_COMPLEX *data[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    VAS_COMPLEX **pointerToFFTSegments[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    
#ifdef VAS_USE_VDSP
    FFTSetup setupReal;   
#endif
#ifdef VAS_USE_KISSFFT
    kiss_fftr_cfg  forwardFFT;
    kiss_fftr_cfg  inverseFFT;
#endif
    bool *segmentIsZero[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    int zeroCounter[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    int nonZeroCounter[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
    float maxAverageSegmentPower[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    float minAverageSegmentPower[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    double *averageSegmentPower[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
    double overallEnergy[VAS_ELEVATION_ANGLES_MAX][VAS_AZIMUTH_ANGLES_MAX];
#endif
    
    int referenceCounter;
    int numberOfSegments;
    int segmentSize;
    int fftSize;
    int fftSizeLog2;
    int filterLength;
    int directionFormat;
    int eleMin;
    int eleMax;
    int eleZero;
    int eleRange;
    int eleStride;
    int aziRange;
    int aziStride;
    int offset;

} vas_dynamicFirChannel_filter;

/**
 * @brief Struct vas_dynamicFirChannel_input. <br>
 * Holds the partitioned frequency responses of the (realtime) input signal<br>
 */

typedef struct vas_dynamicFirChannel_input
{
    float *copy;
    float *real;
    float *imag;

    VAS_COMPLEX *data;
    VAS_COMPLEX **pointerToFFTSegments;
} vas_dynamicFirChannel_input;

/**
 * @brief Struct vas_dynamicFirChannel_target. <br>
 * Holds the calculated outputsignal including overlap. <br>
 */

typedef struct vas_dynamicFirChannel_target
{
#ifdef VAS_USE_VDSP
    VAS_COMPLEX signalComplex;
#else
    VAS_COMPLEX *signalComplex;
#endif
    float *signalFloat;
    float *overlap;
    int elevation;
    int azimuth;
} vas_dynamicFirChannel_target;

/**
 * @brief Struct vas_dynamicFirChannel_output. <br>
 * Holds the result for current and the "target" or next azimuth/elevation <br>
 */

typedef struct vas_dynamicFirChannel_output
{
    vas_dynamicFirChannel_target current;
    vas_dynamicFirChannel_target next;
    float *outputSegment;
    float *nextOutputSegment;
} vas_dynamicFirChannel_output;

/**
 * @brief Struct vas_dynamicFirChannel. <br>
 * Everything necessary for calculating (equal-) partitioned convolution including dynamic exchange of the filter <br>
 */

typedef struct vas_dynamicFirChannel
{
    int setup;                                  // set with flags as "VAS_GLOBALFILTER" or "VAS_LOCALFILTER", defined in vas_util.h
    float *tmp;                                 // tmp is used for zeropadding before calculating the frequency response
    float *fadeOut;                             // holds the fadeout array necessary for the crossfade
    float *fadeIn;                              // holds the fadein array necessary for the crossfade
    float *deInterleaveReal;                     // deInterleaveTmp is used deainterleaving the kissffts
    float *deInterleaveImag;                     // deInterleaveTmp is used deainterleaving the kissffts
    
    vas_dynamicFirChannel_input *input;          // my signal input
    vas_dynamicFirChannel_output output;        // calculated output
    vas_dynamicFirChannel_input *sharedInput;   // can be set if the channel uses the same input as another channel (->stereo reverb for example)
    vas_dynamicFirChannel_filter *sharedFilter; // can be set if the channel uses the same filter as another channel (->stereo reverb for example)
    vas_dynamicFirChannel_filter *filter;       // pointer to filter

#ifdef VAS_USE_FFTW
    fftwf_plan forwardFFT;
    fftwf_plan inverseFFT;
#endif
    
    int init;
    int pointerArrayMiddle;                     // index middle of the partitioned frequences resoponses of the filter (and input..)
    int pointerArraySize;                       // size of the partionied frequency response array (number of partitions * 2)
    int latency;                                // not in use yet
    int movingIndex;                            // movingIndex for accessing filter partitions
    int filterSize;                             // size of the filter
    int segmentIndex;
    int fadeCounter;
    int fadeLength;
    int activeCrossfade;
    int numberOfFramesForCrossfade;
    int elevationTmp, azimuthTmp;
    int useSharedInput;
    int useSharedFilter;
    int frameCounter;
    int startCrossfade;
    
    float gain;
    float segmentThreshold;
    double scale;
} vas_dynamicFirChannel;

/**
 * @brief Creates a new dynamicFirChannel. <br>
 * @param setup An integer containing byte flags for the filter configuration (have a look at vas_util.h) <br>
 * @return Returns a new, initialized fir channel. <br>
 * The dynamicFirChannel is a mono to mono channel containing all structures necessary for calculating <br>
 * an angle dependent convolution. Complex multichannel configurations can be created by using multiple instances <br>
 * which are capable of sharing the input, the filter or both. <br>
 */

vas_dynamicFirChannel *vas_dynamicFirChannel_new(int setup);

/**
 * @brief Frees a dynamicFirChannel. <br>
 * @param x The channel to be freed <br>
 */

void vas_dynamicFirChannel_free(vas_dynamicFirChannel *x);

/**
 * @brief Tells a channel to use the input from another channel. <br>
 * @param x The channel, which is supposed to use the input from another channel. <br>
 * @param sharedInputChannel The channel, which shares its input with x <br>
 * A mono to stereo reverb uses for left and right channel the same input. <br>
 * So it is not necessary to calculate the fft's of the input segments twice. <br>
 */
void vas_dynamicFirChannel_shareInputWith(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel);

/**
 * @brief Get shared filter values <br>
 * @param x The channel, which is supposed to use the filter from another channel. <br>
 * @param sharedInputChannel The channel, which shares its filter with x <br>
 * To be called only, after the sharedInputChannel is initialized with a filter. <br>
 */

void vas_dynamicFirChannel_getSharedFilterValues(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel);

/**
 * @brief Sets the filter size <br>
 * @param x The channel, which is supposed to use the filter from another channel. <br>
 * @param filterSize The size of the filter <br>
 * Usually called after opening a file containing the filter and its size <br>
 * All parameters necessary for the realtime convlution function are calculated here. <br>
 * (number of segements, pointers the filter segmentes etc.)<br>
 */
 
void vas_dynamicFirChannel_setFilterSize(vas_dynamicFirChannel *x, int filterSize);

#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER

void vas_dynamicFirChannel_calculateMinMaxAverageSegmentPower(vas_dynamicFirChannel *x, float *filter, int ele, int azi);

void vas_dynmaicFirChannel_resetMinMaxAverageSegmentPower(vas_dynamicFirChannel *x, int ele, int azi);
    
void vas_dynamicFirChannel_leaveActivePartitions(vas_dynamicFirChannel *x, int numberOfActivePartions);
#endif

/**
 * @brief Memory Allocation, partitioning and transformation to frequency domina for a filter <br>
 * @param x The target channel <br>
 * @param filter The filter to be partitioned and transformed to frequency domain <br>
 * @param ele The elevation index of the filter
 * @param azi The azimuth index of the filter
 * If the filter is not alreay initialized and transformed (by using a shared filter or a global filter)
 * the function allocates the necessary memory for storing the partitioned filter
 * It calculates on fft for every filter segment and does the zero padding
 */

void vas_dynamicFirChannel_prepareFilter(vas_dynamicFirChannel *x, float *filter, int ele, int azi);

/**
 * @brief Memory Allocation and preparation for the output signal <br>
 * @param x The target channel <br>
 * Allocates and prepares all necessary arrays for the output signal. <br>
 * Should be called directly, instead call prepareArrays() after the
 * filter is initialized or call initArraysWithGlobalFilter() if
 * the filter is global and alread initialized
 */

void vas_dynamicFirChannel_prepareOutputSignal(vas_dynamicFirChannel *x);

/**
 * @brief Memory Allocation and preparation for the input signal <br>
 * @param x The target channel <br>
 * Allocates and prepares all necessary arrays for the input signal. <br>
 * Should be called directly, instead call prepareArrays() after the
 * filter is initialized or call initArraysWithGlobalFilter() if
 * the filter is global and alread initialized
 */

void vas_dynamicFirChannel_prepareInputSignal(vas_dynamicFirChannel *x);

/**
 * @brief Memory Allocation and preparatino of all arrays except the filter <br>
 * @param x The target channel <br>
 * Allocates and prepares all necessary arrays including tmp, fadein, fadeout and
 * input and output arrays. Must be called after initialization of the filter because
 * filterLength must be known (usually after reading the filter).
 * vas_dynamicFirChannel_prepareOutputSignal() and vas_dynamicFirChannel_prepareInputSignal() <br>
 * are called from here.
 */

void vas_dynamicFirChannel_prepareArrays(vas_dynamicFirChannel *x);

/**
 * @brief DSP Processing method. Analyzed and prepares everything for realtime convolution <br>
 * @param x The target channel <br>
 * @param in The input vector <br>
 * @param out The output vector <br>
 * @param vectorSize The vector size <br>
 * @param flags  <br>
 * The function compares vector size and segemnt size either calls <br>
 * vas_dynamicFirChannel_calculateConvolution() several times within one block <br>
 * if segementSize is < than the vector size or collects vectors if segment size <br>
 * is >= vector size until the collected vectors together have the same size as the
 * segment size
 * flags can be set to VAS_OUTPUT_ADDINPLACE
 */

void vas_dynamicFirChannel_process(vas_dynamicFirChannel *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize, int flags);

void vas_dynamicFirChannel_setInitFlag(vas_dynamicFirChannel *x);

void vas_dynamicFirChannel_removeInitFlag(vas_dynamicFirChannel *x);
    
void vas_dynamicFirChannel_assignExternMemory(vas_dynamicFirChannel *x, float *real, float *imag);

void vas_dynamicFirChannel_multiplyAddSegments(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target);

void vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter(vas_dynamicFirChannel *x);

void vas_dynamicFirChannel_updateAzimuthAndElevation(vas_dynamicFirChannel *x);

void vas_dynamicFirChannel_calculateConvolution(vas_dynamicFirChannel *x);

void vas_dynamicFirChannel_setAzimuth(vas_dynamicFirChannel *x, int azimuth);

void vas_dynamicFirChannel_setElevation(vas_dynamicFirChannel *x, int elevation);
    
void vas_dynamicFirChannel_setSegmentThreshold(vas_dynamicFirChannel *x, float thresh);

void vas_dynamicFirChannel_setSegmentSize(vas_dynamicFirChannel *x, int segmentSize);

void vas_dynamicFirChannel_init1(vas_dynamicFirChannel *x, vas_fir_metaData *metaData, int segmentSize);
    
void vas_dynamicFirChannel_filter_free(vas_dynamicFirChannel_filter *x);

vas_dynamicFirChannel_input *vas_dynamicFirChannel_input_new(void);

void vas_dynamicFirChannel_input_reset(vas_dynamicFirChannel_input *x, int currentNumberOfSegments);

void vas_dynamicFirChannel_input_free(vas_dynamicFirChannel_input *x, int currentNumberOfSegments);

void vas_dynamicFirChannel_output_init(vas_dynamicFirChannel_output *x, int elevationZero);

void vas_dynamicFirChannel_output_free(vas_dynamicFirChannel_output *x);
    
    
#ifdef __cplusplus
}
#endif

#endif /* vas_dynamicFirChannel_h */
