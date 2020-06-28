#include "vas_dynamicFirChannel.h"
//#include <pthread.h>

void vas_filter_metaData_init(vas_fir_metaData *x)
{
    x->audioFormat = 0;
    x->azimuthStride = 1;
    x->elevationStride = 1;
    x->directionFormat = 0;
    x->filterLength = 0;
    x->lineFormat = 0;
    x->segmentSize = 256;
    x->fullPath = NULL;
}

vas_dynamicFirChannel_filter *vas_dynamicFirChannel_filter_new()
{
    vas_dynamicFirChannel_filter *x = vas_mem_alloc(sizeof(vas_dynamicFirChannel_filter));
    x->segmentSize = 256;
    x->fftSize = x->segmentSize * 2;
    x->fftSizeLog2 = log2(x->fftSize);
    x->numberOfSegments = 0;
    x->referenceCounter = 0;
    x->eleRange = 0;
    x->aziRange = 0;
    x->aziStride = 1;
    x->eleMin = 0;
    x->eleMax = 1;
    
#ifdef VAS_USE_VDSP
    x->setupReal = NULL;
#endif
    
#ifdef VAS_USE_KISSFFT
    x->forwardFFT = NULL;
    x->inverseFFT = NULL;
#endif
    
    for(int eleCount = 0; eleCount < VAS_ELEVATION_ANGLES_MAX; eleCount++)
    {
        for(int aziCount = 0; aziCount < VAS_AZIMUTH_ANGLES_MAX; aziCount++)
        {
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
            x->averageSegmentPower[eleCount][aziCount] = vas_mem_alloc(sizeof(double));
            x->overallEnergy[eleCount][aziCount] = 0;
#endif
            
#ifdef VAS_USE_VDSP
            x->data[eleCount][aziCount] = vas_mem_alloc( sizeof(VAS_COMPLEX *));
#endif
            x->pointerToFFTSegments[eleCount][aziCount] = vas_mem_alloc( sizeof(VAS_COMPLEX *));
            x->segmentIsZero[eleCount][aziCount] = vas_mem_alloc( sizeof(bool));
        }
    }
    return x;
}

void vas_dynamicFirChannel_filter_reset(vas_dynamicFirChannel_filter *x)
{
#ifdef VAS_USE_VDSP
    if(x->setupReal)
        vDSP_destroy_fftsetup(x->setupReal);
#endif
        
#ifdef VAS_USE_KISSFFT
    if(x->forwardFFT)
        kiss_fftr_free(x->forwardFFT);
    if(x->inverseFFT)
        kiss_fftr_free(x->inverseFFT);
#endif
    
    for(int eleCount = 0; eleCount < x->eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < x->aziRange; aziCount++)
        {
#ifdef VAS_USE_VDSP
            for(int i = 0; i < x->numberOfSegments; i++)
            {
                vas_mem_free(x->data[eleCount][aziCount][i].realp);
                vas_mem_free(x->data[eleCount][aziCount][i].imagp);
            }
#endif
            
#ifdef VAS_USE_KISSFFT
            for(int i = 0; i < x->numberOfSegments; i++)
            {
                vas_mem_free(x->pointerToFFTSegments[eleCount][aziCount][i]);
            }
#endif
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
            x->overallEnergy[eleCount][aziCount] = 0;
#endif
        }
    }
    x->numberOfSegments = 0;
}

void vas_dynamicFirChannel_init1(vas_dynamicFirChannel *x, vas_fir_metaData *metaData, int segmentSize) // filter init must be called, if either segmentSize, eleRange or aziRange change
{
    if(x->init)
    {
        vas_dynamicFirChannel_input_reset(x->input, x->filter->numberOfSegments);
        vas_dynamicFirChannel_filter_reset(x->filter);
        x->init = 0;
    }
    
    x->frameCounter = 0;
    x->filter->referenceCounter = 1;
    x->filter->segmentSize = segmentSize;
    x->filter->fftSize = segmentSize * 2;
    x->filter->fftSizeLog2 = log2(x->filter->fftSize);
    x->filter->directionFormat = metaData->directionFormat;
    x->filter->eleStride = metaData->elevationStride;
    x->filter->aziStride = metaData->azimuthStride;
    x->filter->aziRange = metaData->aziRange;
    x->filter->eleRange = metaData->eleRange;
    x->filter->eleMin = metaData->eleMin;
    x->filter->eleMax = metaData->eleMax;
    x->filter->eleZero = metaData->eleZero;
    x->elevationTmp = metaData->eleZero;  // added
    x->filter->offset = metaData->filterOffset;
    
    vas_dynamicFirChannel_setFilterSize(x, metaData->filterLength - x->filter->offset);
    vas_dynamicFirChannel_prepareArrays(x);
    
}

void vas_dynamicFirChannel_filter_free(vas_dynamicFirChannel_filter *x)
{
#ifdef VAS_USE_VDSP
    if(x->setupReal)
        vDSP_destroy_fftsetup(x->setupReal);
#endif
    
#ifdef VAS_USE_KISSFFT
    if(x->forwardFFT)
        kiss_fftr_free(x->forwardFFT);
    if(x->inverseFFT)
        kiss_fftr_free(x->inverseFFT);
#endif
    
    for(int eleCount = 0; eleCount < x->eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < x->aziRange; aziCount++)
        {
#ifdef VAS_USE_VDSP
            for(int i = 0; i < x->numberOfSegments; i++)
            {
                vas_mem_free(x->data[eleCount][aziCount][i].realp);
                vas_mem_free(x->data[eleCount][aziCount][i].imagp);
            }
#endif
            
#ifdef VAS_USE_KISSFFT
            for(int i = 0; i < x->numberOfSegments; i++)
            {
                vas_mem_free(x->pointerToFFTSegments[eleCount][aziCount][i]);
            }
#endif
        }
    }
    
    for(int eleCount = 0; eleCount < VAS_ELEVATION_ANGLES_MAX; eleCount++)
    {
        for(int aziCount = 0; aziCount < VAS_AZIMUTH_ANGLES_MAX; aziCount++)
        {
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
            vas_mem_free(x->averageSegmentPower[eleCount][aziCount]);
#endif
            vas_mem_free(x->segmentIsZero[eleCount][aziCount]);
#ifdef VAS_USE_VDSP
            vas_mem_free(x->data[eleCount][aziCount]);
#endif
            vas_mem_free(x->pointerToFFTSegments[eleCount][aziCount]);
        }
    }
    vas_mem_free(x);
}

vas_dynamicFirChannel_input *vas_dynamicFirChannel_input_new()
{
    vas_dynamicFirChannel_input *x = vas_mem_alloc(sizeof(vas_dynamicFirChannel_input));
    x->copy = (float *)vas_mem_alloc((sizeof(float)));
    x->real = ( float* ) vas_mem_alloc((sizeof(float)));
    x->imag = ( float* ) vas_mem_alloc((sizeof(float)));
    x->pointerToFFTSegments = (VAS_COMPLEX **) vas_mem_alloc(sizeof(VAS_COMPLEX *));
    x->data = ( VAS_COMPLEX * ) vas_mem_alloc((sizeof(VAS_COMPLEX)));
    return x;
}

void vas_dynamicFirChannel_input_reset(vas_dynamicFirChannel_input *x, int currentNumberOfSegments)
{
    int i = 0;
    while(i < currentNumberOfSegments) // dealloc old input arrays, in case of new object, number of segemnts is zero..
    {
#ifdef VAS_USE_VDSP
        vas_mem_free(x->data[i].realp);
        vas_mem_free(x->data[i].imagp);
#endif
        
#ifdef VAS_USE_KISSFFT
        vas_mem_free(x->pointerToFFTSegments[i]);
#endif
        i++;
    }
}

void vas_dynamicFirChannel_input_free(vas_dynamicFirChannel_input *x, int currentNumberOfSegments)
{
    int i = 0;
    while(i < currentNumberOfSegments) // dealloc old input arrays, in case of new object, number of segemnts is zero..
    {
#ifdef VAS_USE_VDSP
        vas_mem_free(x->data[i].realp);
        vas_mem_free(x->data[i].imagp);
#endif
        
#ifdef VAS_USE_KISSFFT
        vas_mem_free(x->pointerToFFTSegments[i]);
#endif
        i++;
    }
    vas_mem_free(x->real);
    vas_mem_free(x->imag);
    vas_mem_free(x->pointerToFFTSegments);
    vas_mem_free(x->data);
    vas_mem_free(x->copy);
    vas_mem_free(x);
}

void vas_dynamicFirChannel_output_init(vas_dynamicFirChannel_output *x, int elevationZero)
{
    x->current.azimuth = 0;
    x->next.azimuth = 0;
    x->current.elevation = elevationZero;
    x->next.elevation = elevationZero;
    
    x->current.signalFloat = vas_mem_alloc(sizeof(float));
    x->next.signalFloat = vas_mem_alloc(sizeof(float));
    x->outputSegment = vas_mem_alloc(sizeof(float));
    x->current.overlap = vas_mem_alloc(sizeof(float));
    x->next.overlap = vas_mem_alloc(sizeof(float));
#ifdef VAS_USE_VDSP
    x->current.signalComplex.realp = vas_mem_alloc(sizeof(float));
    x->current.signalComplex.imagp = vas_mem_alloc(sizeof(float));
    x->next.signalComplex.realp = vas_mem_alloc(sizeof(float));
    x->next.signalComplex.imagp = vas_mem_alloc(sizeof(float));
#else
    x->current.signalComplex = vas_mem_alloc(sizeof(VAS_COMPLEX));
    x->next.signalComplex = vas_mem_alloc(sizeof(VAS_COMPLEX));
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

void vas_dynamicFirChannel_setAzimuth(vas_dynamicFirChannel *x, int azimuth)
{
    if(x->filter->directionFormat != VAS_IR_DIRECTIONFORMAT_SINGLE)
    {
        int azi = azimuth;
        if(azi < 0)
            azi = 360 - azi;
        
        azi = azi % 360;
        x->azimuthTmp = azi/x->filter->aziStride;
    }
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
    if(elevation >= x->filter->eleMin && elevation < x->filter->eleMax)
        x->elevationTmp = elevation/x->filter->eleStride+x->filter->eleZero;
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
    x->filter->forwardFFT = kiss_fftr_alloc(x->filter->fftSize,0,0,0);
    x->filter->inverseFFT = kiss_fftr_alloc(x->filter->fftSize,1,0,0);
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

void vas_dynamicFirChannel_multiplyAddSegments(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target)
{
    x->segmentIndex = 0;
#ifdef VAS_USE_VDSP
    vas_util_complexWriteZeros(&target->signalComplex, x->filter->segmentSize);
    
    //int nnn = x->filter->numberOfSegments/4; // idea for multithreaded solution
    while(x->segmentIndex < x->filter->numberOfSegments)
    {
        if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex])
            vas_util_complexMultiplyAdd(x->input->pointerToFFTSegments[x->segmentIndex], x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex], &target->signalComplex, x->filter->segmentSize);
        
        x->segmentIndex++;
    }
#else
    vas_util_complexWriteZeros(target->signalComplex, x->filter->segmentSize+8);
    
    while(x->segmentIndex < x->filter->numberOfSegments)
    {
        if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex])
        {
            vas_util_complexMultiplyAdd(x->input->pointerToFFTSegments[x->segmentIndex], x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex], target->signalComplex, x->filter->segmentSize+8);
        }

        x->segmentIndex++;
    }
#endif
}

void vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter(vas_dynamicFirChannel *x)
{
    vas_util_fadd(x->output.next.overlap, x->output.next.signalFloat, x->output.next.signalFloat, x->filter->segmentSize);
    
    vas_util_fmultiply(x->fadeOut+x->fadeCounter*x->filter->segmentSize, x->output.outputSegment, x->output.outputSegment, x->filter->segmentSize);
    vas_util_fmultiply(x->fadeIn+x->fadeCounter*x->filter->segmentSize, x->output.next.signalFloat, x->output.next.signalFloat, x->filter->segmentSize);
    vas_util_fadd(x->output.next.signalFloat, x->output.outputSegment, x->output.outputSegment, x->filter->segmentSize);
    
    x->fadeCounter++;
    
    if(x->fadeCounter == x->numberOfFramesForCrossfade)
    {
        x->fadeCounter = 0;
        x->startCrossfade = 0;
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
    vDSP_ctoz ( ( COMPLEX * ) x->input->copy, 2, x->input->pointerToFFTSegments[x->movingIndex], 1, x->filter->segmentSize  );
    vDSP_fft_zrip ( x->filter->setupReal, x->input->pointerToFFTSegments[x->movingIndex], 1, x->filter->fftSizeLog2, FFT_FORWARD  );
#endif
    
#ifdef VAS_USE_KISSFFT
    kiss_fftr(x->filter->forwardFFT,x->input->copy,x->input->pointerToFFTSegments[x->movingIndex]);
#ifdef VAS_USE_AVX
    vas_util_deinterleaveComplexArray2(x->input->pointerToFFTSegments[x->movingIndex], x->deInterleaveReal, x->deInterleaveImag, x->filter->segmentSize+8);
#endif
#endif
}

void vas_dynamicFirChannel_inverseFFT(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target)
{
#ifdef VAS_USE_VDSP
    vDSP_fft_zrip ( x->filter->setupReal, &target->signalComplex, 1, x->filter->fftSizeLog2, FFT_INVERSE  );
    vDSP_ztoc ( &target->signalComplex, 1, ( COMPLEX * ) (target->signalFloat), 2, x->filter->segmentSize );
#endif
    
#ifdef VAS_USE_KISSFFT
    kiss_fftri(x->filter->inverseFFT, target->signalComplex, target->signalFloat);
#endif
}

void vas_dynamicFirChannel_calculateConvolution(vas_dynamicFirChannel *x)
{
    vas_util_fcopy(x->output.current.signalFloat+x->filter->segmentSize, x->output.current.overlap, x->filter->segmentSize);
     
    if(!x->useSharedInput)
        vas_dynamicFirChannel_forwardFFTInput(x);
      
    vas_dynamicFirChannel_multiplyAddSegments(x, &x->output.current);
    vas_dynamicFirChannel_inverseFFT(x, &x->output.current);
    vas_util_fadd(x->output.current.overlap, x->output.current.signalFloat, x->output.outputSegment, x->filter->segmentSize);
    
    if(x->output.current.azimuth != x->output.next.azimuth || x->output.current.elevation != x->output.next.elevation)
    {
        vas_util_fcopy(x->output.next.signalFloat+x->filter->segmentSize, x->output.next.overlap, x->filter->segmentSize);
        vas_dynamicFirChannel_multiplyAddSegments(x, &x->output.next);
        vas_dynamicFirChannel_inverseFFT(x, &x->output.next);
        x->startCrossfade++;
        if(x->startCrossfade >= 2)
            vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter(x);
    }
       
    x->movingIndex +=1;
    if (x->movingIndex == x->pointerArraySize)
        x->movingIndex = x->pointerArrayMiddle;
    
    if(!x->startCrossfade)
        vas_dynamicFirChannel_updateAzimuthAndElevation(x);
    
    return;
}

void vas_dynamicFirChannel_process(vas_dynamicFirChannel *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize, int flags)
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

void vas_dynamicFirChannel_prepareOutputSignal(vas_dynamicFirChannel *x)
{
    x->output.current.signalFloat = (float *)vas_mem_resize(x->output.current.signalFloat, sizeof(float) * x->filter->fftSize);
    x->output.next.signalFloat = (float *)vas_mem_resize(x->output.next.signalFloat, sizeof(float) * x->filter->fftSize);
    x->output.outputSegment = (float *)vas_mem_resize(x->output.outputSegment, sizeof(float) * x->filter->segmentSize); // must be segmentsize, changed for debugging
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
    x->output.current.signalComplex = (VAS_COMPLEX *)vas_mem_resize(x->output.current.signalComplex, (x->filter->segmentSize+8) * sizeof ( VAS_COMPLEX  ));
    x->output.next.signalComplex = (VAS_COMPLEX *)vas_mem_resize(x->output.next.signalComplex, (x->filter->segmentSize+8) * sizeof ( VAS_COMPLEX  ));
    vas_util_complexWriteZeros(x->output.current.signalComplex, x->filter->segmentSize+8);
    vas_util_complexWriteZeros(x->output.next.signalComplex, x->filter->segmentSize+8);
 #endif
}

void vas_dynamicFirChannel_prepareInputSignal(vas_dynamicFirChannel *x)
{
    if(!x->useSharedInput)
    {
        int i = 0;
        x->input->copy = (float *)vas_mem_resize(x->input->copy, sizeof(float) * x->filter->fftSize);
#ifdef VAS_USE_VDSP
        x->input->pointerToFFTSegments = (VAS_COMPLEX **) vas_mem_resize(x->input->pointerToFFTSegments, x->pointerArraySize * sizeof (VAS_COMPLEX *));
        x->input->data = ( VAS_COMPLEX * )vas_mem_resize(x->input->data, x->filter->numberOfSegments * sizeof (VAS_COMPLEX));
#else
        x->input->pointerToFFTSegments = (VAS_COMPLEX **) vas_mem_resize(x->input->pointerToFFTSegments, x->pointerArraySize * sizeof (VAS_COMPLEX *));
#endif
        while(i < x->filter->numberOfSegments) //create pointer array to signal
        {
#ifdef VAS_USE_VDSP
            x->input->data[i].realp = vas_mem_alloc(sizeof(float) * (x->filter->segmentSize));
            x->input->data[i].imagp = vas_mem_alloc(sizeof(float) * (x->filter->segmentSize));
            x->input->pointerToFFTSegments[i+x->pointerArrayMiddle] = &x->input->data[i];
            x->input->pointerToFFTSegments[i] = &x->input->data[i];
            vas_util_complexWriteZeros(x->input->pointerToFFTSegments[i],  x->filter->segmentSize);
#endif
            
#ifdef VAS_USE_KISSFFT
            x->input->pointerToFFTSegments[i+x->pointerArrayMiddle] = vas_mem_alloc(sizeof(VAS_COMPLEX) * (x->filter->segmentSize+8));
            x->input->pointerToFFTSegments[i] = x->input->pointerToFFTSegments[i+x->pointerArrayMiddle];
            vas_util_complexWriteZeros(x->input->pointerToFFTSegments[i],  x->filter->segmentSize+8);
#endif
            i++;
        }
    }
    else
    {
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Use shared input");
#endif
        x->input->pointerToFFTSegments = x->sharedInput->pointerToFFTSegments;
    }
}

void vas_dynamicFirChannel_setFilterSize(vas_dynamicFirChannel *x, int filterSize)
{
    if(x->filter->segmentSize == 0)
    {
       // post("invalid segmentSize");
        return;
    }
    
    x->filterSize = filterSize;
    if(x->filter->segmentSize > filterSize)
    {
        if(vas_utilities_isValidSegmentSize(filterSize))
            x->filter->segmentSize = filterSize;
        else
            x->filter->segmentSize = vas_utilities_roundUp2NextPowerOf2(filterSize);
    }
    
    vas_dynamicFirChannel_setSegmentSize(x, x->filter->segmentSize);
    x->fadeLength = 1024;
    if(x->fadeLength < x->filter->segmentSize)
        x->fadeLength = x->filter->segmentSize;
     
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
    x->deInterleaveReal = (float *)vas_mem_resize(x->deInterleaveReal, sizeof(float) * (x->filter->segmentSize + 8));
    x->deInterleaveImag = (float *)vas_mem_resize(x->deInterleaveImag, sizeof(float) * (x->filter->segmentSize + 8));
    x->fadeOut = (float *)vas_mem_resize(x->fadeOut, sizeof(float) * x->fadeLength);
    x->fadeIn = (float *)vas_mem_resize(x->fadeIn, sizeof(float) * x->fadeLength);
    vas_utilities_writeFadeOutArray(x->fadeLength, x->fadeOut);
    vas_utilities_writeFadeInArray(x->fadeLength, x->fadeIn);
    vas_dynamicFirChannel_prepareInputSignal(x);
    vas_dynamicFirChannel_prepareOutputSignal(x);
}

#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER

void vas_dynmaicFirChannel_resetMinMaxAverageSegmentPower(vas_dynamicFirChannel *x, int ele, int azi)
{
    x->filter->minAverageSegmentPower[ele][azi] = 100000;
    x->filter->maxAverageSegmentPower[ele][azi] = -100000;
    x->filter->zeroCounter[ele][azi] = 0;
    x->filter->nonZeroCounter[ele][azi] = 0;
}

void vas_dynamicFirChannel_leaveActivePartitions(vas_dynamicFirChannel *x, int numberOfActivePartions)
{
    for(int eleCount = 0; eleCount < x->filter->eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < x->filter->aziRange; aziCount++)
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
                while(j++ < numberOfActivePartions )
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
#endif

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

/*void vas_firobject_postDeinterleavedVasComplexArray(VAS_COMPLEX *array, int length)
{
    int imagIndex = floor(length/2.);
    float *real = (float *)array;
    float *imag = (float *)(&(array[imagIndex].i));
    for (int i = 0; i < length; i++)
    {
        post("Real: %d %f", i, *real++);
        post("Imag %d %f", i, *imag++);
    }
}

void vas_firobject_postVasComplexArray(VAS_COMPLEX *array, int length)
{
    for (int i = 0; i < length; i++)
    {
        post("Real: %d %f", i, array[i].r);
        post("Imag %d %f", i, array[i].i);
    }
}*/



void vas_dynamicFirChannel_prepareFilter(vas_dynamicFirChannel *x, float *filter, int ele, int azi)
{
    int size = x->filter->segmentSize;
    int numberOfSegmentsMinusOne = x->filter->numberOfSegments-1;
    
#ifdef VAS_USE_VDSP
    x->scale = 1.0 / (4*x->filter->fftSize); // this is apples vDSP convention
#endif
    
#ifdef VAS_USE_KISSFFT
    x->scale = 1.0 / (x->filter->fftSize); // this seems to be correct for kissFFT although I did not check it yet
#endif
    
    if(!x->sharedFilter)
    {
        int i = 0;
#ifdef VAS_USE_VDSP
        x->filter->data[ele][azi] = ( VAS_COMPLEX * )vas_mem_resize(x->filter->data[ele][azi], x->filter->numberOfSegments * sizeof (VAS_COMPLEX));
#endif
        x->filter->pointerToFFTSegments[ele][azi] = (VAS_COMPLEX **) vas_mem_resize(x->filter->pointerToFFTSegments[ele][azi], x->pointerArraySize * sizeof (VAS_COMPLEX *));
        
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
        x->filter->averageSegmentPower[ele][azi] = (double *)vas_mem_resize(x->filter->averageSegmentPower[ele][azi], sizeof(double) * x->filter->numberOfSegments);
#endif
        x->filter->segmentIsZero[ele][azi] = (bool *)vas_mem_resize(x->filter->segmentIsZero[ele][azi], sizeof(bool) * x->pointerArraySize);
        
        while(i < x->filter->numberOfSegments)         //create pointer array to filtersegments
        {
#ifdef VAS_USE_VDSP
            x->filter->data[ele][azi][i].realp = vas_mem_alloc(sizeof(float) * x->filter->segmentSize);
            x->filter->data[ele][azi][i].imagp = vas_mem_alloc(sizeof(float) * x->filter->segmentSize);
            x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle] = &x->filter->data[ele][azi][i];
            x->filter->pointerToFFTSegments[ele][azi][i] = &x->filter->data[ele][azi][i];
#endif
#ifdef VAS_USE_KISSFFT
            x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle] = vas_mem_alloc(sizeof(VAS_COMPLEX) * (x->filter->segmentSize+8));
            x->filter->pointerToFFTSegments[ele][azi][i] = x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle];
#endif
            if(i == numberOfSegmentsMinusOne)
            {
                size = x->filterSize - (x->filter->segmentSize*numberOfSegmentsMinusOne);
                vas_utilities_writeZeros(x->filter->fftSize, x->tmp);
            }
            
            vas_util_fcopy_noavx(filter+(i*x->filter->segmentSize), x->tmp, size); // size might not be a multiple of 8
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
            vas_dynamicFirChannel_calculateAverageSegmentPower(x, x->tmp, i, ele, azi);
#endif
            if(!vas_dynamicFirChannel_isFilterSegmentBelowThreshhold(x, x->tmp, ele, azi))
            {
#ifdef VAS_USE_VDSP
                vDSP_ctoz ( ( COMPLEX * ) x->tmp , 2, &(x->filter->data[ele][azi][i]), 1, x->filter->segmentSize  );
                vDSP_fft_zrip ( x->filter->setupReal, &x->filter->data[ele][azi][i], 1, x->filter->fftSizeLog2, FFT_FORWARD  );
                vas_util_complexScale(x->filter->pointerToFFTSegments[ele][azi][i], x->scale, x->filter->segmentSize);
#endif

#ifdef VAS_USE_KISSFFT
                kiss_fftr(x->filter->forwardFFT,x->tmp,x->filter->pointerToFFTSegments[ele][azi][i]);
                vas_util_complexScale(x->filter->pointerToFFTSegments[ele][azi][i], x->scale, x->filter->segmentSize+8);
#ifdef VAS_USE_AVX
                vas_util_deinterleaveComplexArray2(x->filter->pointerToFFTSegments[ele][azi][i], x->deInterleaveReal, x->deInterleaveImag, x->filter->segmentSize+8);
#endif
#endif
                x->filter->segmentIsZero[ele][azi][i] = false;
                x->filter->segmentIsZero[ele][azi][i+x->pointerArrayMiddle] = false;
                x->filter->nonZeroCounter[ele][azi]++;
            }
            else
            {
                x->filter->segmentIsZero[ele][azi][i] = true;
                x->filter->segmentIsZero[ele][azi][i+x->pointerArrayMiddle] = true;
#ifdef VAS_USE_VDSP
                vas_mem_free(x->filter->data[ele][azi][i].realp);
                vas_mem_free(x->filter->data[ele][azi][i].imagp);
                x->filter->data[ele][azi][i].realp = NULL;
                x->filter->data[ele][azi][i].imagp = NULL;
#endif
#ifdef VAS_USE_KISSFFT
                vas_mem_free(x->filter->pointerToFFTSegments[ele][azi][i]);
                x->filter->pointerToFFTSegments[ele][azi][i] = NULL;
#endif
                x->filter->zeroCounter[ele][azi]++;
            }
            i++;
        }
    }
}

void vas_dynamicFirChannel_free(vas_dynamicFirChannel *x)
{
    if(!x->useSharedInput)
        vas_dynamicFirChannel_input_free(x->input, x->filter->numberOfSegments);
   
    vas_mem_free(x->tmp);
    vas_mem_free(x->deInterleaveReal);
    vas_mem_free(x->deInterleaveImag);
    vas_mem_free(x->fadeOut);
    vas_mem_free(x->fadeIn);
    
    vas_dynamicFirChannel_output_free(&x->output);
    
    x->filter->referenceCounter--;
    
    if( !x->filter->referenceCounter)
    {
        x->init = 0; // making sure, that filter is not accessed in the audio thread anymore
        vas_dynamicFirChannel_filter_free(x->filter);
        
//#ifdef VERBOSE
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Free  Filter");
#else
        printf("Free  Filter");
#endif
//#endif
        
    }
}

void vas_dynamicFirChannel_setInitFlag(vas_dynamicFirChannel *x)
{
    x->init = 1;
}

void vas_dynamicFirChannel_removeInitFlag(vas_dynamicFirChannel *x)
{
    x->init = 0;
}

void vas_dynamicFirChannel_shareInputWith(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel)
{
    x->sharedInput = sharedInputChannel->input;
    x->useSharedInput = true;
   // vas_dynamicFirChannel_input_free(&x->input);
}

void vas_dynamicFirChannel_getSharedFilterValues(vas_dynamicFirChannel *x, vas_dynamicFirChannel *sharedInputChannel)
{
    if(x->filter)
        vas_dynamicFirChannel_filter_free(x->filter);
    x->filter = sharedInputChannel->filter;
    x->useSharedFilter = true;
    x->frameCounter = 0;
    vas_dynamicFirChannel_setFilterSize(x, sharedInputChannel->filterSize);
}

vas_dynamicFirChannel *vas_dynamicFirChannel_new(int setup)
{
    vas_dynamicFirChannel *x = vas_mem_alloc(sizeof(vas_dynamicFirChannel));
    x->gain = 1.;
    x->setup = setup;
    x->segmentIndex = 0;
    x->useSharedInput = false;
    x->useSharedFilter = false;
    x->sharedInput = NULL;
    x->sharedFilter = NULL;
    x->init = 0;
    x->frameCounter = 0;
    x->startCrossfade  = 0;

    x->filter = vas_dynamicFirChannel_filter_new();
    x->input = vas_dynamicFirChannel_input_new();
    
    x->tmp = (float *)vas_mem_alloc((sizeof(float)));
    x->fadeOut = (float *)vas_mem_alloc((sizeof(float)));
    x->fadeIn = (float *)vas_mem_alloc((sizeof(float)));
    x->deInterleaveReal = (float *)vas_mem_alloc((sizeof(float)));
    x->deInterleaveImag = (float *)vas_mem_alloc((sizeof(float)));
    x->segmentThreshold = 0;
    
    vas_dynamicFirChannel_output_init(&x->output, 0);
    
    x->elevationTmp = 0;
    x->azimuthTmp = 0;
    
    return x;
}

