//
//  vas_attenuation.c
//  Created by Thomas Resch on 21.11.22.
//  Copyright © 2022 Intrinsic Audio. All rights reserved.
//

#include "vas_attenuation.h"

vas_attenuation *vas_attenuation_new(long maxDistance)
{
    vas_attenuation *x = (vas_attenuation *)malloc(sizeof(vas_attenuation));
    x->maxDistance = maxDistance;
    x->minAttenuation = 0.0f;
    x->maxAttenuation = 1.0f;
    x->distance = 1;
    x->stretchFactor = 1.0f/1.7f;
    return x;
}

void vas_attenuation_free(vas_attenuation *x)
{
    vas_mem_free(x);
}

void vas_attenuation_perform(vas_attenuation *x, float *in, float *out, int vectorSize)
{
    if(in != out)
        vas_util_fcopy(in, out, vectorSize);
    
    vas_util_fscale(out, x->attenuationFactor, vectorSize);
}

void vas_attenuation_setDistance(vas_attenuation *x, float distance)
{
    //x->distance = MAX(distance, VAS_SMALLESTDISTANCE);
    x->attenuationFactor = (1/powf(x->distance, x->stretchFactor)) * (1-(x->distance/x->maxDistance));
    if(x->attenuationFactor < x->minAttenuation)
        x->attenuationFactor = x->minAttenuation;
    
    if(x->attenuationFactor >= x->maxAttenuation)
        x->attenuationFactor = x->maxAttenuation;
}

void vas_attenuation_setMaxAttenuation(vas_attenuation *x, float maxAttenuation)
{
    x->maxAttenuation = maxAttenuation;
    
    if(x->maxAttenuation >= 1.0)
        x->maxAttenuation = 1.0f;
    
    if(x->maxAttenuation <= x->minAttenuation)
        x->maxAttenuation = x->minAttenuation;
}

void vas_attenuation_setMinAttenuation(vas_attenuation *x, float minAttenuation)
{
    x->minAttenuation = minAttenuation;
    if(x->minAttenuation <= 0)
        x->minAttenuation = 0;
    
    if(x->minAttenuation >= x->maxAttenuation)
        x->minAttenuation = x->maxAttenuation;
}

void vas_attenuation_setMaxDistance(vas_attenuation *x, float maxDistance)
{
    x->maxDistance = maxDistance;
}

void vas_attenuation_setSteepness(vas_attenuation *x, float steepness)
{
    if(steepness > 0)
        x->stretchFactor = 1.0f/steepness;
}
