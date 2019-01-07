/**
 * @file vas_fir.h
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - "Base" class for all filters <br>
 *
 * All functions shared by the "real" stereo filter classes (binaural, staticFir etc..) <br>
 * are implemented in vas_filter. <br>
 * The class vas_filter itself can not be used. <br>
 *
 */

#ifndef vas_fir_h
#define vas_fir_h

#include "vas_dynamicFirChannel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Struct vas_fir_metaData. <br>
 * Format Meta data of a vas fir filter <br>
 */

typedef struct vas_fir_metaData
{
    int filterLength;
    int directionFormat;
    int audioFormat;
    int lineFormat;
    int azimuthStride;
    int elevationStride;
} vas_fir_metaData;

void vas_filter_metaData_init(vas_fir_metaData *x);

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
    vas_fir_metaData description;
    vas_dynamicFirChannel *left;
    vas_dynamicFirChannel *right;    
} vas_fir;

void vas_fir_setInitFlag(vas_fir *x);
int vas_fir_getInitFlag(vas_fir *x);
void vas_fir_removeInitFlag(vas_fir *x);
void vas_fir_prepareChannelsWithSharedFilter(vas_fir *x,  vas_dynamicFirChannel *left, vas_dynamicFirChannel *right);

void vas_fir_readText_1ValuePerLine(vas_fir *x, char *fullpath);
void vas_fir_readText_1IrPerLine(vas_fir *x, char *fullpath);
int vas_fir_readSofa(vas_fir *x, char *fullpath, vas_dynamicFirChannel_config *firSetup);
    
#ifdef __cplusplus
}
#endif

#endif
