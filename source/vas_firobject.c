#include "vas_firobject.h"

#ifdef __cplusplus
extern "C" {
#endif
extern vas_fir_list IRs;
#ifdef __cplusplus
}
#endif

void vas_firobject_getFloatArrayAndLength(rwa_firobject *x, t_symbol *arrayname, t_word **array, int *length)
{
    t_garray *a;

    if (!(a = (t_garray *)pd_findbyclass(arrayname, garray_class)))
    {
        if (*arrayname->s_name) pd_error(x, "vas_fir: %s: no such array",
            arrayname->s_name);
        *array = 0;
    }
    else if (!garray_getfloatwords(a, length, array))
    {
        pd_error(x, "%s: bad template for vas_fir", arrayname->s_name);
        *array = 0;
    }
    else
    {
        post("Reading IRs from array %s", arrayname->s_name);
    }
}

void vas_firobject_prepareForInterpolatedIrs(rwa_firobject *x, float segmentSize, float maxLength, float numberOfIrs)
{
    vas_fir *engine = x->convolutionEngine;
    x->segmentSize = segmentSize;
    vas_fir_setMetaData_manually1((vas_fir *)engine, maxLength, segmentSize, VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH, 1, 3, VAS_IR_AUDIOFORMAT_STEREO, VAS_IR_LINEFORMAT_IR, 0, 0);
}

void vas_firobject_loadIr2ArrayIndex(rwa_firobject *x, t_symbol *left, float index)
{
    vas_fir *engine = x->convolutionEngine;
    int filterLength = 0;
    int currentIndex2Write = 0;
     
    filterLength = ((vas_fir *)engine)->metaData.filterLength;
    float *currentIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    engine->left->filter->filterLength = filterLength;
    vas_firobject_getFloatArrayAndLength(x, left, &x->leftArray, &x->leftArrayLength);
       
    currentIndex2Write = 0;
    for (int i=0;i<filterLength;i++)
        currentIr[currentIndex2Write++] = x->leftArray[i].w_float;
    
    vas_dynamicFirChannel_prepareFilter(engine->left, currentIr, 0, index);
    vas_dynamicFirChannel_prepareFilter(engine->right, currentIr, 0, index);
    ((vas_dynamicFirChannel *) (engine->left))->filter->indexIsZero[0][(int)index] = false;
    
    vas_fir_setInitFlag((vas_fir *)engine);
    vas_mem_free(currentIr);
}

void vas_firobject_setAndInterpolateBetweenIndexes1(rwa_firobject *x, t_symbol *s, int argc, t_atom *argv)
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
            vas_firobject_getFloatArrayAndLength(x, argv[i].a_w.w_symbol, &x->leftArray, &x->leftArrayLength);
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

        vas_firobject_getFloatArrayAndLength(x, argv[i].a_w.w_symbol, &x->leftArray, &x->leftArrayLength);
        vas_firobject_getFloatArrayAndLength(x, argv[i+2].a_w.w_symbol, &x->rightArray, &x->rightArrayLength);
          
        maxLengthLeft = MIN(x->leftArrayLength, filterLength);
        for (int i=0;i<maxLengthLeft;i++)
        {
            startIr[i] = x->leftArray[i].w_float;
        }

        maxLengthRight = MIN(x->rightArrayLength, filterLength);
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

void vas_firobject_setAndInterpolateBetweenIndexes(rwa_firobject *x, t_symbol *start, t_symbol *end, float startIndex, float endIndex, float mode)
{
    vas_fir *engine = x->convolutionEngine;
    int filterLength = 0;
    
    float distance = endIndex - startIndex + 1;
    int weightFirstIR = 0;
    int weightSecondIR = 0;
    int normIndex = 0;
    int maxLengthLeft = 0;
    int maxLengthRight = 0;
    
    filterLength = ((vas_fir *)engine)->metaData.filterLength;
    float *startIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    float *endIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    float *finalIr = (float *)vas_mem_alloc( filterLength * sizeof(float));
    engine->left->filter->filterLength = filterLength;
    vas_firobject_getFloatArrayAndLength(x, start, &x->leftArray, &x->leftArrayLength);
    vas_firobject_getFloatArrayAndLength(x, end, &x->rightArray, &x->rightArrayLength);
      
    maxLengthLeft = MIN(x->leftArrayLength, filterLength);
    for (int i=0;i<maxLengthLeft;i++)
    {
        startIr[i] = x->leftArray[i].w_float;
    }
    
    maxLengthRight = MIN(x->rightArrayLength, filterLength);
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
    
    vas_fir_setInitFlag((vas_fir *)engine);
    vas_mem_free(startIr);
    vas_mem_free(endIr);
    vas_mem_free(finalIr);
}

void vas_firobject_set_mono_simple(rwa_firobject *x, t_symbol *left, float segmentSize)
{
    vas_fir *engine = x->convolutionEngine;
    int filterLenghtMinusOffset = 0;
    int minLength = 0;
    int currentIndex2Write = 0;
    
    x->segmentSize = segmentSize;
    vas_firobject_getFloatArrayAndLength(x, left, &x->leftArray, &x->leftArrayLength);
    
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

void vas_firobject_set1(rwa_firobject *x, t_symbol *left, t_symbol *right, float segmentSize, float offset, float end)
{
    vas_fir *engine = x->convolutionEngine;
    int filterLenghtMinusOffset = 0;
    int minLength = 0;
    int currentIndex2Write = 0;
    
    x->segmentSize = segmentSize;
    vas_firobject_getFloatArrayAndLength(x, left, &x->leftArray, &x->leftArrayLength);
    vas_firobject_getFloatArrayAndLength(x, right, &x->rightArray, &x->rightArrayLength);
    
    minLength = x->leftArrayLength;
    if(x->rightArrayLength < minLength)
        minLength = x->rightArrayLength;

    vas_fir_setMetaData_manually1((vas_fir *)engine, minLength, segmentSize, VAS_IR_DIRECTIONFORMAT_SINGLE, 1, 1, VAS_IR_AUDIOFORMAT_STEREO, VAS_IR_LINEFORMAT_IR, offset, end);
        
    filterLenghtMinusOffset = ((vas_fir *)engine)->metaData.filterLength-offset;
    float *currentIr = (float *)vas_mem_alloc( filterLenghtMinusOffset * sizeof(float));
    engine->left->filter->filterLength = filterLenghtMinusOffset;
    engine->right->filter->filterLength = filterLenghtMinusOffset;
       
    currentIndex2Write = 0;
    for (int i=offset;i<filterLenghtMinusOffset;i++)
        currentIr[currentIndex2Write++] = x->leftArray[i].w_float;
    
    vas_dynamicFirChannel_prepareFilter(engine->left, currentIr, 0, 0);
    
    currentIndex2Write = 0;
    for (int i=offset;i<filterLenghtMinusOffset;i++)
        currentIr[currentIndex2Write] = x->rightArray[i].w_float;
    
    vas_dynamicFirChannel_prepareFilter(engine->right, currentIr, 0, 0);
    
    vas_fir_setInitFlag((vas_fir *)engine);
    vas_mem_free(currentIr);
}

void rwa_firobject_read2(rwa_firobject *x, t_symbol *s, float segmentSize, float offset, float end)
{
    const char *fileExtension;
    const char *filename = s->s_name;
    
    vas_fir *engine = x->convolutionEngine;
    if(segmentSize)
        x->segmentSize = segmentSize;
    
    if(engine->left->filter->referenceCounter > 1)
    {
        post("Another instance is referencing this filter.");
        return;
    }
    
    unsigned long length = strlen(x->canvasDirectory);
    
    if(x->canvasDirectory[length-1] == '/')
        sprintf(x->fullpath, "%s%s", x->canvasDirectory, filename);
    else
        sprintf(x->fullpath, "%s/%s", x->canvasDirectory, filename);
    
    post("IR Path: %s", x->fullpath);
    fileExtension = vas_util_getFileExtension(filename);
    
    if(!strcmp(fileExtension, "sofa"))
    {
#ifdef VAS_USE_LIBMYSOFA
        
        void *filter = vas_fir_readSofa_getMetaData(engine, x->fullpath);
        if(filter)
        {
            vas_fir_setMultiDirection3DegreeGridResoluion(engine, engine->metaData.filterLength, segmentSize, 0, 0);
            vas_fir_initFilter2((vas_fir *)engine, x->segmentSize);
            vas_fir_readSofa_getFilter(engine, filter);
            vas_fir_setInitFlag((vas_fir *)engine);
        }
#else
        post("Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA");
#endif
    }
    if(!strcmp(fileExtension, "txt"))
    {
        if(vas_fir_getInitFlag(engine))
        {
            vas_fir_list_removeNode(&IRs, engine->metaData.fullPath);
            post("remove current filter node");
        }
            
        vas_fir *existingFilter = vas_fir_list_find1(&IRs, x->fullpath, segmentSize, offset, end);
        
        if(existingFilter != NULL)
        {
            size_t size = strlen(existingFilter->metaData.fullPath);
            engine->metaData.fullPath = vas_mem_alloc(sizeof(char) * size);
            strcpy(engine->metaData.fullPath, existingFilter->metaData.fullPath);
            vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, engine->left, engine->right);
            vas_fir_setInitFlag((vas_fir *)engine);
            post("Use existing filter");
            return;
        }
        
        FILE *file = vas_fir_readText_metaData1((vas_fir *)engine, x->fullpath);
        if(file)
        {
            post("Load Filter from File with segmenSize: %d", x->segmentSize);
            
            ((vas_fir *)engine)->metaData.filterOffset = offset;
            ((vas_fir *)engine)->metaData.segmentSize = segmentSize;
            
            if(end > 0 && (end < ((vas_fir *)engine)->metaData.filterLength) )
                ((vas_fir *)engine)->metaData.filterLength = end;
            
            ((vas_fir *)engine)->metaData.filterLengthMinusOffset = ((vas_fir *)engine)->metaData.filterLength - ((vas_fir *)engine)->metaData.filterOffset;
            vas_fir_initFilter2((vas_fir *)engine, x->segmentSize);
            vas_fir_readText_Ir1((vas_fir *)engine, file, offset); // move fclose out of this function..
            vas_fir_list_addNode(&IRs, vas_fir_listNode_new(engine));
            vas_fir_setInitFlag((vas_fir *)engine);
        }
        else
            error("Could not open %s", x->fullpath);
    }
}
