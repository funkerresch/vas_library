//
//  vas_fir_fixedFirChannel.c
//  vas_partconv~
//
//  Created by Harvey Keitel on 26.04.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//
#include "vas_dynamicFirChannel.h"

void vas_fixedFirChannel_calculateConvolution(vas_dynamicFirChannel *x)
{
    vas_util_fcopy(x->output.current.signalFloat+x->filter->segmentSize, x->output.current.overlap, x->filter->segmentSize);
     
    if(!x->useSharedInput)
        vas_dynamicFirChannel_forwardFFTInput(x);
      
    vas_dynamicFirChannel_multiplyAddSegments(x, &x->output.current);
    vas_dynamicFirChannel_inverseFFT(x, &x->output.current);
    vas_util_fadd(x->output.current.overlap, x->output.current.signalFloat, x->output.outputSegment, x->filter->segmentSize);
       
    x->movingIndex +=1;
    if (x->movingIndex == x->pointerArraySize)
        x->movingIndex = x->pointerArrayMiddle;
    
    return;
}

void *doWork(void *args)
{
    vas_fir_partitioned *x = args;
    while(1)
    {
        pthread_cond_wait( &x->waitForWorkCondition[0], &x->waitForWorkMutex[0] );
        
        post("Do Work");
        pthread_mutex_lock(&x->waitForWorkMutex[0]);
    }
}

void vas_fixedFirChannel_process(vas_dynamicFirChannel *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize, int flags, void *data)
{
    float *pointerToFFTInput;
    float *pointerToOutputSegment;
    int n;
    int offset;
    int vsOverSegmentSize = vectorSize/x->filter->segmentSize;
    int frameCounter = 0;

    if(!x->init)
    {
        n = vectorSize;
        while (n--)
            *out++ = 0.0;
        return;
    }
    
    if(!vsOverSegmentSize) // segment size > vector size
    {
        int segmentSizeOverVs = x->filter->segmentSize/vectorSize;
        pointerToFFTInput = x->input->copy;
        pointerToOutputSegment = x->output.outputSegment;
        
        if(!x->useSharedInput)
        {
            pointerToFFTInput+= x->frameCounter * vectorSize;
            vas_util_fcopy(in, pointerToFFTInput, vectorSize);
        }
    
        pointerToOutputSegment+= x->frameCounter * vectorSize;
        
        if(flags & VAS_OUTPUTVECTOR_ADDINPLACE)
            vas_util_fadd(out, pointerToOutputSegment, out, vectorSize);
        else
            vas_util_fcopy(pointerToOutputSegment, out, vectorSize);
        
        x->frameCounter++;
        if(x->frameCounter == segmentSizeOverVs)
        {
            x->frameCounter = 0;
            vas_dynamicFirChannel_calculateConvolution(x);
        }
    }
    
    else
    {
        while(frameCounter < vsOverSegmentSize) //  segment size <= vector size
        {
            n = x->filter->segmentSize;
            offset = frameCounter * n;
            pointerToFFTInput = x->input->copy;
            pointerToOutputSegment = x->output.outputSegment;
            
            if(!x->useSharedInput)
                vas_util_fcopy(in + offset, pointerToFFTInput, n);
            
            vas_dynamicFirChannel_calculateConvolution(x);
            
            if(flags & VAS_OUTPUTVECTOR_ADDINPLACE)
                vas_util_fadd(out + offset, pointerToOutputSegment, out + offset, n);
            else
                vas_util_fcopy(pointerToOutputSegment, out + offset, n);
            
            frameCounter++;
        }
    }
}
