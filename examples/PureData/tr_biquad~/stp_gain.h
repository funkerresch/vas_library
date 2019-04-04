/**
 * @file stp_gain.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * A very advanced gain plugin <br>
 * <br>
 * @brief Audio Object for adjusting the volume<br>
 * <br>
 * stp_gain allows for adjusting the level<br>
 * of any incoming audio signal. <br>
 * <br>
 */

#ifndef stp_gain_h
#define stp_gain_h

#include <stdio.h>
#include <stdlib.h>
#include "stp_defines.h"


/** Defines whether the input vector should be single or double precision */
#define STP_INPUTVECTOR_USEFLOAT
/** Defines whether the output vector should be single or double precision */
#define STP_OUTPUTVECTOR_USEFLOAT

/**
 * @struct stp_gain
 * @brief A structure for a gain object <br>
 * @var stp::level The parameter value for adjusting the <br>
 * level of the incoming signal
 */

typedef struct stp_gain
{
    float level; /**< parameter for adjusting the level of the incoming signal */
} stp_gain;

/**
 * @related stp_gain
 * @brief Creates a new gain object<br>
 * The function sets the level parameter of <br>
 * the gain class
 * @return a pointer to the newly created stp_gain object <br>
 */

stp_gain *stp_gain_new();

/**
 * @related stp_gain
 * @brief Frees a gain object<br>
 * @param x My gain object <br>
 * The function frees the allocated memory<br>
 * of a gain object
 */

void stp_gain_free(stp_gain *x);

/**
 * @related stp_gain
 * @brief Sets the gain parameter <br>
 * @param x My gain object <br>
 * @param level The gain value <br>
 * The function sets the level parameter of <br>
 * the gain class
 */

void stp_gain_setLevel(stp_gain *x, float level);

/**
 * @related stp_gain
 * @brief Performs the level adjustment in realtime <br>
 * @param x My gain object <br>
 * @param in The input vector <br>
 * @param out The output vector <br>
 * @param vectorSize The vectorSize <br>
 * The function stp_gain_perform performs the gain adjustment of <br>
 * the incoming signal and copies the result to the output vector <br>
 */

void stp_gain_perform(stp_gain *x, STP_INPUTVECTOR *in, STP_OUTPUTVECTOR *out, int vectorSize);

#endif /* stp_gain_h */
