#include "vas_interpolDelay.h"vas_interpolDelay *vas_interpolDelay_new(long maxDelay){	vas_interpolDelay *x = (vas_interpolDelay *) vas_mem_alloc(sizeof(vas_interpolDelay));	x->maxDelay = maxDelay+1;	x->delayTime = 0;
    x->targetDelayTime = 0;	x->delayBuffer = ( float *) vas_mem_alloc ( x->maxDelay * sizeof ( float ) );	x->inPtr = 0;	x->outPtr = 0;	x->doNextOut = 1;	x->alpha = 0;	x->omAlpha = 0;	x->nextOutput = 0;	x->output = 0;		return x;}

void vas_interpolDelay_setDelayTime(vas_interpolDelay *x, long delayTime)
{
    x->targetDelayTime = delayTime;
}void vas_interpolDelay_approachNewDelayTime(vas_interpolDelay *x){	float outPointerTmp;
    if(x->delayTime < x->targetDelayTime)
        x->delayTime ++;
    if(x->delayTime > x->targetDelayTime)
        x->delayTime--;	if(x->delayTime > x->maxDelay-1)		x->delayTime = x->maxDelay-1;
    	outPointerTmp = x->inPtr - x->delayTime;	while (outPointerTmp < 0)		outPointerTmp += x->maxDelay;	x->outPtr = (long) outPointerTmp;	if (x->outPtr == x->maxDelay)		x->outPtr = 0;	x->alpha = outPointerTmp - x->outPtr;	x->omAlpha = (float) 1.0 - x->alpha;}float vas_interpolDelay_nextOut(vas_interpolDelay *x){	if (x->doNextOut)	{		x->nextOutput = x->delayBuffer[x->outPtr] * x->omAlpha;		if (x->outPtr+1 < x->maxDelay)			x->nextOutput += x->delayBuffer[x->outPtr+1] * x->alpha;		else			x->nextOutput += x->delayBuffer[0] * x->alpha;		x->doNextOut = 0;		}		return x->nextOutput;}void vas_interpolDelay_free(vas_interpolDelay *x){	vas_mem_free(x->delayBuffer);	vas_mem_free(x);}

void vas_interpolDelay_perform1(vas_interpolDelay *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize)
{
    int n = 0;
    
    if(x->delayTime != x->targetDelayTime)
        vas_interpolDelay_approachNewDelayTime(x);
    
    while(n++ < vectorSize)
    {
        x->delayBuffer[x->inPtr++] = *in++;
        if (x->inPtr == x->maxDelay)
            x->inPtr = 0;
        x->output = vas_interpolDelay_nextOut(x);
        x->doNextOut = 1;
        
        if (++x->outPtr == x->maxDelay)
            x->outPtr = 0;
        
        *out++ = x->output;
    }
}

float vas_interpolDelay_perform(vas_interpolDelay *x, float input)
{
	x->delayBuffer[x->inPtr++] = input;
	if (x->inPtr == x->maxDelay)
		x->inPtr = 0;
	x->output = vas_interpolDelay_nextOut(x);
	x->doNextOut = 1;
	
	if (++x->outPtr == x->maxDelay)
		x->outPtr = 0;
	return x->output;
}float vas_interpolDelay_lastValue(vas_interpolDelay *x){	return x->output;}void vas_interpolDelay_resize(vas_interpolDelay *x, long maxDelay){	x->maxDelay = maxDelay;	//x->delayBuffer = ( float *) sysmem_newptrclear ( x->maxDelay * sizeof ( float  ) );}	