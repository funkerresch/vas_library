/**
 * @file vas_fir.h
 * @author Thomas Resch <br>
 * Audio Communication Group, TU-Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * <br>
 * @brief Base-Class for stereo convolution filters<br>
 * <br>
 * All functions shared by the "real" stereo filter classes (binaural, staticFir etc..) <br>
 * are implemented in vas_filter. <br>
 * The class vas_filter should be considered opaque and shall not be used. <br>
 *
 */

#ifndef vas_fir_h
#define vas_fir_h

#include "vas_dynamicFirChannel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Struct vas_fir <br>
 * A "base"-class for all stereo fir filters <br>
 * Cast any *vas_fir_.*. to *vas_fir in order to use <br>
 * the functions below. <br>
 * Without calling vas_fir_setInitFlag, any filter will output zero<br>
 * It is your responsibility to call it after the filter is<br>
 * initialized correctly (usually after calling read).<br>
 *
 */

typedef struct vas_fir
{
    vas_fir_metaData metaData;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;    
} vas_fir;

void vas_fir_setInitFlag(vas_fir *x);
int vas_fir_getInitFlag(vas_fir *x);
void vas_fir_removeInitFlag(vas_fir *x);
void vas_fir_prepareChannelsWithSharedFilter(vas_fir *x,  vas_dynamicFirChannel *left, vas_dynamicFirChannel *right);
void vas_fir_setMetaData(vas_fir *x, int directionFormat, int length);
void vas_fir_readText_1ValuePerLine(vas_fir *x, char *fullpath);
FILE *vas_fir_readText_metaData1(vas_fir *x, char *fullpath);
void vas_fir_readText_Ir2(vas_fir *x, FILE *filePtr);
void vas_fir_readText_Ir1(vas_fir *x, FILE *filePtr, int offset);
void vas_fir_initFilter2(vas_fir *x, int segmentSize);
void *vas_fir_readSofa_getMetaData(vas_fir *x, char *fullpath);
int vas_fir_readSofa_getFilter(vas_fir *x, void *filter);
void vas_fir_setAdditionalMetaData(vas_fir *x, int segmentSize, int offset, int end);
void vas_fir_setMultiDirection3DegreeGridResoluion(vas_fir *x, int filterLength, int segmentSize, int offset, int end);
void vas_fir_setMetaData_manually1(vas_fir *x, int filterLength, int segmentSize, int directionFormat, int eleStride, int aziStride, int audioFormat, int lineFormat, int offset, int end);
void vas_fir_test_4096_1024_azimuthStride3(vas_fir *x);
    
#ifdef __cplusplus
}
#endif

#endif
