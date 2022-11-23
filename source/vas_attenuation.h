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
 * @var minDistance <br>
 * @var maxDistance<br>
 */
typedef struct vas_attenuation
{
    float distance;
    float attenuationFactor;
    float minDistance;
    float maxDistance;
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

void vas_attenuation_setMinDistance(vas_attenuation *x, float minDistance);

void vas_attenuation_setMaxDistance(vas_attenuation *x, float maxDistance);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_attenuation_h */
