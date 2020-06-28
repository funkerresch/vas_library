/**
* @file VasUnitySpatializer.h
* @author Thomas Resch
* @date 10 Sep 2018
* @brief C - Binaural filter plugin <br>
*
* The plugin calculates dynamic binaural synthesis based on<br>
 the source-listener rotation matrix.<br>
 For BRIR-based calculations it is possible to use<br>
 listener orientation only. In VAS_SPAT_CONFIG_AUTO-mode an experimental<br>
 raytracing-based synthesis is calculated.<br>
 <br>
 In order to save resources it is possible to load <br>
 a static reverb tail with a larger vector size. The tail IR must
 contain "BRIR.lengh"-zeros in the beginning.<br>
*
*/

#include "AudioPluginUtil.h"
#include "vas_fir_binauralReflection.h"
#include "vas_delay_crossfade.h"
#include "vas_iir_biquad.h"
#include "vas_delay_crossfade.h"

#ifndef DEBUG_UNITY
#define DEBUG_UNITY
typedef void (*FuncPtr)( const char * );
FuncPtr Debug = NULL;
#endif

#define VAS_MAXNUMBEROFRAYS 12
#define VAS_MAXREFLECTIONORDER 12
#define VAS_REFLECTIONPARAMETERS 7
#define VAS_DEBUG_TO_UNITY

#define VAS_SPAT_CONFIG_SIMPLE 1
#define VAS_SPAT_CONFIG_MANUAL 2
#define VAS_SPAT_CONFIG_AUTO 3

using namespace std;

namespace Spatializer
{
    enum
    {
        P_AUDIOSRCATTN,
        P_FIXEDVOLUME,
        P_CUSTOMFALLOFF,
        P_SPATID,
        P_H_SOURCEDIRECTIVITY,
        P_H_FULLPOWERRANGE,
        P_V_SOURCEDIRECTIVITY,
        P_V_FULLPOWERRANGE,
        P_H_DIRECTIVITYDAMPING,
        P_V_DIRECTIVITYDAMPING,
        P_NUMBEROFRAYS,
        P_REFLECTIONORDER,
        P_INVERSEAZI,
        P_INVERSEELE,
        P_BYPASS,
        P_LISTENERORIENTATIONONLY,
        P_SEGMENTSIZE_EARLYPART,
        P_SEGMENTSIZE_LATEPART,
        P_DISTANCE_SCALING,
        P_OCCLUSION,
        
        P_REF_1_1_X, //9
        P_REF_1_1_Y,
        P_REF_1_1_Z,
        P_REF_1_1_MAT,
        P_REF_1_1_MUTE,
        P_REF_1_1_SCALE,
        P_REF_1_1_DIST,
        
        P_REF_1_2_X, //13
        P_REF_1_2_Y,
        P_REF_1_2_Z,
        P_REF_1_2_MAT,
        P_REF_1_2_MUTE,
        P_REF_1_2_SCALE,
        P_REF_1_2_DIST,
        
        P_REF_1_3_X,
        P_REF_1_3_Y,
        P_REF_1_3_Z,
        P_REF_1_3_MAT,
        P_REF_1_3_MUTE,
        P_REF_1_3_SCALE,
        P_REF_1_3_DIST,
        
        P_REF_1_4_X,
        P_REF_1_4_Y,
        P_REF_1_4_Z,
        P_REF_1_4_MAT,
        P_REF_1_4_MUTE,
        P_REF_1_4_SCALE,
        P_REF_1_4_DIST,
        
        P_REF_1_5_X,
        P_REF_1_5_Y,
        P_REF_1_5_Z,
        P_REF_1_5_MAT,
        P_REF_1_5_MUTE,
        P_REF_1_5_SCALE,
        P_REF_1_5_DIST,
        
        P_REF_1_6_X,
        P_REF_1_6_Y,
        P_REF_1_6_Z,
        P_REF_1_6_MAT,
        P_REF_1_6_MUTE,
        P_REF_1_6_SCALE,
        P_REF_1_6_DIST,
        
        P_REF_1_7_X,
        P_REF_1_7_Y,
        P_REF_1_7_Z,
        P_REF_1_7_MAT,
        P_REF_1_7_MUTE,
        P_REF_1_7_SCALE,
        P_REF_1_7_DIST,
        
        P_REF_1_8_X,
        P_REF_1_8_Y,
        P_REF_1_8_Z,
        P_REF_1_8_MAT,
        P_REF_1_8_MUTE,
        P_REF_1_8_SCALE,
        P_REF_1_8_DIST,
        
        P_REF_1_9_X,
        P_REF_1_9_Y,
        P_REF_1_9_Z,
        P_REF_1_9_MAT,
        P_REF_1_9_MUTE,
        P_REF_1_9_SCALE,
        P_REF_1_9_DIST,
        
        P_REF_1_10_X,
        P_REF_1_10_Y,
        P_REF_1_10_Z,
        P_REF_1_10_MAT,
        P_REF_1_10_MUTE,
        P_REF_1_10_SCALE,
        P_REF_1_10_DIST,
        
        P_REF_1_11_X,
        P_REF_1_11_Y,
        P_REF_1_11_Z,
        P_REF_1_11_MAT,
        P_REF_1_11_MUTE,
        P_REF_1_11_SCALE,
        P_REF_1_11_DIST,
        
        P_REF_1_12_X,
        P_REF_1_12_Y,
        P_REF_1_12_Z,
        P_REF_1_12_MAT,
        P_REF_1_12_MUTE,
        P_REF_1_12_SCALE,
        P_REF_1_12_DIST,
        
        P_REF_2_1_X, //9
        P_REF_2_1_Y,
        P_REF_2_1_Z,
        P_REF_2_1_MAT,
        P_REF_2_1_MUTE,
        P_REF_2_1_SCALE,
        P_REF_2_1_DIST,
        
        P_REF_2_2_X, //13
        P_REF_2_2_Y,
        P_REF_2_2_Z,
        P_REF_2_2_MAT,
        P_REF_2_2_MUTE,
        P_REF_2_2_SCALE,
        P_REF_2_2_DIST,
        
        P_REF_2_3_X,
        P_REF_2_3_Y,
        P_REF_2_3_Z,
        P_REF_2_3_MAT,
        P_REF_2_3_MUTE,
        P_REF_2_3_SCALE,
        P_REF_2_3_DIST,
        
        P_REF_2_4_X,
        P_REF_2_4_Y,
        P_REF_2_4_Z,
        P_REF_2_4_MAT,
        P_REF_2_4_MUTE,
        P_REF_2_4_SCALE,
        P_REF_2_4_DIST,
        
        P_REF_2_5_X,
        P_REF_2_5_Y,
        P_REF_2_5_Z,
        P_REF_2_5_MAT,
        P_REF_2_5_MUTE,
        P_REF_2_5_SCALE,
        P_REF_2_5_DIST,
        
        P_REF_2_6_X,
        P_REF_2_6_Y,
        P_REF_2_6_Z,
        P_REF_2_6_MAT,
        P_REF_2_6_MUTE,
        P_REF_2_6_SCALE,
        P_REF_2_6_DIST,
        
        P_REF_2_7_X,
        P_REF_2_7_Y,
        P_REF_2_7_Z,
        P_REF_2_7_MAT,
        P_REF_2_7_MUTE,
        P_REF_2_7_SCALE,
        P_REF_2_7_DIST,
        
        P_REF_2_8_X,
        P_REF_2_8_Y,
        P_REF_2_8_Z,
        P_REF_2_8_MAT,
        P_REF_2_8_MUTE,
        P_REF_2_8_SCALE,
        P_REF_2_8_DIST,
        
        P_REF_2_9_X,
        P_REF_2_9_Y,
        P_REF_2_9_Z,
        P_REF_2_9_MAT,
        P_REF_2_9_MUTE,
        P_REF_2_9_SCALE,
        P_REF_2_9_DIST,
        
        P_REF_2_10_X,
        P_REF_2_10_Y,
        P_REF_2_10_Z,
        P_REF_2_10_MAT,
        P_REF_2_10_MUTE,
        P_REF_2_10_SCALE,
        P_REF_2_10_DIST,
        
        P_REF_2_11_X,
        P_REF_2_11_Y,
        P_REF_2_11_Z,
        P_REF_2_11_MAT,
        P_REF_2_11_MUTE,
        P_REF_2_11_SCALE,
        P_REF_2_11_DIST,
        
        P_REF_2_12_X,
        P_REF_2_12_Y,
        P_REF_2_12_Z,
        P_REF_2_12_MAT,
        P_REF_2_12_MUTE,
        P_REF_2_12_SCALE,
        P_REF_2_12_DIST,
        
        P_REF_3_1_X, //9
        P_REF_3_1_Y,
        P_REF_3_1_Z,
        P_REF_3_1_MAT,
        P_REF_3_1_MUTE,
        P_REF_3_1_SCALE,
        P_REF_3_1_DIST,
        
        P_REF_3_2_X, //13
        P_REF_3_2_Y,
        P_REF_3_2_Z,
        P_REF_3_2_MAT,
        P_REF_3_2_MUTE,
        P_REF_3_2_SCALE,
        P_REF_3_2_DIST,
        
        P_REF_3_3_X,
        P_REF_3_3_Y,
        P_REF_3_3_Z,
        P_REF_3_3_MAT,
        P_REF_3_3_MUTE,
        P_REF_3_3_SCALE,
        P_REF_3_3_DIST,
        
        P_REF_3_4_X,
        P_REF_3_4_Y,
        P_REF_3_4_Z,
        P_REF_3_4_MAT,
        P_REF_3_4_MUTE,
        P_REF_3_4_SCALE,
        P_REF_3_4_DIST,
        
        P_REF_3_5_X,
        P_REF_3_5_Y,
        P_REF_3_5_Z,
        P_REF_3_5_MAT,
        P_REF_3_5_MUTE,
        P_REF_3_5_SCALE,
        P_REF_3_5_DIST,
        
        P_REF_3_6_X,
        P_REF_3_6_Y,
        P_REF_3_6_Z,
        P_REF_3_6_MAT,
        P_REF_3_6_MUTE,
        P_REF_3_6_SCALE,
        P_REF_3_6_DIST,
        
        P_REF_3_7_X,
        P_REF_3_7_Y,
        P_REF_3_7_Z,
        P_REF_3_7_MAT,
        P_REF_3_7_MUTE,
        P_REF_3_7_SCALE,
        P_REF_3_7_DIST,
        
        P_REF_3_8_X,
        P_REF_3_8_Y,
        P_REF_3_8_Z,
        P_REF_3_8_MAT,
        P_REF_3_8_MUTE,
        P_REF_3_8_SCALE,
        P_REF_3_8_DIST,
        
        P_REF_3_9_X,
        P_REF_3_9_Y,
        P_REF_3_9_Z,
        P_REF_3_9_MAT,
        P_REF_3_9_MUTE,
        P_REF_3_9_SCALE,
        P_REF_3_9_DIST,
        
        P_REF_3_10_X,
        P_REF_3_10_Y,
        P_REF_3_10_Z,
        P_REF_3_10_MAT,
        P_REF_3_10_MUTE,
        P_REF_3_10_SCALE,
        P_REF_3_10_DIST,
        
        P_REF_3_11_X,
        P_REF_3_11_Y,
        P_REF_3_11_Z,
        P_REF_3_11_MAT,
        P_REF_3_11_MUTE,
        P_REF_3_11_SCALE,
        P_REF_3_11_DIST,
        
        P_REF_3_12_X,
        P_REF_3_12_Y,
        P_REF_3_12_Z,
        P_REF_3_12_MAT,
        P_REF_3_12_MUTE,
        P_REF_3_12_SCALE,
        P_REF_3_12_DIST,
        
        P_REF_4_1_X, //9
        P_REF_4_1_Y,
        P_REF_4_1_Z,
        P_REF_4_1_MAT,
        P_REF_4_1_MUTE,
        P_REF_4_1_SCALE,
        P_REF_4_1_DIST,
        
        P_REF_4_2_X, //13
        P_REF_4_2_Y,
        P_REF_4_2_Z,
        P_REF_4_2_MAT,
        P_REF_4_2_MUTE,
        P_REF_4_2_SCALE,
        P_REF_4_2_DIST,
        
        P_REF_4_3_X,
        P_REF_4_3_Y,
        P_REF_4_3_Z,
        P_REF_4_3_MAT,
        P_REF_4_3_MUTE,
        P_REF_4_3_SCALE,
        P_REF_4_3_DIST,
        
        P_REF_4_4_X,
        P_REF_4_4_Y,
        P_REF_4_4_Z,
        P_REF_4_4_MAT,
        P_REF_4_4_MUTE,
        P_REF_4_4_SCALE,
        P_REF_4_4_DIST,
        
        P_REF_4_5_X,
        P_REF_4_5_Y,
        P_REF_4_5_Z,
        P_REF_4_5_MAT,
        P_REF_4_5_MUTE,
        P_REF_4_5_SCALE,
        P_REF_4_5_DIST,
        
        P_REF_4_6_X,
        P_REF_4_6_Y,
        P_REF_4_6_Z,
        P_REF_4_6_MAT,
        P_REF_4_6_MUTE,
        P_REF_4_6_SCALE,
        P_REF_4_6_DIST,
        
        P_REF_4_7_X,
        P_REF_4_7_Y,
        P_REF_4_7_Z,
        P_REF_4_7_MAT,
        P_REF_4_7_MUTE,
        P_REF_4_7_SCALE,
        P_REF_4_7_DIST,
        
        P_REF_4_8_X,
        P_REF_4_8_Y,
        P_REF_4_8_Z,
        P_REF_4_8_MAT,
        P_REF_4_8_MUTE,
        P_REF_4_8_SCALE,
        P_REF_4_8_DIST,
        
        P_REF_4_9_X,
        P_REF_4_9_Y,
        P_REF_4_9_Z,
        P_REF_4_9_MAT,
        P_REF_4_9_MUTE,
        P_REF_4_9_SCALE,
        P_REF_4_9_DIST,
        
        P_REF_4_10_X,
        P_REF_4_10_Y,
        P_REF_4_10_Z,
        P_REF_4_10_MAT,
        P_REF_4_10_MUTE,
        P_REF_4_10_SCALE,
        P_REF_4_10_DIST,
        
        P_REF_4_11_X,
        P_REF_4_11_Y,
        P_REF_4_11_Z,
        P_REF_4_11_MAT,
        P_REF_4_11_MUTE,
        P_REF_4_11_SCALE,
        P_REF_4_11_DIST,
        
        P_REF_4_12_X,
        P_REF_4_12_Y,
        P_REF_4_12_Z,
        P_REF_4_12_MAT,
        P_REF_4_12_MUTE,
        P_REF_4_12_SCALE,
        P_REF_4_12_DIST,
        
        P_REF_5_1_X, //9
        P_REF_5_1_Y,
        P_REF_5_1_Z,
        P_REF_5_1_MAT,
        P_REF_5_1_MUTE,
        P_REF_5_1_SCALE,
        P_REF_5_1_DIST,
        
        P_REF_5_2_X, //13
        P_REF_5_2_Y,
        P_REF_5_2_Z,
        P_REF_5_2_MAT,
        P_REF_5_2_MUTE,
        P_REF_5_2_SCALE,
        P_REF_5_2_DIST,
        
        P_REF_5_3_X,
        P_REF_5_3_Y,
        P_REF_5_3_Z,
        P_REF_5_3_MAT,
        P_REF_5_3_MUTE,
        P_REF_5_3_SCALE,
        P_REF_5_3_DIST,
        
        P_REF_5_4_X,
        P_REF_5_4_Y,
        P_REF_5_4_Z,
        P_REF_5_4_MAT,
        P_REF_5_4_MUTE,
        P_REF_5_4_SCALE,
        P_REF_5_4_DIST,
        
        P_REF_5_5_X,
        P_REF_5_5_Y,
        P_REF_5_5_Z,
        P_REF_5_5_MAT,
        P_REF_5_5_MUTE,
        P_REF_5_5_SCALE,
        P_REF_5_5_DIST,
        
        P_REF_5_6_X,
        P_REF_5_6_Y,
        P_REF_5_6_Z,
        P_REF_5_6_MAT,
        P_REF_5_6_MUTE,
        P_REF_5_6_SCALE,
        P_REF_5_6_DIST,
        
        P_REF_5_7_X,
        P_REF_5_7_Y,
        P_REF_5_7_Z,
        P_REF_5_7_MAT,
        P_REF_5_7_MUTE,
        P_REF_5_7_SCALE,
        P_REF_5_7_DIST,
        
        P_REF_5_8_X,
        P_REF_5_8_Y,
        P_REF_5_8_Z,
        P_REF_5_8_MAT,
        P_REF_5_8_MUTE,
        P_REF_5_8_SCALE,
        P_REF_5_8_DIST,
        
        P_REF_5_9_X,
        P_REF_5_9_Y,
        P_REF_5_9_Z,
        P_REF_5_9_MAT,
        P_REF_5_9_MUTE,
        P_REF_5_9_SCALE,
        P_REF_5_9_DIST,
        
        P_REF_5_10_X,
        P_REF_5_10_Y,
        P_REF_5_10_Z,
        P_REF_5_10_MAT,
        P_REF_5_10_MUTE,
        P_REF_5_10_SCALE,
        P_REF_5_10_DIST,
        
        P_REF_5_11_X,
        P_REF_5_11_Y,
        P_REF_5_11_Z,
        P_REF_5_11_MAT,
        P_REF_5_11_MUTE,
        P_REF_5_11_SCALE,
        P_REF_5_11_DIST,
        
        P_REF_5_12_X,
        P_REF_5_12_Y,
        P_REF_5_12_Z,
        P_REF_5_12_MAT,
        P_REF_5_12_MUTE,
        P_REF_5_12_SCALE,
        P_REF_5_12_DIST,
            
        P_REF_6_1_X, //9
        P_REF_6_1_Y,
        P_REF_6_1_Z,
        P_REF_6_1_MAT,
        P_REF_6_1_MUTE,
        P_REF_6_1_SCALE,
        P_REF_6_1_DIST,
        
        P_REF_6_2_X, //13
        P_REF_6_2_Y,
        P_REF_6_2_Z,
        P_REF_6_2_MAT,
        P_REF_6_2_MUTE,
        P_REF_6_2_SCALE,
        P_REF_6_2_DIST,
        
        P_REF_6_3_X,
        P_REF_6_3_Y,
        P_REF_6_3_Z,
        P_REF_6_3_MAT,
        P_REF_6_3_MUTE,
        P_REF_6_3_SCALE,
        P_REF_6_3_DIST,
        
        P_REF_6_4_X,
        P_REF_6_4_Y,
        P_REF_6_4_Z,
        P_REF_6_4_MAT,
        P_REF_6_4_MUTE,
        P_REF_6_4_SCALE,
        P_REF_6_4_DIST,
        
        P_REF_6_5_X,
        P_REF_6_5_Y,
        P_REF_6_5_Z,
        P_REF_6_5_MAT,
        P_REF_6_5_MUTE,
        P_REF_6_5_SCALE,
        P_REF_6_5_DIST,
        
        P_REF_6_6_X,
        P_REF_6_6_Y,
        P_REF_6_6_Z,
        P_REF_6_6_MAT,
        P_REF_6_6_MUTE,
        P_REF_6_6_SCALE,
        P_REF_6_6_DIST,
        
        P_REF_6_7_X,
        P_REF_6_7_Y,
        P_REF_6_7_Z,
        P_REF_6_7_MAT,
        P_REF_6_7_MUTE,
        P_REF_6_7_SCALE,
        P_REF_6_7_DIST,
        
        P_REF_6_8_X,
        P_REF_6_8_Y,
        P_REF_6_8_Z,
        P_REF_6_8_MAT,
        P_REF_6_8_MUTE,
        P_REF_6_8_SCALE,
        P_REF_6_8_DIST,
        
        P_REF_6_9_X,
        P_REF_6_9_Y,
        P_REF_6_9_Z,
        P_REF_6_9_MAT,
        P_REF_6_9_MUTE,
        P_REF_6_9_SCALE,
        P_REF_6_9_DIST,
        
        P_REF_6_10_X,
        P_REF_6_10_Y,
        P_REF_6_10_Z,
        P_REF_6_10_MAT,
        P_REF_6_10_MUTE,
        P_REF_6_10_SCALE,
        P_REF_6_10_DIST,
        
        P_REF_6_11_X,
        P_REF_6_11_Y,
        P_REF_6_11_Z,
        P_REF_6_11_MAT,
        P_REF_6_11_MUTE,
        P_REF_6_11_SCALE,
        P_REF_6_11_DIST,
        
        P_REF_6_12_X,
        P_REF_6_12_Y,
        P_REF_6_12_Z,
        P_REF_6_12_MAT,
        P_REF_6_12_MUTE,
        P_REF_6_12_SCALE,
        P_REF_6_12_DIST,
        
        P_REF_7_1_X, //9
        P_REF_7_1_Y,
        P_REF_7_1_Z,
        P_REF_7_1_MAT,
        P_REF_7_1_MUTE,
        P_REF_7_1_SCALE,
        P_REF_7_1_DIST,
        
        P_REF_7_2_X, //13
        P_REF_7_2_Y,
        P_REF_7_2_Z,
        P_REF_7_2_MAT,
        P_REF_7_2_MUTE,
        P_REF_7_2_SCALE,
        P_REF_7_2_DIST,
        
        P_REF_7_3_X,
        P_REF_7_3_Y,
        P_REF_7_3_Z,
        P_REF_7_3_MAT,
        P_REF_7_3_MUTE,
        P_REF_7_3_SCALE,
        P_REF_7_3_DIST,
        
        P_REF_7_4_X,
        P_REF_7_4_Y,
        P_REF_7_4_Z,
        P_REF_7_4_MAT,
        P_REF_7_4_MUTE,
        P_REF_7_4_SCALE,
        P_REF_7_4_DIST,
        
        P_REF_7_5_X,
        P_REF_7_5_Y,
        P_REF_7_5_Z,
        P_REF_7_5_MAT,
        P_REF_7_5_MUTE,
        P_REF_7_5_SCALE,
        P_REF_7_5_DIST,
        
        P_REF_7_6_X,
        P_REF_7_6_Y,
        P_REF_7_6_Z,
        P_REF_7_6_MAT,
        P_REF_7_6_MUTE,
        P_REF_7_6_SCALE,
        P_REF_7_6_DIST,
        
        P_REF_7_7_X,
        P_REF_7_7_Y,
        P_REF_7_7_Z,
        P_REF_7_7_MAT,
        P_REF_7_7_MUTE,
        P_REF_7_7_SCALE,
        P_REF_7_7_DIST,

        P_REF_7_8_X,
        P_REF_7_8_Y,
        P_REF_7_8_Z,
        P_REF_7_8_MAT,
        P_REF_7_8_MUTE,
        P_REF_7_8_SCALE,
        P_REF_7_8_DIST,
        
        P_REF_7_9_X,
        P_REF_7_9_Y,
        P_REF_7_9_Z,
        P_REF_7_9_MAT,
        P_REF_7_9_MUTE,
        P_REF_7_9_SCALE,
        P_REF_7_9_DIST,
        
        P_REF_7_10_X,
        P_REF_7_10_Y,
        P_REF_7_10_Z,
        P_REF_7_10_MAT,
        P_REF_7_10_MUTE,
        P_REF_7_10_SCALE,
        P_REF_7_10_DIST,
        
        P_REF_7_11_X,
        P_REF_7_11_Y,
        P_REF_7_11_Z,
        P_REF_7_11_MAT,
        P_REF_7_11_MUTE,
        P_REF_7_11_SCALE,
        P_REF_7_11_DIST,
        
        P_REF_7_12_X,
        P_REF_7_12_Y,
        P_REF_7_12_Z,
        P_REF_7_12_MAT,
        P_REF_7_12_MUTE,
        P_REF_7_12_SCALE,
        P_REF_7_12_DIST,
        
        P_REF_8_1_X, //9
        P_REF_8_1_Y,
        P_REF_8_1_Z,
        P_REF_8_1_MAT,
        P_REF_8_1_MUTE,
        P_REF_8_1_SCALE,
        P_REF_8_1_DIST,
        
        P_REF_8_2_X, //13
        P_REF_8_2_Y,
        P_REF_8_2_Z,
        P_REF_8_2_MAT,
        P_REF_8_2_MUTE,
        P_REF_8_2_SCALE,
        P_REF_8_2_DIST,

        P_REF_8_3_X,
        P_REF_8_3_Y,
        P_REF_8_3_Z,
        P_REF_8_3_MAT,
        P_REF_8_3_MUTE,
        P_REF_8_3_SCALE,
        P_REF_8_3_DIST,
        
        P_REF_8_4_X,
        P_REF_8_4_Y,
        P_REF_8_4_Z,
        P_REF_8_4_MAT,
        P_REF_8_4_MUTE,
        P_REF_8_4_SCALE,
        P_REF_8_4_DIST,
        
        P_REF_8_5_X,
        P_REF_8_5_Y,
        P_REF_8_5_Z,
        P_REF_8_5_MAT,
        P_REF_8_5_MUTE,
        P_REF_8_5_SCALE,
        P_REF_8_5_DIST,

        P_REF_8_6_X,
        P_REF_8_6_Y,
        P_REF_8_6_Z,
        P_REF_8_6_MAT,
        P_REF_8_6_MUTE,
        P_REF_8_6_SCALE,
        P_REF_8_6_DIST,
        
        P_REF_8_7_X,
        P_REF_8_7_Y,
        P_REF_8_7_Z,
        P_REF_8_7_MAT,
        P_REF_8_7_MUTE,
        P_REF_8_7_SCALE,
        P_REF_8_7_DIST,
        
        P_REF_8_8_X,
        P_REF_8_8_Y,
        P_REF_8_8_Z,
        P_REF_8_8_MAT,
        P_REF_8_8_MUTE,
        P_REF_8_8_SCALE,
        P_REF_8_8_DIST,
        
        P_REF_8_9_X,
        P_REF_8_9_Y,
        P_REF_8_9_Z,
        P_REF_8_9_MAT,
        P_REF_8_9_MUTE,
        P_REF_8_9_SCALE,
        P_REF_8_9_DIST,
        
        P_REF_8_10_X,
        P_REF_8_10_Y,
        P_REF_8_10_Z,
        P_REF_8_10_MAT,
        P_REF_8_10_MUTE,
        P_REF_8_10_SCALE,
        P_REF_8_10_DIST,
        
        P_REF_8_11_X,
        P_REF_8_11_Y,
        P_REF_8_11_Z,
        P_REF_8_11_MAT,
        P_REF_8_11_MUTE,
        P_REF_8_11_SCALE,
        P_REF_8_11_DIST,
        
        P_REF_8_12_X,
        P_REF_8_12_Y,
        P_REF_8_12_Z,
        P_REF_8_12_MAT,
        P_REF_8_12_MUTE,
        P_REF_8_12_SCALE,
        P_REF_8_12_DIST,

        P_REF_9_1_X, //9
        P_REF_9_1_Y,
        P_REF_9_1_Z,
        P_REF_9_1_MAT,
        P_REF_9_1_MUTE,
        P_REF_9_1_SCALE,
        P_REF_9_1_DIST,
        
        P_REF_9_2_X, //13
        P_REF_9_2_Y,
        P_REF_9_2_Z,
        P_REF_9_2_MAT,
        P_REF_9_2_MUTE,
        P_REF_9_2_SCALE,
        P_REF_9_2_DIST,
        
        P_REF_9_3_X,
        P_REF_9_3_Y,
        P_REF_9_3_Z,
        P_REF_9_3_MAT,
        P_REF_9_3_MUTE,
        P_REF_9_3_SCALE,
        P_REF_9_3_DIST,
        
        P_REF_9_4_X,
        P_REF_9_4_Y,
        P_REF_9_4_Z,
        P_REF_9_4_MAT,
        P_REF_9_4_MUTE,
        P_REF_9_4_SCALE,
        P_REF_9_4_DIST,
        
        P_REF_9_5_X,
        P_REF_9_5_Y,
        P_REF_9_5_Z,
        P_REF_9_5_MAT,
        P_REF_9_5_MUTE,
        P_REF_9_5_SCALE,
        P_REF_9_5_DIST,
        
        P_REF_9_6_X,
        P_REF_9_6_Y,
        P_REF_9_6_Z,
        P_REF_9_6_MAT,
        P_REF_9_6_MUTE,
        P_REF_9_6_SCALE,
        P_REF_9_6_DIST,
        
        P_REF_9_7_X,
        P_REF_9_7_Y,
        P_REF_9_7_Z,
        P_REF_9_7_MAT,
        P_REF_9_7_MUTE,
        P_REF_9_7_SCALE,
        P_REF_9_7_DIST,
        
        P_REF_9_8_X,
        P_REF_9_8_Y,
        P_REF_9_8_Z,
        P_REF_9_8_MAT,
        P_REF_9_8_MUTE,
        P_REF_9_8_SCALE,
        P_REF_9_8_DIST,
    
        P_REF_9_9_X,
        P_REF_9_9_Y,
        P_REF_9_9_Z,
        P_REF_9_9_MAT,
        P_REF_9_9_MUTE,
        P_REF_9_9_SCALE,
        P_REF_9_9_DIST,
        
        P_REF_9_10_X,
        P_REF_9_10_Y,
        P_REF_9_10_Z,
        P_REF_9_10_MAT,
        P_REF_9_10_MUTE,
        P_REF_9_10_SCALE,
        P_REF_9_10_DIST,
        
        P_REF_9_11_X,
        P_REF_9_11_Y,
        P_REF_9_11_Z,
        P_REF_9_11_MAT,
        P_REF_9_11_MUTE,
        P_REF_9_11_SCALE,
        P_REF_9_11_DIST,
        
        P_REF_9_12_X,
        P_REF_9_12_Y,
        P_REF_9_12_Z,
        P_REF_9_12_MAT,
        P_REF_9_12_MUTE,
        P_REF_9_12_SCALE,
        P_REF_9_12_DIST,
        
        P_REF_10_1_X, //9
        P_REF_10_1_Y,
        P_REF_10_1_Z,
        P_REF_10_1_MAT,
        P_REF_10_1_MUTE,
        P_REF_10_1_SCALE,
        P_REF_10_1_DIST,
        
        P_REF_10_2_X, //13
        P_REF_10_2_Y,
        P_REF_10_2_Z,
        P_REF_10_2_MAT,
        P_REF_10_2_MUTE,
        P_REF_10_2_SCALE,
        P_REF_10_2_DIST,
        
        P_REF_10_3_X,
        P_REF_10_3_Y,
        P_REF_10_3_Z,
        P_REF_10_3_MAT,
        P_REF_10_3_MUTE,
        P_REF_10_3_SCALE,
        P_REF_10_3_DIST,
        
        P_REF_10_4_X,
        P_REF_10_4_Y,
        P_REF_10_4_Z,
        P_REF_10_4_MAT,
        P_REF_10_4_MUTE,
        P_REF_10_4_SCALE,
        P_REF_10_4_DIST,
        
        P_REF_10_5_X,
        P_REF_10_5_Y,
        P_REF_10_5_Z,
        P_REF_10_5_MAT,
        P_REF_10_5_MUTE,
        P_REF_10_5_SCALE,
        P_REF_10_5_DIST,
        
        P_REF_10_6_X,
        P_REF_10_6_Y,
        P_REF_10_6_Z,
        P_REF_10_6_MAT,
        P_REF_10_6_MUTE,
        P_REF_10_6_SCALE,
        P_REF_10_6_DIST,
        
        P_REF_10_7_X,
        P_REF_10_7_Y,
        P_REF_10_7_Z,
        P_REF_10_7_MAT,
        P_REF_10_7_MUTE,
        P_REF_10_7_SCALE,
        P_REF_10_7_DIST,
        
        P_REF_10_8_X,
        P_REF_10_8_Y,
        P_REF_10_8_Z,
        P_REF_10_8_MAT,
        P_REF_10_8_MUTE,
        P_REF_10_8_SCALE,
        P_REF_10_8_DIST,
        
        P_REF_10_9_X,
        P_REF_10_9_Y,
        P_REF_10_9_Z,
        P_REF_10_9_MAT,
        P_REF_10_9_MUTE,
        P_REF_10_9_SCALE,
        P_REF_10_9_DIST,
        
        P_REF_10_10_X,
        P_REF_10_10_Y,
        P_REF_10_10_Z,
        P_REF_10_10_MAT,
        P_REF_10_10_MUTE,
        P_REF_10_10_SCALE,
        P_REF_10_10_DIST,
        
        P_REF_10_11_X,
        P_REF_10_11_Y,
        P_REF_10_11_Z,
        P_REF_10_11_MAT,
        P_REF_10_11_MUTE,
        P_REF_10_11_SCALE,
        P_REF_10_11_DIST,
        
        P_REF_10_12_X,
        P_REF_10_12_Y,
        P_REF_10_12_Z,
        P_REF_10_12_MAT,
        P_REF_10_12_MUTE,
        P_REF_10_12_SCALE,
        P_REF_10_12_DIST,
        
        P_REF_11_1_X, //9
        P_REF_11_1_Y,
        P_REF_11_1_Z,
        P_REF_11_1_MAT,
        P_REF_11_1_MUTE,
        P_REF_11_1_SCALE,
        P_REF_11_1_DIST,
        
        P_REF_11_2_X, //13
        P_REF_11_2_Y,
        P_REF_11_2_Z,
        P_REF_11_2_MAT,
        P_REF_11_2_MUTE,
        P_REF_11_2_SCALE,
        P_REF_11_2_DIST,
        
        P_REF_11_3_X,
        P_REF_11_3_Y,
        P_REF_11_3_Z,
        P_REF_11_3_MAT,
        P_REF_11_3_MUTE,
        P_REF_11_3_SCALE,
        P_REF_11_3_DIST,
        
        P_REF_11_4_X,
        P_REF_11_4_Y,
        P_REF_11_4_Z,
        P_REF_11_4_MAT,
        P_REF_11_4_MUTE,
        P_REF_11_4_SCALE,
        P_REF_11_4_DIST,
        
        P_REF_11_5_X,
        P_REF_11_5_Y,
        P_REF_11_5_Z,
        P_REF_11_5_MAT,
        P_REF_11_5_MUTE,
        P_REF_11_5_SCALE,
        P_REF_11_5_DIST,
        
        P_REF_11_6_X,
        P_REF_11_6_Y,
        P_REF_11_6_Z,
        P_REF_11_6_MAT,
        P_REF_11_6_MUTE,
        P_REF_11_6_SCALE,
        P_REF_11_6_DIST,
        
        P_REF_11_7_X,
        P_REF_11_7_Y,
        P_REF_11_7_Z,
        P_REF_11_7_MAT,
        P_REF_11_7_MUTE,
        P_REF_11_7_SCALE,
        P_REF_11_7_DIST,
        
        P_REF_11_8_X,
        P_REF_11_8_Y,
        P_REF_11_8_Z,
        P_REF_11_8_MAT,
        P_REF_11_8_MUTE,
        P_REF_11_8_SCALE,
        P_REF_11_8_DIST,
        
        P_REF_11_9_X,
        P_REF_11_9_Y,
        P_REF_11_9_Z,
        P_REF_11_9_MAT,
        P_REF_11_9_MUTE,
        P_REF_11_9_SCALE,
        P_REF_11_9_DIST,
        
        P_REF_11_10_X,
        P_REF_11_10_Y,
        P_REF_11_10_Z,
        P_REF_11_10_MAT,
        P_REF_11_10_MUTE,
        P_REF_11_10_SCALE,
        P_REF_11_10_DIST,
        
        P_REF_11_11_X,
        P_REF_11_11_Y,
        P_REF_11_11_Z,
        P_REF_11_11_MAT,
        P_REF_11_11_MUTE,
        P_REF_11_11_SCALE,
        P_REF_11_11_DIST,
        
        P_REF_11_12_X,
        P_REF_11_12_Y,
        P_REF_11_12_Z,
        P_REF_11_12_MAT,
        P_REF_11_12_MUTE,
        P_REF_11_12_SCALE,
        P_REF_11_12_DIST,

        P_REF_12_1_X, //9
        P_REF_12_1_Y,
        P_REF_12_1_Z,
        P_REF_12_1_MAT,
        P_REF_12_1_MUTE,
        P_REF_12_1_SCALE,
        P_REF_12_1_DIST,
        
        P_REF_12_2_X, //13
        P_REF_12_2_Y,
        P_REF_12_2_Z,
        P_REF_12_2_MAT,
        P_REF_12_2_MUTE,
        P_REF_12_2_SCALE,
        P_REF_12_2_DIST,
        
        P_REF_12_3_X,
        P_REF_12_3_Y,
        P_REF_12_3_Z,
        P_REF_12_3_MAT,
        P_REF_12_3_MUTE,
        P_REF_12_3_SCALE,
        P_REF_12_3_DIST,
        
        P_REF_12_4_X,
        P_REF_12_4_Y,
        P_REF_12_4_Z,
        P_REF_12_4_MAT,
        P_REF_12_4_MUTE,
        P_REF_12_4_SCALE,
        P_REF_12_4_DIST,
        
        P_REF_12_5_X,
        P_REF_12_5_Y,
        P_REF_12_5_Z,
        P_REF_12_5_MAT,
        P_REF_12_5_MUTE,
        P_REF_12_5_SCALE,
        P_REF_12_5_DIST,
        
        P_REF_12_6_X,
        P_REF_12_6_Y,
        P_REF_12_6_Z,
        P_REF_12_6_MAT,
        P_REF_12_6_MUTE,
        P_REF_12_6_SCALE,
        P_REF_12_6_DIST,
        
        P_REF_12_7_X,
        P_REF_12_7_Y,
        P_REF_12_7_Z,
        P_REF_12_7_MAT,
        P_REF_12_7_MUTE,
        P_REF_12_7_SCALE,
        P_REF_12_7_DIST,
        
        P_REF_12_8_X,
        P_REF_12_8_Y,
        P_REF_12_8_Z,
        P_REF_12_8_MAT,
        P_REF_12_8_MUTE,
        P_REF_12_8_SCALE,
        P_REF_12_8_DIST,
        
        P_REF_12_9_X,
        P_REF_12_9_Y,
        P_REF_12_9_Z,
        P_REF_12_9_MAT,
        P_REF_12_9_MUTE,
        P_REF_12_9_SCALE,
        P_REF_12_9_DIST,
        
        P_REF_12_10_X,
        P_REF_12_10_Y,
        P_REF_12_10_Z,
        P_REF_12_10_MAT,
        P_REF_12_10_MUTE,
        P_REF_12_10_SCALE,
        P_REF_12_10_DIST,
        
        P_REF_12_11_X,
        P_REF_12_11_Y,
        P_REF_12_11_Z,
        P_REF_12_11_MAT,
        P_REF_12_11_MUTE,
        P_REF_12_11_SCALE,
        P_REF_12_11_DIST,
        
        P_REF_12_12_X,
        P_REF_12_12_Y,
        P_REF_12_12_Z,
        P_REF_12_12_MAT,
        P_REF_12_12_MUTE,
        P_REF_12_12_SCALE,
        P_REF_12_12_DIST,
        
        P_NUM
    };
    
    static int reflectionOffset = P_REF_1_1_X;

    struct EffectData
    {
        float p[P_NUM]; // Parameters
        float input[VAS_MAXVECTORSIZE];
        float outputL[VAS_MAXVECTORSIZE];
        float outputR[VAS_MAXVECTORSIZE];
        float lastReflectionInput[VAS_MAXVECTORSIZE];
        float tmp[VAS_MAXVECTORSIZE];
        
        int reflectionOrder = 1;
        int numberOfRays = 6;
        int config = 1;
        int init = 0;
        int initReverbTail = 0;
        
        vas_fir_binaural *binauralEngine;
        vas_fir_reverb *reverbEngine;
        vas_iir_biquad *directivityDamping;
        vas_fir_binauralReflection *reflections[VAS_MAXNUMBEROFRAYS][VAS_MAXREFLECTIONORDER];
        
        char debugString[512];
    };
    
    extern "C"
    {
        static void *currentInstance[100];
        int instanceCounter = 0;
        extern vas_fir_list IRs;
    
        /*union {
            struct
            {
                float        _11, _12, _13, _14;
                float        _21, _22, _23, _24;
                float        _31, _32, _33, _34;
                float        _41, _42, _43, _44;
            };
            float m[4][4];
            float m2[16];
        };

        void GetRotation(float& Yaw, float& Pitch, float& Roll) const
        {
            if (_11 == 1.0f)
            {
                Yaw = atan2f(_13, _34);
                Pitch = 0;
                Roll = 0;

            }else if (_11 == -1.0f)
            {
                Yaw = atan2f(_13, _34);
                Pitch = 0;
                Roll = 0;
            }else
            {

                Yaw = atan2(-_31,_11);
                Pitch = asin(_21);
                Roll = atan2(-_23,_22);
            }
        }
         If matrix has size, n by m [i.e. i goes from 0 to (n-1) and j from 0 to (m-1) ], then:

         matrix[ i ][ j ] = array[ i*m + j ].
         
         */
    
        void HeadingAndElevationFromTranformationMatrix(float *matrix, float *heading, float *elevation)
        {
            if (matrix[0] == 1.0f)
            {
                *heading = vas_utilities_radians2degrees(atan2f(matrix[2], matrix[11]));
                *elevation = vas_utilities_radians2degrees(atan2f(-matrix[6],matrix[5]+0.001f));
            }
            else if (matrix[0]  == -1.0f)
            {
                *heading = vas_utilities_radians2degrees(atan2f(matrix[2], matrix[11]));
                *elevation = vas_utilities_radians2degrees(atan2f(-matrix[6],matrix[5]+0.001f));
            }
            else
            {
                float radians = atan2(matrix[8], matrix[0]);
                radians += M_PI;
                    
                *heading = vas_utilities_radians2degrees(radians);
                *elevation = vas_utilities_radians2degrees(atan2f(-matrix[6],matrix[5]+0.001f));
            }
        }
        
		VAS_EXPORT void SetDebugFunction( FuncPtr fp )
        {
            Debug = fp;
        }
        
		VAS_EXPORT void *GetInstance(int id)
        {
#ifdef VAS_DEBUG_TO_UNITY
            if(Debug != NULL)
                Debug("Trying to find Renderer");
#endif
            int i=0;
            while(i<100)
            {
                if(currentInstance[i])
                {
                    EffectData *current = static_cast<EffectData *>(currentInstance[i]);
                    if( current->p[3] == id)
                    {
#ifdef VAS_DEBUG_TO_UNITY
                        if(Debug != NULL)
                            Debug("Found Renderer");
#endif
                        return current;
                        
                    }
                }
                i++;
            }
            return NULL;
        }
    
		VAS_EXPORT void SetConfig(EffectData *effectData, int config)
        {
            effectData->config = config;
        }
    
        void readIR(vas_fir *x, char *fullpath, int segmentSize, int offset)
        {
            const char *fileExtension;
            int end = 0;
            fileExtension = vas_util_getFileExtension(fullpath);
            if(!strcmp(fileExtension, "sofa"))
            {
#ifdef VAS_USE_LIBMYSOFA
                void *filter = vas_fir_readSofa_getMetaData(x, fullpath);
                if(filter)
                {
                    vas_fir_initFilter1((vas_fir *)x, segmentSize);
                    vas_fir_readSofa_getFilter(x, filter);
                    vas_fir_setInitFlag((vas_fir *)x);
                }
#else
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug("Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA");
#endif
#endif
            }
            if(!strcmp(fileExtension, "txt"))
            {
                vas_fir *existingFilter = vas_fir_list_find(&IRs, fullpath);
                //vas_fir *existingFilter = vas_fir_list_find1(&IRs, fullpath, segmentSize, offset, end);
                 
                if(existingFilter != NULL)
                {
#ifdef VAS_DEBUG_TO_UNITY
                    if(Debug != NULL)
                    {
                        Debug("Use existing filter");
                        Debug(existingFilter->metaData.fullPath);
                        
                        char tmp[64];
                        sprintf(tmp, "search for: %d %d %d", segmentSize, offset, end);
                        Debug(tmp);
                        
                        sprintf(tmp, "existing is: %d %d %d", existingFilter->metaData.segmentSize, existingFilter->metaData.filterOffset, existingFilter->metaData.filterEnd);
                        Debug(tmp);
                    }
#endif
                    size_t size = strlen(existingFilter->metaData.fullPath);
                    x->metaData.fullPath = (char *)vas_mem_alloc(sizeof(char) * size);
                    strcpy(x->metaData.fullPath, existingFilter->metaData.fullPath);
                    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
                    vas_fir_setInitFlag((vas_fir *)x);
                    return;
                }
                
                else
                {
#ifdef VAS_DEBUG_TO_UNITY
                    if(Debug != NULL)
                        Debug("Read text file");
#endif
                    FILE *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
                    if(file)
                    {
                        ((vas_fir *)x)->metaData.filterOffset = offset;
                        ((vas_fir *)x)->metaData.segmentSize = segmentSize;
                        if(end > 0 && (end < ((vas_fir *)x)->metaData.filterLength) )
                            ((vas_fir *)x)->metaData.filterLength = end;
                   
                        vas_fir_initFilter2((vas_fir *)x, segmentSize);
                        vas_fir_readText_Ir1((vas_fir *)x, file, offset);
                        vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
                        vas_fir_setInitFlag((vas_fir *)x);
                    }
#ifdef VAS_DEBUG_TO_UNITY
                    else
                    {
                        if(Debug != NULL)
                            Debug("Error: Could not read file");
                    }
#endif
                }
            }
        }
        
		VAS_EXPORT void LoadHRTF(EffectData *effectData, char *fullpath)
        {
            vas_fir *x;
            if(effectData != NULL)
            {
                int segmentSize = effectData->p[P_SEGMENTSIZE_EARLYPART];
                x = (vas_fir *)effectData->binauralEngine;
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug(fullpath);
#endif
                readIR(x, fullpath, segmentSize, 0);
                    
                if(effectData->config == VAS_SPAT_CONFIG_AUTO)
                {
                    for(int i = 0; i < effectData->numberOfRays; i++)
                    {
                        for (int j = 0; j < effectData->reflectionOrder; j ++)
                        {
                            effectData->reflections[i][j] = vas_fir_binauralReflection_new(effectData->binauralEngine, 100000);
                            vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x, effectData->reflections[i][j]->binauralEngine->left, effectData->reflections[i][j]->binauralEngine->right);
                            vas_fir_setInitFlag((vas_fir *)effectData->reflections[i][j]->binauralEngine);
                        }
                    }
                }
                effectData->init = 1;
            }
            else
            {
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug("Could not find Instance");
#endif
            }
        }
    
		VAS_EXPORT void LoadReverbTail(EffectData *effectData, char *fullpath)
        {
            vas_fir *x;
            if(effectData != NULL)
            {
                int segmentSize = effectData->p[P_SEGMENTSIZE_LATEPART];
                int offset = segmentSize - effectData->p[P_SEGMENTSIZE_EARLYPART];
                x = (vas_fir *)effectData->reverbEngine;
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug(fullpath);
#endif
                
                readIR(x, fullpath, segmentSize, offset);
                effectData->initReverbTail = 1;
            }
            else
            {
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug("Could not find Instance");
#endif
            }
        }
    }

    inline bool IsHostCompatible(UnityAudioEffectState* state)
    {
        // Somewhat convoluted error checking here because hostapiversion is only supported from SDK version 1.03 (i.e. Unity 5.2) and onwards.
        // Since we are only checking for version 0x010300 here, we can't use newer fields in the UnityAudioSpatializerData struct, such as minDistance and maxDistance.
        return
            state->structsize >= sizeof(UnityAudioEffectState) &&
            state->hostapiversion >= 0x010300;
    }

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        //char name[12];
       // char description[30];
        
        int paramNumber;
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        RegisterParameter(definition, "AudioSrc Attn", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, P_AUDIOSRCATTN, "AudioSource distance attenuation");
        RegisterParameter(definition, "Fixed Volume", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_FIXEDVOLUME, "Fixed volume amount");
        RegisterParameter(definition, "Custom Falloff", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_CUSTOMFALLOFF, "Custom volume falloff amount (logarithmic)");
        RegisterParameter(definition, "Spat Id", "", -1.0f, 100.0f, -1.0f, 1.0f, 1.0f, P_SPATID, "Spat id");
        RegisterParameter(definition, "HS Dir", "", 0.0f, 360, 80.0f, 1.0f, 1.0f, P_H_SOURCEDIRECTIVITY, "Horizontal Source Directivity");
        RegisterParameter(definition, "HF Po Range", "", 0.0f, 360, 40.0f, 1.0f, 1.0f, P_H_FULLPOWERRANGE, "Horizontal Full Power Range");
        RegisterParameter(definition, "VS Dir", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_SOURCEDIRECTIVITY, "Vertical Source Directivity");
        RegisterParameter(definition, "VF Po Range", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_FULLPOWERRANGE, "Vertical Full Power Range");
        RegisterParameter(definition, "HDir Damping", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_H_DIRECTIVITYDAMPING, "Horizontal Directivity Damping");
        RegisterParameter(definition, "VDir Damping", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_V_DIRECTIVITYDAMPING, "Vertical Directivity Damping");
        RegisterParameter(definition, "No of rays", "", 0.0f, 6.0f, 0.0f, 1.0f, 1.0f, P_NUMBEROFRAYS, "Number of rays");
        RegisterParameter(definition, "Ref order", "", 0.0f, 10.0f, 0.0f, 1.0f, 1.0f, P_REFLECTIONORDER, "Reflection order");
        RegisterParameter(definition, "Inverse Azi", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_INVERSEAZI, "Inverse Azimuth");
        RegisterParameter(definition, "Inverse Ele", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_INVERSEELE, "Inverse Elevation");
        RegisterParameter(definition, "Bypass", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_BYPASS, "Bypass");
        RegisterParameter(definition, "Lister Only", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_LISTENERORIENTATIONONLY, "Listener orientation only");
        RegisterParameter(definition, "SegSizeEarly", "", 0.0f, 4096.0f, 1024.0f, 1.0f, 1.0f, P_SEGMENTSIZE_EARLYPART, "Segment size early reverb");
        RegisterParameter(definition, "SegSizeLate", "", 0.0f, 4096.0f, 1024.0f, 1.0f, 1.0f, P_SEGMENTSIZE_LATEPART, "Segment size late reverb");
        RegisterParameter(definition, "Dist Scaling", "", 0.0f, 10000.0f, 1.0f, 1.0f, 1.0f, P_DISTANCE_SCALING, "Scale Roomsize");
        RegisterParameter(definition, "Occlusion", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_OCCLUSION, "Occlusion");

        for(int i = 0; i < VAS_MAXNUMBEROFRAYS; i++)
        {
            for (int j = 0; j < VAS_MAXREFLECTIONORDER; j ++)
            {
                //sprintf(name, "ray%d%dX", i, j);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS + j * VAS_REFLECTIONPARAMETERS;
                RegisterParameter(definition, "ray", "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                //sprintf(name, "ray%d%dY", i, j);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS +j * VAS_REFLECTIONPARAMETERS + 1;
                RegisterParameter(definition, "ray", "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                //sprintf(name, "ray%d%dZ", i, j);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS +j * VAS_REFLECTIONPARAMETERS + 2;
                RegisterParameter(definition, "ray", "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS +j * VAS_REFLECTIONPARAMETERS + 3; // Material
                RegisterParameter(definition, "ray", "", 0.0f, 10.f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS +j * VAS_REFLECTIONPARAMETERS + 4; // Mute
                RegisterParameter(definition, "ray", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS +j * VAS_REFLECTIONPARAMETERS + 5; // Scaling
                RegisterParameter(definition, "ray", "", 0.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS +j * VAS_REFLECTIONPARAMETERS + 6; // Distance
                RegisterParameter(definition, "ray", "", 0.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
            }
        }
        
        definition.flags |= UnityAudioEffectDefinitionFlags_IsSpatializer;
        return numparams;
    }

    static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK DistanceAttenuationCallback(UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        //*attenuationOut = attenuationIn;
           // data->p[P_AUDIOSRCATTN] * attenuationIn +
           // data->p[P_FIXEDVOLUME] +
           // data->p[P_CUSTOMFALLOFF] * (1.0f / FastMax(1.0f, distanceIn));
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        EffectData* effectdata = new EffectData;
        memset(effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;
        
        effectdata->binauralEngine = vas_fir_binaural_new(0);
        effectdata->reverbEngine = vas_fir_binaural_new(0);
        effectdata->numberOfRays = 0;
        effectdata->reflectionOrder = 0;
        effectdata->directivityDamping = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWPASS, 20000, 10);
        currentInstance[instanceCounter++] = effectdata;
        
        if (IsHostCompatible(state))
            ;//  state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;
        
        for(int i = 0;i < VAS_MAXVECTORSIZE; i++)
        {
            effectdata->input[i] = 0.0;
            effectdata->outputL[i] = 0.0;
            effectdata->outputR[i] = 0.0;
            effectdata->lastReflectionInput[i] = 0.0;
            effectdata->tmp[i] = 0.0;
        }
        
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        
        if(instanceCounter == 1)
        {
#ifdef VAS_DEBUG_TO_UNITY
            if(Debug != NULL)
                Debug("Cleared filter list");
#endif
             vas_fir_list_clear(&IRs);
        }
          
        data->init = 0;
        
        vas_fir_binaural_free(data->binauralEngine);
        vas_fir_binaural_free(data->reverbEngine);
        vas_iir_biquad_free(data->directivityDamping);
 
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            for(int i = 0; i < data->numberOfRays; i++)
            {
                for (int j = 0; j < data->reflectionOrder; j ++)
                    vas_fir_binauralReflection_free(data->reflections[i][j]);
            }
        }
        
        instanceCounter--;
        
        delete data;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
            
        data->p[index] = value;
        
        if(index == P_SPATID)
        {
#ifdef VAS_DEBUG_TO_UNITY
            if(Debug != NULL)
                Debug("Set Spat Id");
#endif
        }
        if(index == P_NUMBEROFRAYS)
            data->numberOfRays = data->p[index];
        if(index == P_REFLECTIONORDER)
            data->reflectionOrder = data->p[index];
        
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char *valuestr)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        if (value != NULL)
            *value = data->p[index];
        if (valuestr != NULL)
            valuestr[0] = 0;
         
        return UNITY_AUDIODSP_OK;
    }

    int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
    {
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int outchannels)
    {
        if (inchannels != 2 || outchannels != 2 || state->spatializerdata == NULL)
        {
            memset(outbuffer, 0, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }
        
        EffectData* data = state->GetEffectData<EffectData>();
        if(!data->init || data->p[P_BYPASS])
        {
            memset(outbuffer, 0, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }

        static const float kRad2Deg = 180.0f / kPI;
        float azimuth, elevation;

        float* listener = state->spatializerdata->listenermatrix;
        float* source = state->spatializerdata->sourcematrix;
        float px, py, pz;
        float dir_x, dir_y, dir_z;
     
        if(data->p[P_LISTENERORIENTATIONONLY])
           HeadingAndElevationFromTranformationMatrix(listener, &azimuth, &elevation);
        
        else
        {
            px = source[12];
            py = source[13];
            pz = source[14];
            
            dir_x = listener[0] * px + listener[4] * py + listener[8] * pz + listener[12];
            dir_y = listener[1] * px + listener[5] * py + listener[9] * pz + listener[13];
            dir_z = listener[2] * px + listener[6] * py + listener[10] * pz + listener[14];
            
            azimuth = (fabsf(dir_z) < 0.001f) ? 0.0f : atan2f(dir_x, dir_z);
            if (azimuth < 0.0f)
                azimuth += 2.0f * kPI;
            azimuth = FastClip(azimuth * kRad2Deg, 0.0f, 360.0f);
            elevation = atan2f(dir_y, sqrtf(dir_x * dir_x + dir_z * dir_z) + 0.001f) * kRad2Deg;
        }
        
        for(unsigned int n = 0; n < length; n++)
        {
            data->input[n] = inbuffer[n * inchannels]; // last reflectionInput deleted
        }
     
        if(data->p[P_INVERSEAZI])
            azimuth = 360 - azimuth;
        
        vas_fir_binaural_setAzimuth(data->binauralEngine, azimuth);
        vas_fir_binaural_setElevation(data->binauralEngine, elevation);
        
        float r_hDirectivityDamping = data->p[P_H_DIRECTIVITYDAMPING];
        float r_vDirectivityDamping = data->p[P_V_DIRECTIVITYDAMPING];
        
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            vas_iir_biquad_setFrequency(data->directivityDamping, r_hDirectivityDamping * r_vDirectivityDamping * 20000);
            vas_iir_biquad_process(data->directivityDamping, data->input, data->input, length);
        }
        
        vas_fir_binaural_process(data->binauralEngine, data->input, data->outputL, data->outputR, length);
        if(data->initReverbTail)
            vas_fir_binaural_processOutputInPlace(data->reverbEngine, data->input, data->outputL, data->outputR, length); // input was lastReflectionInput
        
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            float r_px;
            float r_py;
            float r_pz;
            float r_scaling;
            float r_distance;
            int material;
            int mute;
            int paramIndex;
            
            for(int i = 0; i < data->numberOfRays; i++)
            {
                for(int n = 0; n < length; n++)
                    data->lastReflectionInput[n] = inbuffer[n * inchannels];
                
                for (int j = 0; j < data->reflectionOrder; j ++)
                {
                    paramIndex = reflectionOffset + VAS_MAXNUMBEROFRAYS * i * VAS_REFLECTIONPARAMETERS + j * VAS_REFLECTIONPARAMETERS;
                    r_px = data->p[paramIndex];
                    r_py = data->p[paramIndex+1];
                    r_pz = data->p[paramIndex+2];
                    material = (int)data->p[paramIndex+3];
                    mute =  data->p[paramIndex+4];
                    r_scaling = data->p[paramIndex+5];
                    r_distance = data->p[paramIndex+6];
                    
//                    if(Debug)
//                    {
//                        if(mute)
//                        {
//                            sprintf(data->debugString, "%d %d", paramIndex, mute);
//                            Debug(data->debugString);
//                        }
//                    }
                     
                    //distance = sqrt(pow(r_px-px, 2) + pow(r_py-py, 2) + pow(r_pz-pz, 2)); // bullshit... need to set distance as parameter
                    float delayTime = r_distance/343.0 * 44100.0;
                      
                    vas_fir_binauralReflection_setScaling(data->reflections[i][j], r_scaling);
                    vas_fir_binauralReflection_setMaterial(data->reflections[i][j], material); // for a very simplyfied material characteristics
                    vas_fir_binauralReflection_setDelayTime(data->reflections[i][j], delayTime); // these should be called from game loop, not within the dsp loop
                    
                    /*if(Debug)
                    {
                        sprintf(data->debugString, "%d %d %f",i, j, delayTime);
                        Debug(data->debugString);
                    }*/
                     
                    if(!mute)
                    {
                        dir_x = listener[0] * r_px + listener[4] * r_py + listener[8] * r_pz + listener[12];
                        dir_y = listener[1] * r_px + listener[5] * r_py + listener[9] * r_pz + listener[13];
                        dir_z = listener[2] * r_px + listener[6] * r_py + listener[10] * r_pz + listener[14];
                         
                        azimuth = (fabsf(dir_z) < 0.001f) ? 0.0f : atan2f(dir_x, dir_z);
                        if (azimuth < 0.0f)
                            azimuth += 2.0f * kPI;
                        azimuth = FastClip(azimuth * kRad2Deg, 0.0f, 360.0f);
                        if(data->p[P_INVERSEAZI])
                            azimuth = 360 - azimuth;
                         
                        elevation = atan2f(dir_y, sqrtf(dir_x * dir_x + dir_z * dir_z) + 0.001f) * kRad2Deg;
                        
                        vas_fir_binauralReflection_setAzimuth(data->reflections[i][j], azimuth);
                        vas_fir_binauralReflection_setElevation(data->reflections[i][j], elevation);
                        vas_fir_binauralReflection_process(data->reflections[i][j], data->lastReflectionInput, data->outputL, data->outputR, length);
                    }
                    else
                    {
                        vas_fir_binauralReflection_process_mute(data->reflections[i][j], data->lastReflectionInput, length);
                    }
                }
            }
        }
        
        for(unsigned int n = 0; n < length; n++)
        {
            outbuffer[n * outchannels] = data->outputL[n];
            outbuffer[n * outchannels + 1] = data->outputR[n];
        }
 
        return UNITY_AUDIODSP_OK;
    }
}




