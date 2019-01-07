//
//  unused.c
//  ak.binaural~
//
//  Created by Admin on 30.11.17.
//
/*
void ak_binauralFilter_dd_process_static(ak_vaTools *x, double *in, double *out, int vectorSize)
{
    float *pointerToFFTInput = x->input.copy;
    float *pointerToOutputSegment = x->output.outputSegment;
    int segmentSize = x->segmentSize;
    int n = vectorSize;
    
    while (n--)
        *pointerToFFTInput++ = *in++; // copy current signal input vector to fftInputSignalFloat
    
    ak_vaTools_fcopy_vDSP(x->segmentSize, x->input.copy, x->tmp); // zeropadding
    vDSP_ctoz ( ( COMPLEX * ) x->tmp, 2, x->input.pointerToFFTSegments[x->movingIndex], 1, segmentSize  );
    vDSP_fft_zrip ( x->setupReal, x->input.pointerToFFTSegments[x->movingIndex], 1, x->fftSizeLog2, FFT_FORWARD  );
    
    ak_vaTools_fadd_vDSP(x->segmentSize, x->output.current.overlap, x->output.current.signalFloat, x->output.outputSegment);
    
    vDSP_fft_zrip ( x->setupReal, &x->output.current.signalComplex, 1, x->fftSizeLog2, FFT_INVERSE  );
    vDSP_ztoc ( &x->output.current.signalComplex, 1, ( COMPLEX * ) (x->output.current.signalFloat), 2, segmentSize );
    
    ak_vaTools_fadd_vDSP(segmentSize, x->output.current.overlap, x->output.current.signalFloat, x->output.outputSegment);
    n = vectorSize;
    
    while (n--)
        *out++ = *pointerToOutputSegment++; // copy current output vector from outputSegment to out
    
    x->movingIndex +=1;
    if (x->movingIndex == x->pointerArraySize)
        x->movingIndex = x->pointerArrayMiddle;
}*/

/*void ak_binaural_process_timeDomain(ak_vaTools *x, double *in, double *out, int vectorSize)
 {
     int n = vectorSize;
     int i = 0;
 
     float *pointerToInputBuffer = x->inputSignal.copy;
     float inSample;
     float convSum; // to accumulate the sum during convolution.
 
     while(n--)
     {
         convSum = 0;
         inSample = *(in++);
 
         pointerToInputBuffer[x->bufferPin] = inSample;
 
         for (i = 0; i < vectorSize; i++)
         convSum += hrtfLeft[x->elevation][x->azimuth][i]  * pointerToInputBuffer[(x->bufferPin - i) &255];
 
 
         x->bufferPin = (x->bufferPin + 1) & 255;
 
         *out++ = convSum;
     }
 }
 
 void ak_binaural_dd_process_timeDomain_vDSP(ak_vaTools *x, double *in, double *out, int vectorSize)
 {
     int n = vectorSize;
 
     float inSample;
     float convSum;
 
     while(n--)
     {
         convSum = 0;
         inSample = *(in++);
 
         x->convBuffer[511-x->bufferPin] = inSample;
         x->convBuffer[255-x->bufferPin] = inSample;
 
         vDSP_vmul(hrtfLeft[x->elevation][x->azimuth], 1, &x->convBuffer[256-x->bufferPin], 1, x->convSumArrayL, 1, 256);
         vDSP_sve(x->convSumArrayL, 1, &convSum, 256);
 
         x->bufferPin = (x->bufferPin + 1) & 255;
 
         *out++ = convSum;
     }
 }*/
