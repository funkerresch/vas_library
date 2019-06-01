#include "vas_dynamicFirChannel.h"
//#include <pthread.h>

vas_dynamicFirChannel_filter globalFilterLeft = { .init = 0, .referenceCounter = 0};
vas_dynamicFirChannel_filter globalFilterRight = { .init = 0, .referenceCounter = 0};
vas_dynamicFirChannel_config vas_binauralSetup_std = { .aziRange = 120, .aziStride = 3, .eleMin = -90, .eleMax = 90, .eleRange = 60, .eleStride = 3, .eleZero = 30};
vas_dynamicFirChannel_config vas_binauralSetup_noEle = { .aziRange = 120, .aziStride = 3, .eleMin = 0, .eleMax = 1, .eleRange = 1, .eleStride = 1, .eleZero = 0};
vas_dynamicFirChannel_config vas_staticSetup = { .aziRange = 1, .aziStride = 1, .eleMin = 0, .eleMax = 1, .eleRange = 1, .eleStride = 1, .eleZero = 0};



void vas_dynamicFirChannel_filter_init(vas_dynamicFirChannel_filter *x, int segmentSize)
{
    x->segmentSize = segmentSize;
    x->fftSize = segmentSize * 2;
    x->fftSizeLog2 = log2(x->fftSize);
    x->numberOfSegments = 0;
    
    for(int eleCount = 0; eleCount < x->firSetup->eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < x->firSetup->aziRange; aziCount++)
        {
            x->averageSegmentPower[eleCount][aziCount]  = ( double * ) vas_mem_alloc(sizeof(double));
            x->overallEnergy[eleCount][aziCount] = 0;
            x->real[eleCount][aziCount] = ( float ** ) vas_mem_alloc((sizeof(float *)));
            x->imag[eleCount][aziCount] = ( float ** ) vas_mem_alloc((sizeof(float *)));
            x->pointerToFFTSegments[eleCount][aziCount]  = (VAS_COMPLEX **) vas_mem_alloc(sizeof(VAS_COMPLEX *));
            x->data[eleCount][aziCount]  = ( VAS_COMPLEX * ) vas_mem_alloc((sizeof(VAS_COMPLEX)));
            x->segmentIsZero[eleCount][aziCount] = ( bool* ) vas_mem_alloc((sizeof(bool)));
        }
    }
}

void vas_dynamicFirChannel_filter_free(vas_dynamicFirChannel_filter *x)
{
#ifdef VAS_USE_VDSP
    
    vDSP_destroy_fftsetup(x->setupReal);
    
#endif
    
    for(int eleCount = 0; eleCount < x->firSetup->eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < x->firSetup->aziRange; aziCount++)
        {
#ifdef VAS_USE_VDSP
            for(int i = 0; i < x->numberOfSegments; i++)
            {
                
                if(!x->segmentIsZero[eleCount][aziCount][i])
                {
                   // post("FREE: %d %d", aziCount, i);
                    vas_mem_free(x->real[eleCount][aziCount][i]);
                    vas_mem_free(x->imag[eleCount][aziCount][i]);
                }
            }
#endif
            vas_mem_free(x->averageSegmentPower[eleCount][aziCount]);
            vas_mem_free(x->segmentIsZero[eleCount][aziCount]);
            vas_mem_free(x->real[eleCount][aziCount]);
            vas_mem_free(x->imag[eleCount][aziCount]);
            vas_mem_free(x->pointerToFFTSegments[eleCount][aziCount]);
            vas_mem_free(x->data[eleCount][aziCount]);
        }
    }
}

void vas_dynamicFirChannel_input_init(vas_dynamicFirChannel_input *x)
{
    x->copy = (float *)vas_mem_alloc((sizeof(float)));
    x->real = ( float* ) vas_mem_alloc((sizeof(float)));
    x->imag = ( float* ) vas_mem_alloc((sizeof(float)));
    x->pointerToFFTSegments = (VAS_COMPLEX **) vas_mem_alloc(sizeof(VAS_COMPLEX *));
    x->data = ( VAS_COMPLEX * ) vas_mem_alloc((sizeof(VAS_COMPLEX)));
}

void vas_dynamicFirChannel_input_free(vas_dynamicFirChannel_input *x)
{
    vas_mem_free(x->real);
    vas_mem_free(x->imag);
    vas_mem_free(x->pointerToFFTSegments);
    vas_mem_free(x->data);
    vas_mem_free(x->copy);
}

void vas_dynamicFirChannel_output_init(vas_dynamicFirChannel_output *x, int elevationZero)
{
    x->current.azimuth = 0;
    x->next.azimuth = 0;
    x->current.elevation = elevationZero;
    x->next.elevation = elevationZero;
    x->current.signalFloat = (float *)vas_mem_alloc((sizeof(float)));
    x->next.signalFloat = (float *)vas_mem_alloc((sizeof(float)));
    x->outputSegment = (float *)vas_mem_alloc((sizeof(float)));
    x->current.overlap = (float *)vas_mem_alloc((sizeof(float)));
    x->next.overlap = (float *)vas_mem_alloc((sizeof(float)));
#ifdef VAS_USE_VDSP
    x->current.signalComplex.realp = ( float* ) vas_mem_alloc((sizeof(float)));
    x->current.signalComplex.imagp = ( float* ) vas_mem_alloc((sizeof(float)));
    x->next.signalComplex.realp = ( float* ) vas_mem_alloc((sizeof(float)));
    x->next.signalComplex.imagp = ( float* ) vas_mem_alloc((sizeof(float)));
#else
    x->current.signalComplex = ( VAS_COMPLEX * ) vas_mem_alloc((sizeof(VAS_COMPLEX)));
    x->next.signalComplex = ( VAS_COMPLEX * ) vas_mem_alloc((sizeof(VAS_COMPLEX)));
#endif
}

void vas_dynamicFirChannel_output_free(vas_dynamicFirChannel_output *x)
{
    vas_mem_free(x->current.signalFloat);
    vas_mem_free(x->next.signalFloat);
    vas_mem_free(x->outputSegment);
    vas_mem_free(x->current.overlap);
    vas_mem_free(x->next.overlap);
#ifdef VAS_USE_VDSP
    vas_mem_free(x->current.signalComplex.realp);
    vas_mem_free(x->current.signalComplex.imagp);
    vas_mem_free(x->next.signalComplex.realp);
    vas_mem_free(x->next.signalComplex.imagp);
#else
    vas_mem_free(x->current.signalComplex);
    vas_mem_free(x->next.signalComplex);
#endif
}

vas_dynamicFirChannel_config *vas_dynamicFirChannel_config_new()
{
    vas_dynamicFirChannel_config *x = vas_mem_alloc(sizeof(vas_dynamicFirChannel_config));
    x->aziRange = 120;
    x->aziStride = 3;
    x->eleMin = -90;
    x->eleMax = 90;
    x->eleRange = 60;
    x->eleStride = 3;
    x->eleZero = 90;
    return x;
}

void vas_dynamicFirChannel_config_set(vas_dynamicFirChannel_config *x, int eleMin, int eleMax, int eleStride, int aziStride)
{
    x->eleMin = eleMin;
    x->eleMax = eleMax;
    x->eleStride = eleStride;
    x->aziStride = aziStride;
    
    if(x->eleMin >= x->eleMax)
    {
        x->eleMin = 0;
        x->eleMax = 1;
    }
    
    if(!x->eleStride)
        x->eleStride = 1;
    
    x->eleRange = ceil((abs(x->eleMin) + abs(x->eleMax)) / x->eleStride) ;
    x->eleZero = abs(x->eleMin) / x->eleStride;
    
    if(x->aziStride != 0)
        x->aziRange = ceil(360 / x->aziStride);
    else
        x->aziRange = 1;
    
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
    post("Elevation Array Range: %d", x->eleRange);
    post("Elevation Array Zero: %d", x->eleZero);
    post("Azimuth Array Range: %d", x->aziRange);
#endif
#endif
}

void vas_dynamicFirChannel_config_free(vas_dynamicFirChannel_config *x)
{
    vas_mem_free(x);
}

void vas_dynamicFirChannel_setAzimuth(vas_dynamicFirChannel *x, int azimuth)
{
    int azi = azimuth;
    if(azi < 0)
        azi = 360 - azi;
    
    azi = azi % 360;
    x->azimuthTmp = azi/x->filter->firSetup->aziStride;
}

void vas_dynamicFirChannel_setSegmentThreshold(vas_dynamicFirChannel *x, float thresh)
{
    if(thresh >= 0 && thresh < 1)
        x->segmentThreshold = thresh;
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post("%.20f", x->segmentThreshold);
#endif
}

void vas_dynamicFirChannel_setElevation(vas_dynamicFirChannel *x, int elevation)
{
    if(elevation >= x->filter->firSetup->eleMin && elevation < x->filter->firSetup->eleMax)
        x->elevationTmp = elevation/x->filter->firSetup->eleStride+x->filter->firSetup->eleZero;
}

void vas_dynamicFirChannel_setSegmentSize(vas_dynamicFirChannel *x, int segmentSize)
{
    if(!vas_utilities_isValidSegmentSize(segmentSize))
    {
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Set Segment Size");
#endif
        
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Invalid Segment Size: %d", segmentSize);
#else
        printf("Invalid Segment Size");
#endif
        return;
    }
    
    x->filter->segmentSize = segmentSize;
    x->filter->fftSize = x->filter->segmentSize*2;
    x->scale = 1.0 / (x->filter->segmentSize * x->filter->segmentSize);
    x->filter->fftSizeLog2 = log2(x->filter->fftSize);
#ifdef VAS_USE_VDSP
    x->filter->setupReal = vDSP_create_fftsetup ( x->filter->fftSizeLog2, FFT_RADIX2);
#endif
    
#ifdef VAS_USE_KISSFFT
    x->forwardFFT = kiss_fftr_alloc(x->filter->fftSize,0,0,0);
    x->inverseFFT = kiss_fftr_alloc(x->filter->fftSize,1,0,0);
    
#endif
    
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
    post("Segment Size is: %d", x->filter->segmentSize);
    post("FFT Size is: %d", x->filter->fftSize);
    post("FFT Size Log2 is: %d", x->filter->fftSizeLog2);
#else
    printf("Segment Size is: %d", x->filter->segmentSize);
    printf("FFT Size is: %d", x->filter->fftSize);
    printf("FFT Size Log2 is: %d", x->filter->fftSizeLog2);
#endif
#endif
}

void vas_dynamicFirChannel_setAllInputSegments2Zero(vas_dynamicFirChannel *x)
{
    vas_utilities_writeZeros(x->filter->numberOfSegments * x->filter->segmentSize, x->input.real);
    vas_utilities_writeZeros(x->filter->numberOfSegments * x->filter->segmentSize, x->input.imag);
    vas_utilities_writeZeros(x->filter->fftSize, x->output.current.signalFloat);
    vas_utilities_writeZeros(x->filter->fftSize, x->output.next.signalFloat);
    vas_utilities_writeZeros(x->filter->segmentSize, x->output.outputSegment);
    vas_utilities_writeZeros(x->filter->segmentSize, x->output.current.overlap);
    vas_utilities_writeZeros(x->filter->segmentSize, x->output.next.overlap);
}

void vas_dynamicFirChannel_multiplyAddSegments(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target)
{
    x->segmentIndex = 0;
#ifdef VAS_USE_VDSP

    vas_util_complexWriteZeros(&target->signalComplex, x->filter->segmentSize);
    
    while(x->segmentIndex < x->filter->numberOfSegments)
    {
        if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex])
            vas_util_complexMultiplyAdd(x->input.pointerToFFTSegments[x->segmentIndex], x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex], &target->signalComplex, x->filter->segmentSize);
        
        x->segmentIndex++;
    }
    
#else
    vas_util_complexWriteZeros(target->signalComplex, x->filter->segmentSize);
    
    while(x->segmentIndex < x->filter->numberOfSegments)
    {
        if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex])
        {
            vas_util_complexMultiplyAdd(x->input.pointerToFFTSegments[x->segmentIndex], x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex], target->signalComplex, x->filter->segmentSize);
        }

        x->segmentIndex++;
    }
#endif
}

void vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter(vas_dynamicFirChannel *x)
{
    if(x->fadeCounter)
        vas_util_fadd(x->output.next.overlap, x->output.next.signalFloat, x->output.next.signalFloat, x->filter->segmentSize);
    
    vas_util_fmultiply(x->fadeOut+x->fadeCounter*x->filter->segmentSize, x->output.outputSegment, x->output.outputSegment, x->filter->segmentSize);
    vas_util_fmultiply(x->fadeIn+x->fadeCounter*x->filter->segmentSize, x->output.next.signalFloat, x->output.next.signalFloat, x->filter->segmentSize);
    vas_util_fadd(x->output.next.signalFloat, x->output.outputSegment, x->output.outputSegment, x->filter->segmentSize);
    
    x->fadeCounter++;
    
    if(x->fadeCounter == x->numberOfFramesForCrossfade)
    {
        x->fadeCounter = 0;
        vas_util_fcopy(x->output.next.signalFloat+x->filter->segmentSize, x->output.current.signalFloat+x->filter->segmentSize, x->filter->segmentSize);
    }
}

void vas_dynamicFirChannel_updateAzimuthAndElevation(vas_dynamicFirChannel *x)
{
    x->output.current.azimuth = x->output.next.azimuth;
    x->output.current.elevation = x->output.next.elevation;
    x->output.next.azimuth = x->azimuthTmp;
    x->output.next.elevation = x->elevationTmp;
}

void vas_dynamicFirChannel_forwardFFTInput(vas_dynamicFirChannel *x)
{
#ifdef VAS_USE_VDSP
    vDSP_ctoz ( ( COMPLEX * ) x->input.copy, 2, x->input.pointerToFFTSegments[x->movingIndex], 1, x->filter->segmentSize  );
    vDSP_fft_zrip ( x->filter->setupReal, x->input.pointerToFFTSegments[x->movingIndex], 1, x->filter->fftSizeLog2, FFT_FORWARD  );
#endif
    
#ifdef VAS_USE_FFTW
    fftwf_execute_dft_r2c(x->forwardFFT, x->input.copy, x->input.pointerToFFTSegments[x->movingIndex]);
#endif
    
#ifdef VAS_USE_KISSFFT
    kiss_fftr(x->forwardFFT,x->input.copy,x->input.pointerToFFTSegments[x->movingIndex]);
#endif
}

void vas_dynamicFirChannel_inverseFFT(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target)
{
    
#ifdef VAS_USE_VDSP
    vDSP_fft_zrip ( x->filter->setupReal, &target->signalComplex, 1, x->filter->fftSizeLog2, FFT_INVERSE  );
    vDSP_ztoc ( &target->signalComplex, 1, ( COMPLEX * ) (target->signalFloat), 2, x->filter->segmentSize );
#endif
    
#ifdef VAS_USE_FFTW
    fftwf_execute_dft_c2r(x->inverseFFT, target->signalComplex, target->signalFloat);
#endif
    
#ifdef VAS_USE_KISSFFT
    kiss_fftri(x->inverseFFT, target->signalComplex, target->signalFloat);
#endif
}

void vas_dynamicFirChannel_calculateConvolution(vas_dynamicFirChannel *x)
{
    float *pointerToFFTInput;
    float *pointerToOutputSegment;
    
    pointerToFFTInput = x->input.copy;
    pointerToOutputSegment = x->output.outputSegment;
   
    if(!x->fadeCounter)
        vas_dynamicFirChannel_updateAzimuthAndElevation(x);
    
    vas_util_fcopy(x->output.current.signalFloat+x->filter->segmentSize, x->output.current.overlap, x->filter->segmentSize);
    
    if(!x->useSharedInput)
    {
        vas_dynamicFirChannel_forwardFFTInput(x);
    }
   
    vas_dynamicFirChannel_multiplyAddSegments(x, &x->output.current);
    
    vas_dynamicFirChannel_inverseFFT(x, &x->output.current);
    
    vas_util_fadd(x->output.current.overlap, x->output.current.signalFloat, x->output.outputSegment, x->filter->segmentSize);
    
    if(x->output.current.azimuth != x->output.next.azimuth || x->output.current.elevation != x->output.next.elevation)
    {
        if(x->fadeCounter)
            vas_util_fcopy(x->output.next.signalFloat+x->filter->segmentSize, x->output.next.overlap, x->filter->segmentSize);
        
        vas_dynamicFirChannel_multiplyAddSegments(x, &x->output.next);
        
        vas_dynamicFirChannel_inverseFFT(x, &x->output.next);
        
        vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter(x);
    }
    
    x->movingIndex +=1;
    if (x->movingIndex == x->pointerArraySize)
        x->movingIndex = x->pointerArrayMiddle;
    
    return;
}

void vas_dynamicFirChannel_process(vas_dynamicFirChannel *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize, int flags)
{
    float *pointerToFFTInput;
    float *pointerToOutputSegment;
    int n;
    int vsOverSegmentSize = vectorSize/x->filter->segmentSize;
    int frameCounter = 0;
    
    if(!x->filter->init)
    {
        n = vectorSize;
        while (n--)
            *out++ = 0.0;
        return;
    }
    
    if(!vsOverSegmentSize) // < segment size > vector size 
    {
        int segmentSizeOverVs = x->filter->segmentSize/vectorSize;
        pointerToFFTInput = x->input.copy;
        pointerToOutputSegment = x->output.outputSegment;
        n = vectorSize;
        
        pointerToFFTInput+= x->frameCounter * vectorSize;
        while (n--)
            *pointerToFFTInput++ = *in++; // copy current signal input vector to fftInputSignalFloat
    
        n = vectorSize;
    
        pointerToOutputSegment+= x->frameCounter * vectorSize;
    
        while (n--)
        {
            if(flags & VAS_OUTPUTVECTOR_ADDINPLACE)
                *out++ += *pointerToOutputSegment++;
            else
                *out++ = *pointerToOutputSegment++;
        }
        
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
            pointerToFFTInput = x->input.copy;
            pointerToOutputSegment = x->output.outputSegment;
            
            if(!x->useSharedInput)
            {
                while (n--)
                    *pointerToFFTInput++ = *in++; // copy current signal input vector to fftInputSignalFloat
            }
            
            vas_dynamicFirChannel_calculateConvolution(x);
            
            n = x->filter->segmentSize;
            while (n--)
            {
                if(flags & VAS_OUTPUTVECTOR_ADDINPLACE)
                    *out++ += *pointerToOutputSegment++;
                else
                    *out++ = *pointerToOutputSegment++;
            }
            
            frameCounter++;
        }
    }
}

void vas_dynamicFirChannel_prepareOutputSignal(vas_dynamicFirChannel *x)
{
    x->output.current.signalFloat = (float *)vas_mem_resize(x->output.current.signalFloat, sizeof(float) * x->filter->fftSize);
    x->output.next.signalFloat = (float *)vas_mem_resize(x->output.next.signalFloat, sizeof(float) * x->filter->fftSize);
    x->output.outputSegment = (float *)vas_mem_resize(x->output.outputSegment, sizeof(float) * x->filter->fftSize); // must be segmentsize, changed for debugging
    x->output.current.overlap = (float *)vas_mem_resize(x->output.current.overlap, sizeof(float) * x->filter->segmentSize);
    x->output.next.overlap = (float *)vas_mem_resize(x->output.next.overlap, sizeof(float) * x->filter->segmentSize);
#ifdef VAS_USE_VDSP
    x->output.current.signalComplex.realp = (float *)vas_mem_resize(x->output.current.signalComplex.realp, x->filter->segmentSize * sizeof ( float  ));
    x->output.current.signalComplex.imagp = (float *)vas_mem_resize(x->output.current.signalComplex.imagp, x->filter->segmentSize * sizeof ( float  ));
    x->output.next.signalComplex.realp = (float *)vas_mem_resize(x->output.next.signalComplex.realp, x->filter->segmentSize * sizeof ( float  ));
    x->output.next.signalComplex.imagp = (float *)vas_mem_resize(x->output.next.signalComplex.imagp, x->filter->segmentSize * sizeof ( float  ));
    vas_util_complexWriteZeros(&x->output.current.signalComplex, x->filter->segmentSize);
    vas_util_complexWriteZeros(&x->output.next.signalComplex, x->filter->segmentSize);
#else
    x->output.current.signalComplex = (VAS_COMPLEX *)vas_mem_resize(x->output.current.signalComplex, x->filter->segmentSize * sizeof ( VAS_COMPLEX  ));
    x->output.next.signalComplex = (VAS_COMPLEX *)vas_mem_resize(x->output.next.signalComplex, x->filter->segmentSize * sizeof ( VAS_COMPLEX  ));
#endif
    vas_utilities_writeZeros(x->filter->fftSize, x->output.current.signalFloat);
    vas_utilities_writeZeros(x->filter->fftSize, x->output.next.signalFloat);
    vas_utilities_writeZeros(x->filter->segmentSize, x->output.outputSegment);
    vas_utilities_writeZeros(x->filter->segmentSize, x->output.current.overlap);
    vas_utilities_writeZeros(x->filter->segmentSize, x->output.next.overlap);
    
}

void vas_dynamicFirChannel_prepareInputSignal(vas_dynamicFirChannel *x)
{
    if(!x->useSharedInput)
    {
        int i = 0;
        x->input.copy = (float *)vas_mem_resize(x->input.copy, sizeof(float) * x->filter->fftSize);
        vas_utilities_writeZeros(x->filter->fftSize, x->input.copy);
#ifdef VAS_USE_VDSP
        x->input.pointerToFFTSegments = (VAS_COMPLEX **) vas_mem_resize(x->input.pointerToFFTSegments, x->pointerArraySize * sizeof (VAS_COMPLEX *));
        x->input.data = ( VAS_COMPLEX * )vas_mem_resize(x->input.data, x->filter->numberOfSegments * sizeof (VAS_COMPLEX));
        x->input.real = (float *)vas_mem_resize(x->input.real, sizeof(float) * x->filter->numberOfSegments * x->filter->segmentSize);
        x->input.imag = (float *)vas_mem_resize(x->input.imag, sizeof(float) * x->filter->numberOfSegments * x->filter->segmentSize);
#else
        x->input.pointerToFFTSegments = (VAS_COMPLEX **) vas_mem_resize(x->input.pointerToFFTSegments, x->pointerArraySize * sizeof (VAS_COMPLEX *));
        x->input.data = ( VAS_COMPLEX * )vas_mem_resize(x->input.data, x->filter->numberOfSegments * x->filter->fftSize *sizeof (VAS_COMPLEX));
#endif
        while(i < x->filter->numberOfSegments) //create pointer array to signal
        {
#ifdef VAS_USE_VDSP
            x->input.data[i].realp = x->input.real+i*x->filter->segmentSize;
            x->input.data[i].imagp = x->input.imag+i*x->filter->segmentSize;
            x->input.pointerToFFTSegments[i+x->pointerArrayMiddle] = &x->input.data[i];
            x->input.pointerToFFTSegments[i] = &x->input.data[i];
#endif
            
#ifdef VAS_USE_FFTW
            x->input.pointerToFFTSegments[i+x->pointerArrayMiddle] = &x->input.data[i*x->filter->fftSize];
            x->input.pointerToFFTSegments[i] = &x->input.data[i*x->filter->fftSize];
            x->forwardFFT = fftwf_plan_dft_r2c_1d(x->filter->fftSize, x->input.copy, x->input.pointerToFFTSegments[x->movingIndex], FFTW_PATIENT);
            x->inverseFFT = fftwf_plan_dft_c2r_1d(x->filter->fftSize, x->input.pointerToFFTSegments[x->movingIndex], x->input.copy, FFTW_PATIENT);
#endif
            
#ifdef VAS_USE_KISSFFT
            x->input.pointerToFFTSegments[i+x->pointerArrayMiddle] = &x->input.data[i*x->filter->fftSize];
            x->input.pointerToFFTSegments[i] = &x->input.data[i*x->filter->fftSize];
#endif
            i++;
        }
    }
    else
        x->input.pointerToFFTSegments = x->sharedInput->pointerToFFTSegments;
}

void vas_dynamicFirChannel_setFilterSize(vas_dynamicFirChannel *x, int filterSize)
{
    x->filterSize = filterSize;
    if(x->filter->segmentSize > filterSize)
        x->filter->segmentSize = filterSize;
    
    vas_dynamicFirChannel_setSegmentSize(x, x->filter->segmentSize);
    //x->fadeLength = x->filter->segmentSize * 2;
    //if(x->fadeLength < 512)
     x->fadeLength = 1024;
    
    if(x->filter->segmentSize == 0)
    {
       // post("invalid segmentSize");
        return;
    }
    
    x->filter->numberOfSegments = x->filterSize/x->filter->segmentSize;
    if(x->filterSize%x->filter->segmentSize != 0)
        x->filter->numberOfSegments+=1;
#ifdef VERBOSE
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post("Number of Segments: %d", x->filter->numberOfSegments);
#else
    printf("Number of Segments: %d", x->filter->numberOfSegments);
#endif
#endif
    
    x->numberOfFramesForCrossfade = x->fadeLength/x->filter->segmentSize;
    x->fadeCounter = 0;
    x->pointerArrayMiddle = x->filter->numberOfSegments;
    x->pointerArraySize = x->filter->numberOfSegments*2;
    x->movingIndex = x->pointerArrayMiddle;
}

void vas_dynamicFirChannel_prepareArrays(vas_dynamicFirChannel *x)
{
    x->tmp = (float *)vas_mem_resize(x->tmp, sizeof(float) * x->filter->fftSize);
    vas_utilities_writeZeros(x->filter->fftSize, x->tmp);
    x->fadeOut = (float *)vas_mem_resize(x->fadeOut, sizeof(float) * x->fadeLength);
    x->fadeIn = (float *)vas_mem_resize(x->fadeIn, sizeof(float) * x->fadeLength);
    vas_utilities_writeFadeOutArray(x->fadeLength, x->fadeOut);
    vas_utilities_writeFadeInArray(x->fadeLength, x->fadeIn);
    vas_dynamicFirChannel_prepareInputSignal(x);
    vas_dynamicFirChannel_prepareOutputSignal(x);
}

void vas_dynmaicFirChannel_resetMinMaxAverageSegmentPower(vas_dynamicFirChannel *x, int ele, int azi)
{
    x->filter->minAverageSegmentPower[ele][azi] = 100000;
    x->filter->maxAverageSegmentPower[ele][azi] = -100000;
    x->filter->zeroCounter[ele][azi] = 0;
    x->filter->nonZeroCounter[ele][azi] = 0;
}

void vas_dynamicFirChannel_leaveActivePartitions(vas_dynamicFirChannel *x, int numberOfActivePartions)
{
    for(int eleCount = 0; eleCount < x->filter->firSetup->eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < x->filter->firSetup->aziRange; aziCount++)
        {
            int j = x->filter->nonZeroCounter[eleCount][aziCount];
            
            if(j > numberOfActivePartions)
            {
                while(j-- > numberOfActivePartions)
                {
                    int minIndex = -1;
                    double minAverageSegmentPower = 10000;
                    
                    for(int i=0; i<x->filter->numberOfSegments;i++)
                    {
                        if(!x->filter->segmentIsZero[eleCount][aziCount][i])
                        {
                            if(x->filter->averageSegmentPower[eleCount][aziCount][i] < minAverageSegmentPower)
                            {
                                minIndex = i;
                                minAverageSegmentPower = x->filter->averageSegmentPower[eleCount][aziCount][i] ;
                            }
                        }
                    }
                    
                    x->filter->segmentIsZero[eleCount][aziCount][minIndex] = true;
                    x->filter->segmentIsZero[eleCount][aziCount][minIndex+x->pointerArrayMiddle] = true;
                    x->filter->zeroCounter[eleCount][aziCount]++;
                    x->filter->nonZeroCounter[eleCount][aziCount]--;
                    if(aziCount == 0)
                    {
#if defined(MAXMSPSDK) || defined(PUREDATA)
                        post("Set Segment with Index %d to zero", minIndex);
                        post("Energy of this Segment is %.14f, Max Energy: %.14f, Overall: %.14f", x->filter->averageSegmentPower[eleCount][aziCount][minIndex], x->filter->maxAverageSegmentPower[eleCount][aziCount], x->filter->overallEnergy[eleCount][aziCount]);
                        post("Number of active partitions %d", x->filter->nonZeroCounter[eleCount][aziCount]);
#endif
                    }
                }
            }
            
            else
            {
                while(j++ < numberOfActivePartions)
                {
                    int maxIndex = -1;
                    double maxAverageSegmentPower = -10000;
                    
                    for(int i=0; i<x->filter->numberOfSegments;i++)
                    {
                        if(x->filter->segmentIsZero[eleCount][aziCount][i])
                        {
                            if(x->filter->averageSegmentPower[eleCount][aziCount][i] > maxAverageSegmentPower)
                            {
                                maxIndex = i;
                                maxAverageSegmentPower = x->filter->averageSegmentPower[eleCount][aziCount][i] ;
                            }
                        }
                    }
                    
                    x->filter->segmentIsZero[eleCount][aziCount][maxIndex] = false;
                    x->filter->segmentIsZero[eleCount][aziCount][maxIndex+x->pointerArrayMiddle] = false;
                    x->filter->zeroCounter[eleCount][aziCount]--;
                    x->filter->nonZeroCounter[eleCount][aziCount]++;
                    if(aziCount == 0)
                    {
#if defined(MAXMSPSDK) || defined(PUREDATA)
                        post("Set Segment with Index %d to zero", maxIndex);
                        post("Energy of this Segment is %.14f, Max Energy: %.14f, Overall: %.14f", x->filter->averageSegmentPower[eleCount][aziCount][maxIndex], x->filter->maxAverageSegmentPower[eleCount][aziCount], x->filter->overallEnergy[eleCount][aziCount]);
                        post("Number of active partitions %d", x->filter->nonZeroCounter[eleCount][aziCount]);
#endif
                    }
                }
            }
        }
    }
}

void vas_dynamicFirChannel_calculateAverageSegmentPower(vas_dynamicFirChannel *x, float *filter, int segmentNumber, int ele, int azi)
{
    double energy = 0;
    double averagePower = 0;
    for(int i = 0; i < x->filter->segmentSize; i++)
        energy += pow(filter[i], 2);
    
    averagePower = (double)energy / x->filter->segmentSize;
    x->filter->averageSegmentPower[ele][azi][segmentNumber] = energy;
    x->filter->overallEnergy[ele][azi] += energy;
    
    if(energy < x->filter->minAverageSegmentPower[ele][azi])
    {
        if(energy > 0)
            x->filter->minAverageSegmentPower[ele][azi] = energy;
    }
    
    if(energy > x->filter->maxAverageSegmentPower[ele][azi])
        x->filter->maxAverageSegmentPower[ele][azi] = energy;
}

bool vas_dynamicFirChannel_isFilterSegmentBelowThreshhold(vas_dynamicFirChannel *x, float *filter, int ele, int azi)
{
    double energy = 0;
    double averagePower = 0;
    for(int i = 0; i < x->filter->segmentSize; i++)
        energy += pow(filter[i], 2);
    
    averagePower = energy / x->filter->segmentSize;
    
    if(energy > x->segmentThreshold)
    {
        if(azi == 0)
            ;//post("segment is greater: %.16f %.16f", energy, x->segmentThreshold);
        return false;
    }
    else
    {
        //if(azi == 0)
           // post("segment is smaller: %.16f %.16f", energy, x->segmentThreshold);
        return true;
    }
}

void vas_dynamicFirChannel_prepareFilter(vas_dynamicFirChannel *x, float *filter, int ele, int azi)
{
    x->scale = 1.0 / (x->filter->fftSize);
    
    if(!x->sharedFilter)
    {
        int i = 0;
#ifdef VAS_USE_VDSP
        
        x->filter->averageSegmentPower[ele][azi] = (double *)vas_mem_resize(x->filter->averageSegmentPower[ele][azi], sizeof(double) * x->filter->numberOfSegments);
        x->filter->real[ele][azi] = (float **)vas_mem_resize(x->filter->real[ele][azi], sizeof(float *) * x->filter->numberOfSegments);
        x->filter->imag[ele][azi] = (float **)vas_mem_resize(x->filter->imag[ele][azi], sizeof(float *) * x->filter->numberOfSegments);
        x->filter->pointerToFFTSegments[ele][azi] = (VAS_COMPLEX **) vas_mem_resize(x->filter->pointerToFFTSegments[ele][azi], x->pointerArraySize * sizeof (VAS_COMPLEX *));
        x->filter->data[ele][azi] = ( VAS_COMPLEX * )vas_mem_resize(x->filter->data[ele][azi], x->filter->numberOfSegments * sizeof (VAS_COMPLEX));
#else
        x->filter->pointerToFFTSegments[ele][azi] = (VAS_COMPLEX **) vas_mem_resize(x->filter->pointerToFFTSegments[ele][azi], x->pointerArraySize * sizeof (VAS_COMPLEX *));
        x->filter->data[ele][azi] = ( VAS_COMPLEX * )vas_mem_resize(x->filter->data[ele][azi], x->filter->numberOfSegments * x->filter->fftSize * sizeof (VAS_COMPLEX));
#endif
        x->filter->segmentIsZero[ele][azi] = (bool *)vas_mem_resize(x->filter->segmentIsZero[ele][azi], sizeof(bool) * x->pointerArraySize);
        
        while(i < x->filter->numberOfSegments)         //create pointer array to filtersegments
        {
#ifdef VAS_USE_VDSP

            x->filter->real[ele][azi][i] = vas_mem_alloc(sizeof(float) * x->filter->segmentSize);
            x->filter->imag[ele][azi][i] = vas_mem_alloc(sizeof(float) * x->filter->segmentSize);
            x->filter->data[ele][azi][i].realp = x->filter->real[ele][azi][i];
            x->filter->data[ele][azi][i].imagp = x->filter->imag[ele][azi][i];
            x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle] = &x->filter->data[ele][azi][i];
            x->filter->pointerToFFTSegments[ele][azi][i] = &x->filter->data[ele][azi][i];
#else
            x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle] = &x->filter->data[ele][azi][i*x->filter->fftSize];
            x->filter->pointerToFFTSegments[ele][azi][i] = &x->filter->data[ele][azi][i*x->filter->fftSize];
#endif
            vas_dynamicFirChannel_calculateAverageSegmentPower(x, filter+(i*x->filter->segmentSize), i, ele, azi);
            
            if(!vas_dynamicFirChannel_isFilterSegmentBelowThreshhold(x, filter+(i*x->filter->segmentSize), ele, azi))
            {
                vas_util_fcopy(filter+(i*x->filter->segmentSize), x->tmp, x->filter->segmentSize);
            
#ifdef VAS_USE_VDSP
                vDSP_ctoz ( ( COMPLEX * ) x->tmp , 2, &(x->filter->data[ele][azi][i]), 1, x->filter->segmentSize  );
                vDSP_fft_zrip ( x->filter->setupReal, &x->filter->data[ele][azi][i], 1, x->filter->fftSizeLog2, FFT_FORWARD  );
#endif
#ifdef VAS_USE_FFTW
                fftwf_plan tmpPlan = fftwf_plan_dft_r2c_1d(x->filter->fftSize, x->tmp, x->filter->pointerToFFTSegments[ele][azi][i], FFTW_ESTIMATE);
                fftwf_execute(tmpPlan);
                fftwf_destroy_plan(tmpPlan);
#endif
#ifdef VAS_USE_KISSFFT
                kiss_fftr(x->forwardFFT,x->tmp,x->filter->pointerToFFTSegments[ele][azi][i]);
#endif
                vas_util_complexScale(x->filter->pointerToFFTSegments[ele][azi][i], x->scale, x->filter->segmentSize);

                x->filter->segmentIsZero[ele][azi][i] = false;
                x->filter->segmentIsZero[ele][azi][i+x->pointerArrayMiddle] = false;
                x->filter->nonZeroCounter[ele][azi]++;
            }
            else
            {
                x->filter->segmentIsZero[ele][azi][i] = true;
                x->filter->segmentIsZero[ele][azi][i+x->pointerArrayMiddle] = true;
#ifdef VAS_USE_VDSP
                vas_mem_free(x->filter->real[ele][azi][i]);
                vas_mem_free(x->filter->imag[ele][azi][i]);
#endif
                x->filter->zeroCounter[ele][azi]++;
            }
            
            i++;
        }
    }
}

void vas_dynamicFirChannel_initArraysWithGlobalFilter(vas_dynamicFirChannel *x)
{
    vas_dynamicFirChannel_setFilterSize(x, x->filter->filterLength);
    vas_dynamicFirChannel_prepareArrays(x);
}

void vas_dynamicFirChannel_free(vas_dynamicFirChannel *x)
{
    if(!x->useSharedInput)
        vas_dynamicFirChannel_input_free(&x->input);
   
#ifdef VAS_USE_FFTW
     fftwf_destroy_plan(x->forwardFFT);
     fftwf_destroy_plan(x->inverseFFT);
#endif
    vas_mem_free(x->tmp);
    vas_mem_free(x->fadeOut);
    vas_mem_free(x->fadeIn);
    
    vas_dynamicFirChannel_output_free(&x->output);
    
    x->filter->referenceCounter--;
    
    if( ((x->setup & (VAS_GLOBALFILTER) ) && !x->filter->referenceCounter)
       || x->setup & VAS_LOCALFILTER)
    {
        x->filter->init = 0;
        if(!x->useSharedFilter)
        {
            vas_dynamicFirChannel_filter_free(x->filter);
#ifdef VERBOSE
#if defined(MAXMSPSDK) || defined(PUREDATA)
            post("Free  Filter");
#else
            printf("Free  Filter");
#endif
#endif
        }
    }
}

void vas_dynamicFirChannel_setInitFlag(vas_dynamicFirChannel *x)
{
    x->filter->init = 1;
}

void vas_dynamicFirChannel_removeInitFlag(vas_dynamicFirChannel *x)
{
    x->filter->init = 0;
}

void vas_dynamicFirChannel_shareInputWith(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel)
{
    x->sharedInput = &sharedInputChannel->input;
    x->useSharedInput = true;
    vas_dynamicFirChannel_input_free(&x->input);
}

void vas_dynamicFirChannel_getSharedFilterValues(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel)
{
    vas_dynamicFirChannel_setFilterSize(x, sharedInputChannel->filterSize);
}

void vas_dynamicFirChannel_shareFilterWith(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel)
{
    vas_dynamicFirChannel_filter_free(x->filter);
    x->filter = sharedInputChannel->filter;
    x->useSharedFilter = true;
}

vas_dynamicFirChannel *vas_dynamicFirChannel_new(int setup, int segmentSize, vas_dynamicFirChannel_config *firConfig)
{
    if(!setup)
        return NULL;
    
    vas_dynamicFirChannel *x = vas_mem_alloc(sizeof(vas_dynamicFirChannel));
    x->gain = 1.;
    x->setup = setup;
    x->segmentIndex = 0;
    x->useSharedInput = false;
    x->useSharedFilter = false;
    x->sharedInput = NULL;
    x->sharedFilter = NULL;
    x->init = 0;
    
    if(setup & VAS_GLOBALFILTER_LEFT)
    {
        x->filter = &globalFilterLeft;
        x->filter->referenceCounter++;
    }
    
    else if(setup & VAS_GLOBALFILTER_RIGHT)
    {
        x->filter = &globalFilterRight;
        x->filter->referenceCounter++;
    }
    else
    {
        x->filter = &x->localFilter;
        x->filter->init = 0;
    }
    
    if(firConfig == NULL)
    {
        if(setup & VAS_BINAURALSETUP_STD)
            x->filter->firSetup = &vas_binauralSetup_std;
        if(setup & VAS_BINAURALSETUP_NOELEVATION)
            x->filter->firSetup = &vas_binauralSetup_noEle;
        if(setup & VAS_STATICFILTER)
            x->filter->firSetup = &vas_staticSetup;
    }
    else
        x->filter->firSetup = firConfig;
    
    if(!x->filter->init)
        vas_dynamicFirChannel_filter_init(x->filter, segmentSize);
    
    x->scale = 1.0 / (x->filter->fftSize * x->filter->fftSize);
    x->tmp = (float *)vas_mem_alloc((sizeof(float)));
    x->fadeOut = (float *)vas_mem_alloc((sizeof(float)));
    x->fadeIn = (float *)vas_mem_alloc((sizeof(float)));
    x->segmentThreshold = 0;
    
    vas_dynamicFirChannel_output_init(&x->output, x->filter->firSetup->eleZero);
    vas_dynamicFirChannel_input_init(&x->input);
    
    x->elevationTmp = x->filter->firSetup->eleZero;
    x->azimuthTmp = 0;
    
    return x;
}

