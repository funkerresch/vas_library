/**
 * @file vas_attenuation.h
 * @author Thomas Resch<br>
 * @brief Min and Max Distance Air Attenuation.<br>
 * <br>
 * vas_attenuation performs signal attenuation <br>
 * with a given min and max distance<br>
 * <br>
 */

#ifndef vas_attenuation_h
#define vas_attenuation_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vas_mem.h"
#include "vas_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VAS_SMALLESTDISTANCE 0.0001
    
/**
 * @struct vas_attenuation
 * @brief A structure for an attenuation object <br>
 * @param distance The current distance. <br>
 * @param attenuationFactor  <br>
 * @param minAttenuation <br>
 * @param maxAttenuation<br>
 * @maxDistance<br>
 * @stretchFactor<br>
 *
 */
typedef struct vas_attenuation
{
    float distance;
    float attenuationFactor;
    float minAttenuation;
    float maxAttenuation;
    float maxDistance;
    float stretchFactor;
} vas_attenuation;

/**
 * @related vas_attenuation
 * @brief Creates a new delay object<br>
 * The function sets the buffer size and delay parameter of <br>
 * the delay class
 * @return a pointer to the newly created vas_attenuation object <br>
 */
vas_attenuation *vas_attenuation_new(long maxDistance);

/**
 * @related vas_attenuation
 * @brief Frees a delay object<br>
 * @param x My delay object <br>
 * The function frees the allocated memory<br>
 * of a delay object
 */
void vas_attenuation_free(vas_attenuation *x);

/**
 * @related vas_attenuation
 * @brief Performs attenuation in real time. <br>
 * @param x The attenuation object <br>
 * @param in The input vector <br>
 * @param out The output vector <br>
 * @param vector_size The size of the i/o vectors <br>
 * The function vas_attenuation_perform calculates the
 * signal attenuation based on the distance and the minDistance
 * and maxDistance parameters<br>
 */

void vas_attenuation_perform(vas_attenuation *x, float *in, float *out, int vector_size);

void vas_attenuation_setDistance(vas_attenuation *x, float distance);

void vas_attenuation_setMaxDistance(vas_attenuation *x, float maxDistance);

void vas_attenuation_setMaxAttenuation(vas_attenuation *x, float maxAttenuation);

void vas_attenuation_setMinAttenuation(vas_attenuation *x, float minAttenuation);

/**
 * @related vas_attenuation
 * @brief Sets the steepness of the curve. <br>
 * @param x The attenuation object <br>
 * @param steepness The steepness of the curve .<br>
 * The function sets the steepness of the attenuation curve. <br>
 * a value of 1 results (if max distance is big enough) in the <br>
 * physical correct 1/r attenuation. A value of 10 results almost <br>
 * in a linear damping function.<br>
 */

void vas_attenuation_setSteepness(vas_attenuation *x, float steepness);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_attenuation_h */
