//
//  vas_airAbsorption.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 19.01.21.
//
/*
 airAbsorption: korrekte Gain-Dämpfung gehört auch dazu! bei allen Reflexionen machen!
 */

#ifndef vas_airAbsorption_h
#define vas_airAbsorption_h

#include <stdio.h>
#include "vas_util.h"
#include "vas_mem.h"

#ifndef T0
#define T0 293.15f
#endif

#ifndef T01
#define T01 273.16f
#endif

#ifndef P0
#define P0 1
#endif

#ifndef P0_PASCAL
#define P0_PASCAL 101325.0
#endif

#ifndef H0
#define H0 50
#endif

#ifndef VAS_AIRABS_MIN_T
#define VAS_AIRABS_MIN_T 0
#endif

#ifndef VAS_AIRABS_MAX_T
#define VAS_AIRABS_MAX_T 330
#endif

#ifndef VAS_AIRABS_MIN_H
#define VAS_AIRABS_MIN_H 0
#endif

#ifndef VAS_AIRABS_MAX_H
#define VAS_AIRABS_MAX_H 100
#endif

#ifndef VAS_AIRABS_MIN_P
#define VAS_AIRABS_MIN_P 0
#endif

#ifndef VAS_AIRABS_MAX_P
#define VAS_AIRABS_MAX_P 2
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_airAbsorbtionFilter {
    bool damp;
    bool attenuate;
    float p; ///pressure
    float h; ///humidity
    float t; ///temperature
    float d; ///distance;
    float scaling; ///this is the distance at which attenuation starts; and at the stame time the scaling factor for the rolloff curve;
    float f_O; //oxygen relaxation frequency
    float f_N; //nitrogen relaxation frequency
    float sampleRate; //TODOHB1
    void* filter_guts; //points to a vas_airAbsorptionFIR or a vas_airAbsorptionSimple
} vas_airAbsorptionFilter;

/**
 * \brief Update the parameters of the air absorption filter.
 *
 * @param x A pointer to a vas_airAbsorptionFilter struct.
 * @param t Temperature in Kelvin.
 * @param h Humidity in percent.
 * @param p Pressure in atm.
 * @param d Distance in meters.
 * @param scaling The scaling to be applied (1.0 is real physical attenuation, larger values result in slower volume decrease per distance)
 * @param shouldAttenuate Toggle volume decrease over distance on/off.
 * @param shouldDamp Toggle high-frequency damping over distance on/off.
 *
 */
void vas_airAbsorptionFilter_update(vas_airAbsorptionFilter *x, float t, float h, float p, float d, float scaling, float shouldAttenuate, float shouldDamp);

/**
 * \brief Allocates a vas_airAbsorptionFilter struct and returns a pointer to it.
 *
 * @param sampleRate The audio sampleRate the filter should operate on (e.g. 44100 or 48000).
 *
 * @return A pointer to a vas_airAbsorptionFilter struct.
 */
vas_airAbsorptionFilter *vas_airAbsorptionFilter_new(float sampleRate);

/**
 * \brief Frees a vas_airAbsorptionFilter struct.
 */
void vas_airAbsorbtionFilter_free(vas_airAbsorptionFilter *x);

/**
 * \brief Process an input vector and copy the results to an output vector. Input vector and output vector can be identical.
 *
 * @param x A pointer to a vas_airAbsorptionFilter struct.
 * @param input The input vector.
 * @param output The output vector.
 * @param vectorSize The number of elements to process.
 *
 */
void vas_airAbsorbtionFilter_perform(vas_airAbsorptionFilter *x, VAS_INPUTBUFFER* input, VAS_OUTPUTBUFFER* output, int vectorSize);

#ifdef __cplusplus
}
#endif

#endif /* vas_airAbsorption_h */
