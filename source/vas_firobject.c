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
        if (*arrayname->s_name) pd_error(x, "vas_binaural~: %s: no such array",
            arrayname->s_name);
        *array = 0;
    }
    else if (!garray_getfloatwords(a, length, array))
    {
        pd_error(x, "%s: bad template for vas_binaural~", arrayname->s_name);
        *array = 0;
    }
    else
    {
        post("Reading IRs from array %s", arrayname->s_name);
    }
}

 // very quick and dirty reading from pd array

void vas_firobject_set(rwa_firobject *x, t_symbol *left, t_symbol *right)
{
    vas_fir *engine = x->convolutionEngine;
    vas_firobject_getFloatArrayAndLength(x, left, &x->leftArray, &x->leftArrayLength);
    vas_firobject_getFloatArrayAndLength(x, right, &x->rightArray, &x->rightArrayLength);
    vas_fir_setMetaData((vas_fir *)engine, VAS_IR_DIRECTIONFORMAT_SINGLE, x->leftArrayLength);
    vas_fir_initFilter2((vas_fir *)engine, x->segmentSize, 0);
    float *currentIr = (float *)vas_mem_alloc( x->leftArrayLength * sizeof(float));
    
    for (int i=0;i<x->leftArrayLength;i++)
        currentIr[i] = x->leftArray[i].w_float;
    
    vas_dynamicFirChannel_prepareFilter(engine->left, currentIr, 0, 0);
    
    for (int i=0;i<x->rightArrayLength;i++)
        currentIr[i] = x->rightArray[i].w_float;
    
    vas_dynamicFirChannel_prepareFilter(engine->right, currentIr, 0, 0);
    vas_fir_setInitFlag((vas_fir *)engine);
    //post("%d %d", x->leftArrayLength, x->rightArrayLength);
    
    vas_mem_free(currentIr);
}

void rwa_firobject_read2(rwa_firobject *x, t_symbol *s, float segmentSize, float offset)
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
            vas_fir_initFilter2((vas_fir *)engine, x->segmentSize, 0);
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
            vas_fir_list_removeNode(&IRs, engine->description.fullPath);
            post("remove current filter node");
        }
            
        vas_fir *existingFilter = vas_fir_list_find(&IRs, x->fullpath);
        
        if(existingFilter != NULL)
        {
            if(segmentSize == existingFilter->left->filter->segmentSize)
            {
                size_t size = strlen(existingFilter->description.fullPath);
                engine->description.fullPath = vas_mem_alloc(sizeof(char) * size);
                strcpy(engine->description.fullPath, existingFilter->description.fullPath);
                vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, engine->left, engine->right);
                vas_fir_setInitFlag((vas_fir *)engine);
                return;
            }
        }
        FILE *file = vas_fir_readText_metaData1((vas_fir *)engine, x->fullpath);
        if(file)
        {
            post("Load Filter from File with segmenSize: %d", x->segmentSize);
            vas_fir_initFilter2((vas_fir *)engine, x->segmentSize, offset);
            vas_fir_readText_Ir1((vas_fir *)engine, file, offset); // move fclose out of this function..
            vas_fir_list_addNode(&IRs, vas_fir_listNode_new(engine));
            vas_fir_setInitFlag((vas_fir *)engine);
        }
    }
}
