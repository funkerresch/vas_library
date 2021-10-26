//
//  vas_pdmaxobject.c
//  vas_binaural~
//
//  Created by Harvey Keitel on 18.02.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#include "vas_pdmaxobject.h"

#ifdef PUREDATA
void vas_pdmaxobject_getFloatArrayAndLength(t_symbol *arrayname, t_word **array, int *length)
{
    t_garray *a;

    if (!(a = (t_garray *)pd_findbyclass(arrayname, garray_class)))
    {
        if (*arrayname->s_name) post("vas_fir: %s: no such array");
        *array = 0;
    }
    else if (!garray_getfloatwords(a, length, array))
    {
        post("bad template for vas_fir");
        *array = 0;
    }
    else
    {
        post("Reading IRs from array %s", arrayname->s_name);
    }
}

void vas_pdmaxobject_set_mono_simple(vas_pdmaxobject *x, t_symbol *left, float segmentSize)
{
    vas_fir *engine = x->convolutionEngine;
    int filterLenghtMinusOffset = 0;
    int minLength = 0;
    int currentIndex2Write = 0;
    
    x->segmentSize = segmentSize;
    vas_pdmaxobject_getFloatArrayAndLength(left, &x->leftArray, &x->leftArrayLength);
    
    minLength = x->leftArrayLength;

    vas_fir_setMetaData_manually1((vas_fir *)engine, minLength, segmentSize, VAS_IR_DIRECTIONFORMAT_SINGLE, 1, 1, VAS_IR_AUDIOFORMAT_STEREO, VAS_IR_LINEFORMAT_IR, 0, 0);
        
    filterLenghtMinusOffset = ((vas_fir *)engine)->metaData.filterLength;
    float *currentIr = (float *)vas_mem_alloc( filterLenghtMinusOffset * sizeof(float));
    engine->left->filter->filterLength = filterLenghtMinusOffset;
       
    currentIndex2Write = 0;
    for (int i=0;i<filterLenghtMinusOffset;i++)
        currentIr[currentIndex2Write++] = x->leftArray[i].w_float;
    
    vas_dynamicFirChannel_prepareFilter(engine->left, currentIr, 0, 0);
    
    vas_fir_setInitFlag((vas_fir *)engine);
    vas_mem_free(currentIr);
}

 // very quick and dirty reading from pd array

void vas_pdmaxobject_set1(vas_pdmaxobject *x, t_symbol *left, t_symbol *right, float segmentSize, float offset, float end)
{
    vas_fir *engine = x->convolutionEngine;
    int maxLength = 0;
    
    x->segmentSize = segmentSize;
    vas_pdmaxobject_getFloatArrayAndLength(left, &x->leftArray, &x->leftArrayLength);
    vas_pdmaxobject_getFloatArrayAndLength(right, &x->rightArray, &x->rightArrayLength);
    
    maxLength = x->leftArrayLength;
    if(x->rightArrayLength > maxLength)
        maxLength = x->rightArrayLength;
        
    size_t size = maxLength * sizeof(float);
    float *leftIr = (float *)vas_mem_alloc( size);
    float *rightIr = (float *)vas_mem_alloc( size);
    memset(leftIr, 0, size);
    memset(rightIr, 0, size);
       
    for (int i=0; i<x->leftArrayLength; i++)
        leftIr[i] = x->leftArray[i].w_float;
    
    for (int i=0; i<x->rightArrayLength;i++)
        rightIr[i] = x->rightArray[i].w_float;
    
    vas_fir_read_singleImpulseFromFloatArray(engine, left->s_name, leftIr, rightIr, maxLength, segmentSize, offset, end);
     
    vas_mem_free(leftIr);
    vas_mem_free(rightIr);
}

void vas_pdmaxobject_setAndInterpolateBetweenIndexes1(vas_pdmaxobject *x, t_symbol *s, int argc, t_atom *argv)
{
    vas_fir *engine = x->convolutionEngine;
    
    float *startIr;
    float *endIr;
    float *finalIr;
    
    float distance = 0;
    int weightFirstIR = 0;
    int weightSecondIR = 0;
    int normIndex = 0;
    int maxLengthLeft = 0;
    int maxLengthRight = 0;
    int startIndex = 0;
    int endIndex = 0;
    
    if(argc < 4)
        return;
    if(argc % 2 != 0)
        return;
    
    for(int i=0; i<argc; i++)
    {
        if(i %2 == 0)
        {
            if(argv[i].a_type != A_SYMBOL)
            {
                    post("Even arguments must specify array name as symbol");
                    return;
            }
            vas_pdmaxobject_getFloatArrayAndLength(argv[i].a_w.w_symbol, &x->leftArray, &x->leftArrayLength);
            if(x->leftArray == 0)
            {
                post("Array is zero");
                return;
            }
        }
        else
        {
            if(argv[i].a_type != A_FLOAT)
            {
                post("Odd arguments must specify index as float");
                return;
            }
            if(argv[i].a_w.w_float < 0)
                return;
            if(argv[i].a_w.w_float >= engine->metaData.numberOfIrs)
                return;
            
            endIndex = argv[i].a_w.w_float;
            
            if(endIndex < startIndex)
            {
                post("Specified index must be larges than previous one.");
                return;
            }
            
            startIndex = endIndex;
        }
    }
     
    int filterLength = ((vas_fir *)engine)->metaData.filterLength;
    if(filterLength == 0)
        return;
    
    startIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    endIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    finalIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    engine->left->filter->filterLength = filterLength;
    
    for(int i=0; i<argc; i+=2)
    {
        if(i+4 > argc)
        {
            vas_fir_setInitFlag((vas_fir *)engine);
            vas_mem_free(startIr);
            vas_mem_free(endIr);
            vas_mem_free(finalIr);
            return;
        }
        
        startIndex = argv[i+1].a_w.w_float;
        endIndex = argv[i+3].a_w.w_float;
        distance = endIndex - startIndex + 1;

        vas_pdmaxobject_getFloatArrayAndLength(argv[i].a_w.w_symbol, &x->leftArray, &x->leftArrayLength);
        vas_pdmaxobject_getFloatArrayAndLength(argv[i+2].a_w.w_symbol, &x->rightArray, &x->rightArrayLength);
          
        maxLengthLeft = fmin(x->leftArrayLength, filterLength);
        for (int i=0;i<maxLengthLeft;i++)
        {
            startIr[i] = x->leftArray[i].w_float;
        }

        maxLengthRight = fmin(x->rightArrayLength, filterLength);
        for (int i=0;i<maxLengthRight;i++)
        {
            endIr[i] = x->rightArray[i].w_float;
        }

        vas_dynamicFirChannel_prepareFilter(engine->left, startIr, 0, startIndex);
        vas_dynamicFirChannel_prepareFilter(engine->right, startIr, 0, startIndex);
        ((vas_dynamicFirChannel *) (engine->left))->filter->indexIsZero[0][(int)startIndex] = false;

        vas_dynamicFirChannel_prepareFilter(engine->left, endIr, 0, endIndex);
        vas_dynamicFirChannel_prepareFilter(engine->right, endIr, 0, endIndex);
        ((vas_dynamicFirChannel *) (engine->left))->filter->indexIsZero[0][(int)endIndex] = false;

        for (int index=startIndex+1; index<endIndex; index++)
        {
            normIndex = index - startIndex;
            weightFirstIR = distance - normIndex;
            weightSecondIR = normIndex;
            for (int i=0;i<filterLength;i++)
            {
                finalIr[i] = (startIr[i] * weightFirstIR + endIr[i] * weightSecondIR) / distance;
            }
            
            vas_dynamicFirChannel_prepareFilter(engine->left, finalIr, 0, index);
            vas_dynamicFirChannel_prepareFilter(engine->right, finalIr, 0, index);
            ((vas_dynamicFirChannel *) (engine->left))->filter->indexIsZero[0][(int)index] = false;
        }
    }
}

#endif

void vas_pdmaxobject_read(vas_pdmaxobject *x, t_symbol *s, float segmentSize, float offset, float end)
{
#ifdef PUREDATA
    if(canvas_dspstate)
#else
    if(sys_getdspstate())
#endif
    {
        post("Turn off DSP before loading new IRs");
        return;
    }
    const char *filename = s->s_name;
    
    vas_fir *engine = x->convolutionEngine;
    if(segmentSize)
        x->segmentSize = segmentSize;
    
    if(engine->left->filter->referenceCounter > 1)
    {
        post("Another instance is referencing this filter.");
        return;
    }

#ifdef PUREDATA
    unsigned long length = strlen(x->canvasDirectory);
    if(x->canvasDirectory[length-1] == '/')
        sprintf(x->fullpath, "%s%s", x->canvasDirectory, filename);
    else
        sprintf(x->fullpath, "%s/%s", x->canvasDirectory, filename);
#else
    vas_maxObjectUtilities_openFile1(s, x->fullpath);
#endif
    post("IR Path: %s", x->fullpath);
    
    vas_fir_read_impulseFromFile(x->convolutionEngine, x->fullpath, segmentSize, offset, end);
}
