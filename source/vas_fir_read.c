//
//  vas_fir_read.c
//  vas_binaural~
//
//  Created by Thomas Resch on 18.02.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#include "vas_fir_read.h"

extern vas_fir_list IRs;

void vas_fir_read_singleImpulseFromFloatArray(vas_fir *x, char *name, float *left, float *right, float length, int segmentSize, int offset, int end)
{
    int ele = 0;
    int azi = 0;
    
    if(vas_fir_getInitFlag(x))
    {
        vas_fir_list_removeNode(&IRs, x->metaData.fullPath);
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("remove current filter node");
#endif
    }
        
    vas_fir *existingFilter = vas_fir_list_find1(&IRs, name, segmentSize, offset, end);
    
    if(existingFilter != NULL)
    {
        size_t size = strlen(existingFilter->metaData.fullPath);
        x->metaData.fullPath = vas_mem_alloc(sizeof(char) * size);
        strcpy(x->metaData.fullPath, existingFilter->metaData.fullPath);
        vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
        vas_fir_setInitFlag((vas_fir *)x);
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Use existing filter");
#endif
        return;
    }

    vas_fir_setMetaData_manually1(x, length, segmentSize, VAS_IR_DIRECTIONFORMAT_SINGLE, 1, 1, VAS_IR_AUDIOFORMAT_STEREO, VAS_IR_LINEFORMAT_IR, offset, end);
    x->metaData.fullPath = vas_mem_alloc(sizeof(char) * strlen(name));
    strcpy(x->metaData.fullPath, name);
    vas_dynamicFirChannel_prepareFilter(x->left, left, ele, azi);
    vas_dynamicFirChannel_prepareFilter(x->right, right, ele, azi);
    vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
    vas_fir_setInitFlag((vas_fir *)x);
}

void vas_fir_read_impulseFromFile(vas_fir *x, char *fullpath, int segmentSize, int offset, int end)
{
    const char *fileExtension;
    fileExtension = vas_util_getFileExtension(fullpath);
    if(!strcmp(fileExtension, "sofa"))
    {
#ifdef VAS_USE_LIBMYSOFA
        void *filter = vas_fir_readSofa_getMetaData(x, fullpath);
        if(filter)
        {
            if(vas_fir_getInitFlag(x))
            {
#ifndef VAS_USE_MULTITHREADED_LOADING
                vas_fir_list_removeNode(&IRs, x->metaData.fullPath);
#if defined(MAXMSPSDK) || defined(PUREDATA)
                post("remove current filter node");
#endif
#endif
            }
                
            vas_fir *existingFilter = NULL;
#ifndef VAS_USE_MULTITHREADED_LOADING
            existingFilter = vas_fir_list_find1(&IRs, fullpath, segmentSize, offset, end);
#endif
            if(existingFilter != NULL)
            {
                size_t size = strlen(existingFilter->metaData.fullPath);
                x->metaData.fullPath = vas_mem_alloc(sizeof(char) * size);
                strcpy(x->metaData.fullPath, existingFilter->metaData.fullPath);
                vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
                vas_fir_setInitFlag((vas_fir *)x);
#if defined(MAXMSPSDK) || defined(PUREDATA)
                post("Use existing filter");
#endif
                return;
            }
            
            vas_fir_setMultiDirection3DegreeGridResoluion(x, x->metaData.filterLength, segmentSize, offset, end);
            vas_fir_readSofa_getFilter(x, filter);
#ifndef VAS_USE_MULTITHREADED_LOADING
            vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
#endif
            vas_fir_setInitFlag((vas_fir *)x);
        }
#else
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post("Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA");
#endif
#endif
    }
    
    if(!strcmp(fileExtension, "txt"))
    {
        FILE *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
        
        if(file)
        {
            if(vas_fir_getInitFlag(x))
            {
#ifndef VAS_USE_MULTITHREADED_LOADING
                vas_fir_list_removeNode(&IRs, x->metaData.fullPath);
#if defined(MAXMSPSDK) || defined(PUREDATA)
                post("remove current filter node");
#endif
#endif
            }
            
            vas_fir *existingFilter = NULL;
#ifndef VAS_USE_MULTITHREADED_LOADING
            existingFilter = vas_fir_list_find1(&IRs, fullpath, segmentSize, offset, end);
#endif
            if(existingFilter != NULL)
            {
                size_t size = strlen(existingFilter->metaData.fullPath);
                x->metaData.fullPath = vas_mem_alloc(sizeof(char) * size);
                strcpy(x->metaData.fullPath, existingFilter->metaData.fullPath);
                vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
                vas_fir_setInitFlag((vas_fir *)x);
#if defined(MAXMSPSDK) || defined(PUREDATA)
                post("Use existing filter");
#endif
                fclose(file);
                return;
            }
#if defined(MAXMSPSDK) || defined(PUREDATA)
            post("Load Filter from File with segmenSize: %d", segmentSize);
#endif
            
            vas_fir_setAdditionalMetaData(x, segmentSize, offset, end);
            vas_fir_readText_Ir2((vas_fir *)x, file); // move fclose out of this function..
#ifndef VAS_USE_MULTITHREADED_LOADING
            vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
#endif
            vas_fir_setInitFlag((vas_fir *)x);
            fclose(file);
        }
        else
        {
#if defined(MAXMSPSDK) || defined(PUREDATA)
            error("Could not open %s", fullpath);
#endif
        }
    }
}

