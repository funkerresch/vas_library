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
    x->minDistance = VAS_SMALLESTDISTANCE;
    x->distance = 1;
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
    float stretchFactor = 1/1.7;
    x->distance = MAX(distance, VAS_SMALLESTDISTANCE);
    x->attenuationFactor = (1/powf(x->distance, stretchFactor)) * (1-(x->distance/x->maxDistance));
    if(x->attenuationFactor < 0)
        x->attenuationFactor = 0;
    
    //if(x->distance > x->maxDistance)
        //x->attenuationFactor = 0;
   // else if(x->distance <= x->minDistance)
        //x->attenuationFactor = 1;
    //else
        //x->attenuationFactor = 1 / ( (x->distance-x->minDistance) * 250 / (x->maxDistance-x->minDistance) );
}

void vas_attenuation_setMinDistance(vas_attenuation *x, float minDistance)
{
    x->minDistance = minDistance;
    if(x->minDistance <= VAS_SMALLESTDISTANCE)
        x->minDistance = VAS_SMALLESTDISTANCE;
}

void vas_attenuation_setMaxDistance(vas_attenuation *x, float maxDistance)
{
    x->maxDistance = maxDistance;
    if(x->maxDistance < x->minDistance+1)
        x->maxDistance = x->minDistance+1;
}
