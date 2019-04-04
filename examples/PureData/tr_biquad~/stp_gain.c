#include "stp_gain.h"

stp_gain *stp_gain_new()
{
    stp_gain *x = (stp_gain *)malloc(sizeof(stp_gain));
    x->level = 0.5;
    return x;
}

void stp_gain_free(stp_gain *x)
{
    free(x);
}

void stp_gain_setLevel(stp_gain *x, float level)
{
    x->level = level;
}

void stp_gain_perform(stp_gain *x, STP_INPUTVECTOR *in, STP_OUTPUTVECTOR *out, int vectorSize)
{
    int i = 0;
    while(i < vectorSize)
    {
        *out++ = *in++ * x->level;
        i++;
        //printf("%f", *out);
       
    }
}
