#include "vas_dynamicFirChannel.h"

#ifdef VAS_USE_MULTITHREADCONVOLUTION
threadpool vas_partitioned_thpool;
#endif

void vas_filter_metaData_init(vas_fir_metaData *x)
{
    x->audioFormat = 0;
    x->azimuthStride = 1;
    x->elevationStride = 1;
    x->directionFormat = 0;
    x->filterLength = 0;
    x->lineFormat = 0;
    x->segmentSize = 256;
    x->numberOfIrs = 0;
    x->aziMin = 0;
    x->aziMax = 1000; // some value > 359
    x->aziRange = 0;
    x->aziZero = 0;
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
    x->aziMin = 0;
    x->aziMax = 1000; // some random value > 359
    
#ifdef VAS_USE_VDSP
    x->setupReal = NULL;
#else
    x->setupReal = NULL;
#endif
    
    for(int eleCount = 0; eleCount < VAS_ELEVATION_ANGLES_MAX; eleCount++)
    {
        for(int aziCount = 0; aziCount < VAS_AZIMUTH_ANGLES_MAX; aziCount++)
        {
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
            x->averageSegmentPower[eleCount][aziCount] = NULL;
            x->overallEnergy[eleCount][aziCount] = 0;
#endif
            
#ifdef VAS_USE_VDSP
            x->data[eleCount][aziCount] = NULL;
#endif
            x->pointerToFFTSegments[eleCount][aziCount] = NULL;
            x->segmentIsZero[eleCount][aziCount] = NULL;
        }
    }
    return x;
}

void vas_dynamicFirChannel_filter_reset(vas_dynamicFirChannel_filter *x)
{
#ifdef VAS_USE_VDSP
    if(x->setupReal)
        vDSP_destroy_fftsetup(x->setupReal);
#else
    if(x->setupReal)
        pffft_destroy_setup(x->setupReal);
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
#else
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
    x->filter->aziZero = metaData->aziZero;
    x->filter->aziMin = metaData->aziMin;
    x->filter->aziMax = metaData->aziMax;
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
#else
    if(x->setupReal)
        pffft_destroy_setup(x->setupReal);
#endif
    
    for(int eleCount = 0; eleCount < VAS_ELEVATION_ANGLES_MAX; eleCount++)
    {
        for(int aziCount = 0; aziCount < VAS_AZIMUTH_ANGLES_MAX; aziCount++)
        {
            if(x->data[eleCount][aziCount] != NULL)
            {
#ifdef VAS_USE_VDSP
                for(int i = 0; i < x->numberOfSegments; i++)
                {
                    vas_mem_free(x->data[eleCount][aziCount][i].realp);
                    vas_mem_free(x->data[eleCount][aziCount][i].imagp);
                }
#else
                for(int i = 0; i < x->numberOfSegments; i++)
                {
                    vas_mem_free(x->pointerToFFTSegments[eleCount][aziCount][i]);
                }
#endif
            }
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
            if(x->data[eleCount][aziCount])
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
    x->copy = NULL;
    x->real = NULL;
    x->imag = NULL;
    x->pointerToFFTSegments = NULL;
    x->data = NULL;
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
#else
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
#else
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
    x->current.signalFloat = NULL;
    x->next.signalFloat = NULL;
    x->outputSegment = NULL;
    x->current.overlap = NULL;
    x->next.overlap = NULL;
#ifdef VAS_USE_VDSP
    x->current.signalComplex.realp = NULL;
    x->current.signalComplex.imagp = NULL;
    x->next.signalComplex.realp = NULL;
    x->next.signalComplex.imagp = NULL;
#else
    x->current.signalComplex = NULL;
    x->next.signalComplex = NULL;
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
        int aziMin = 360 + x->filter->aziMin;
        int azi = azimuth;
        
        if(x->aziDirection)
            azi = 360 - azi;
        
        if(azi < 0)
            azi = 360 + azi;
   
        azi = azi % 360;
        
        if(azi > x->filter->aziMax && azi < aziMin)
            return;
         
        x->azimuthTmp = azi/x->filter->aziStride;
    }
}

void vas_dynamicFirChannel_setSegmentThreshold(vas_dynamicFirChannel *x, float thresh)
{
    if(thresh >= 0 && thresh < 1)
        x->segmentThreshold = thresh;

    vas_util_debug("%.20f", x->segmentThreshold);
}

void vas_dynamicFirChannel_setElevation(vas_dynamicFirChannel *x, int elevation)
{
    if(elevation >= x->filter->eleMin && elevation < x->filter->eleMax)
        x->elevationTmp = elevation/x->filter->eleStride+x->filter->eleZero;
}

void vas_dynamicFirChannel_setSegmentSize(vas_dynamicFirChannel *x, int segmentSize)
{
    if(!vas_util_isValidSegmentSize(segmentSize))
    {
        vas_util_debug("Set Segment Size");
        return;
    }
    
    x->filter->segmentSize = segmentSize;
    x->filter->fftSize = x->filter->segmentSize*2;
    x->scale = 1.0 / (x->filter->segmentSize * x->filter->segmentSize);
    x->filter->fftSizeLog2 = log2(x->filter->fftSize);
#ifdef VAS_USE_VDSP
    x->filter->setupReal = vDSP_create_fftsetup ( x->filter->fftSizeLog2, FFT_RADIX2);
#else
    x->filter->setupReal = pffft_new_setup(x->filter->fftSize, PFFFT_REAL);
#endif
    
#ifdef VERBOSE
    vas_util_debug("Segment Size is: %d", x->filter->segmentSize);
    vas_util_debug("FFT Size is: %d", x->filter->fftSize);
    vas_util_debug("FFT Size Log2 is: %d", x->filter->fftSizeLog2);
#endif
}

void vas_dynamicFirChannel_multiplyAddSegments(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target)
{
    x->segmentIndex = 1; // first segment just multiplying instead of multiplyAdd
#ifdef VAS_USE_VDSP
    if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex])
        vas_util_complexMultiply(x->filter->segmentSize, x->input->pointerToFFTSegments[0], x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex], &target->signalComplex);
    else
        vas_util_complexWriteZeros(&target->signalComplex, x->filter->segmentSize);
    
    while(x->segmentIndex < x->filter->numberOfSegments)
    {
        if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex])
            vas_util_complexMultiplyAdd(x->input->pointerToFFTSegments[x->segmentIndex], x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex], &target->signalComplex, x->filter->segmentSize);
        
        x->segmentIndex++;
    }
#else
    if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex])
        pffft_zconvolve_no_accu(x->filter->setupReal, (float *)x->input->pointerToFFTSegments[0], (float *)x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex], (float *) target->signalComplex, 1);
    else
        vas_util_complexWriteZeros(target->signalComplex, x->filter->segmentSize);
    
    while(x->segmentIndex < x->filter->numberOfSegments)
    {
        if(!x->filter->segmentIsZero[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex])
        {
            pffft_zconvolve_accumulate(x->filter->setupReal, (float *)x->input->pointerToFFTSegments[x->segmentIndex], (float *)x->filter->pointerToFFTSegments[target->elevation][target->azimuth][x->movingIndex-x->segmentIndex], (float *) target->signalComplex, 1); // move this to vas_util to complexMultiplyAdd
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
#else
    pffft_transform(x->filter->setupReal, x->input->copy, (float *) x->input->pointerToFFTSegments[x->movingIndex], x->fftWork, PFFFT_FORWARD );
#endif
}

void vas_dynamicFirChannel_inverseFFT(vas_dynamicFirChannel *x, vas_dynamicFirChannel_target *target)
{
#ifdef VAS_USE_VDSP
    vDSP_fft_zrip ( x->filter->setupReal, &target->signalComplex, 1, x->filter->fftSizeLog2, FFT_INVERSE  );
    vDSP_ztoc ( &target->signalComplex, 1, ( COMPLEX * ) (target->signalFloat), 2, x->filter->segmentSize );
#else
    pffft_transform(x->filter->setupReal, (float *) target->signalComplex,  target->signalFloat, x->fftWork, PFFFT_BACKWARD );
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
        if(x->startCrossfade >= 2) // we do not have an overlap for the first frame of the target direction, so we have to start one frame later..
            vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter(x);
    }
       
    x->movingIndex +=1;
    if (x->movingIndex == x->pointerArraySize)
        x->movingIndex = x->pointerArrayMiddle;
    
    if(!x->startCrossfade)
        vas_dynamicFirChannel_updateAzimuthAndElevation(x);
    
    return;
}

#ifdef VAS_USE_MULTITHREADCONVOLUTION

void vas_dynamicFirChannel_calculateConvolution_threaded(vas_dynamicFirChannel *x, float *output)
{
    vas_util_fcopy(x->output.current.signalFloat+x->filter->segmentSize, x->output.current.overlap, x->filter->segmentSize);
     
    if(!x->useSharedInput)
        vas_dynamicFirChannel_forwardFFTInput(x);
      
    vas_dynamicFirChannel_multiplyAddSegments(x, &x->output.current);
    vas_dynamicFirChannel_inverseFFT(x, &x->output.current);
    vas_util_fadd(x->output.current.overlap, x->output.current.signalFloat, output, x->filter->segmentSize);
       
    x->movingIndex +=1;
    if (x->movingIndex == x->pointerArraySize)
        x->movingIndex = x->pointerArrayMiddle;
     
    return;
}

void doWork1(void *args)
{
    vas_dynamicFirChannel *x = ((vas_threadedConvolutionArg *)args)->x;
    vas_dynamicFirChannel_calculateConvolution_threaded(x, ((vas_threadedConvolutionArg *)args)->data);
}

void vas_dynamicFirChannel_process_threaded1(vas_threadedConvolutionArg *arg, VAS_INPUTBUFFER *in, int vectorSize)
{
    vas_dynamicFirChannel *x = arg->x;
    float *pointerToFFTInput;
    int vsOverSegmentSize = vectorSize/x->filter->segmentSize;

    if(!x->init)
        return;
    
    if(!vsOverSegmentSize) // segment size > vector size
    {
        int segmentSizeOverVs = x->filter->segmentSize/vectorSize;
        pointerToFFTInput = x->input->copy;
        
        if(!x->useSharedInput)
        {
            pointerToFFTInput+= x->frameCounter * vectorSize;
            vas_util_fcopy(in, pointerToFFTInput, vectorSize);
        }
        
        x->frameCounter++;
        
        if(x->frameCounter == segmentSizeOverVs)
        {
            x->frameCounter = 0;
            thpool_add_work_noHeap(vas_partitioned_thpool, doWork1, arg);
        }
    }
}

#endif

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
#else
    x->output.current.signalComplex = (VAS_COMPLEX *)vas_mem_resize(x->output.current.signalComplex, (x->filter->segmentSize+8) * sizeof ( VAS_COMPLEX  ));
    x->output.next.signalComplex = (VAS_COMPLEX *)vas_mem_resize(x->output.next.signalComplex, (x->filter->segmentSize+8) * sizeof ( VAS_COMPLEX  ));
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
#else
            x->input->pointerToFFTSegments[i+x->pointerArrayMiddle] = vas_mem_alloc(sizeof(VAS_COMPLEX) * (x->filter->segmentSize+8));
            x->input->pointerToFFTSegments[i] = x->input->pointerToFFTSegments[i+x->pointerArrayMiddle];
#endif
            i++;
        }
    }
    else
    {
        vas_util_debug("Use shared input");
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
        if(vas_util_isValidSegmentSize(filterSize))
            x->filter->segmentSize = filterSize;
        else
            x->filter->segmentSize = vas_util_roundUp2NextPowerOf2(filterSize);
    }
    
    vas_dynamicFirChannel_setSegmentSize(x, x->filter->segmentSize);
    x->fadeLength = 512;
    if(x->fadeLength < x->filter->segmentSize)
        x->fadeLength = x->filter->segmentSize;
     
    x->filter->numberOfSegments = x->filterSize/x->filter->segmentSize;
    if(x->filterSize%x->filter->segmentSize != 0)
        x->filter->numberOfSegments+=1;
    
#ifdef VERBOSE
    vas_util_debug("Number of Segments: %d", x->filter->numberOfSegments);
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
#ifdef VAS_USE_PFFFT
    x->fftWork = (float *)vas_mem_resize(x->fftWork, sizeof(float) * x->filter->fftSize);
#endif
    x->fadeOut = (float *)vas_mem_resize(x->fadeOut, sizeof(float) * x->fadeLength);
    x->fadeIn = (float *)vas_mem_resize(x->fadeIn, sizeof(float) * x->fadeLength);
    vas_util_writeFadeOutArray(x->fadeLength, x->fadeOut);
    vas_util_writeFadeInArray(x->fadeLength, x->fadeIn);
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

void vas_dynamicFirChannel_prepareFilter(vas_dynamicFirChannel *x, float *filter, int ele, int azi)
{
    int size = x->filter->segmentSize;
    int numberOfSegmentsMinusOne = x->filter->numberOfSegments-1;
    
#ifdef VAS_USE_VDSP
    x->scale = 1.0 / (4*x->filter->fftSize); // this is apples vDSP convention
#else
    x->scale = 1.0 / (x->filter->fftSize);   // this seems to be correct for pffft
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
            x->filter->data[ele][azi][i].realp = vas_mem_resize(x->filter->data[ele][azi][i].realp, sizeof(float) * x->filter->segmentSize);
            x->filter->data[ele][azi][i].imagp = vas_mem_resize(x->filter->data[ele][azi][i].imagp, sizeof(float) * x->filter->segmentSize);
            x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle] = &x->filter->data[ele][azi][i];
            x->filter->pointerToFFTSegments[ele][azi][i] = &x->filter->data[ele][azi][i];
#else
            x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle] = vas_mem_resize(x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle], sizeof(VAS_COMPLEX) * (x->filter->segmentSize+8));
            x->filter->pointerToFFTSegments[ele][azi][i] = x->filter->pointerToFFTSegments[ele][azi][i+x->pointerArrayMiddle];
#endif
            if(i == numberOfSegmentsMinusOne)
            {
                size = x->filterSize - (x->filter->segmentSize*numberOfSegmentsMinusOne);
                vas_util_writeZeros(x->filter->fftSize, x->tmp);
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
#else
                pffft_transform(x->filter->setupReal, x->tmp, (float *) x->filter->pointerToFFTSegments[ele][azi][i], x->fftWork, PFFFT_FORWARD );
                vas_util_complexScale(x->filter->pointerToFFTSegments[ele][azi][i], x->scale, x->filter->segmentSize+8);
#endif
                x->filter->segmentIsZero[ele][azi][i] = false;
                x->filter->segmentIsZero[ele][azi][i+x->pointerArrayMiddle] = false;
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
                x->filter->nonZeroCounter[ele][azi]++;
#endif
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
#else
                vas_mem_free(x->filter->pointerToFFTSegments[ele][azi][i]);
                x->filter->pointerToFFTSegments[ele][azi][i] = NULL;
#endif
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
                x->filter->zeroCounter[ele][azi]++;
#endif
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
    vas_mem_free(x->fadeOut);
    vas_mem_free(x->fadeIn);
#ifdef VAS_USE_PFFFT
    if(x->fftWork)
        vas_mem_free(x->fftWork);
#endif
    vas_dynamicFirChannel_output_free(&x->output);
    
    x->filter->referenceCounter--;
    
    if( !x->filter->referenceCounter)
    {
        x->init = 0; // making sure, that filter is not accessed in the audio thread anymore
        vas_dynamicFirChannel_filter_free(x->filter);
        vas_util_debug("Free  Filter");
    }
    vas_mem_free(x);
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
    x->elevationTmp = 0;
    x->azimuthTmp = 0;
    x->filter = vas_dynamicFirChannel_filter_new();
    x->input = vas_dynamicFirChannel_input_new();
    x->tmp = NULL;
    x->fadeOut = NULL;
    x->fadeIn = NULL;
    x->segmentThreshold = 0;
#ifdef VAS_USE_PFFFT
    x->fftWork = NULL;
#endif
    vas_dynamicFirChannel_output_init(&x->output, 0);
    return x;
}

