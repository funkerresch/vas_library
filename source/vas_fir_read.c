//
//  vas_fir_read.c
//  vas_binaural~
//
//  Created by Thomas Resch on 18.02.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#include "vas_fir_read.h"

#ifdef VAS_USE_NEUMANNHEADERS
#include "vas_fir_neumannFilter_48kHz.c"
#endif

extern vas_fir_list IRs;

int vas_fir_read_sofa_0Degrees(char *fullpath, float *leftIR, float *rightIR)
{
    int err = -1;
#ifdef VAS_USE_LIBMYSOFA
    struct MYSOFA_EASY *hrtf = NULL;
    int filterLength;
    hrtf = mysofa_open(fullpath, 44100, &filterLength, &err);
    
    leftIR = vas_mem_alloc(sizeof(float) * filterLength);
    rightIR = vas_mem_alloc(sizeof(float) * filterLength);

    float leftDelay;          // unit is sec.
    float rightDelay;         // unit is sec.
             
    mysofa_getfilter_float(hrtf , 0, 0, 0, leftIR, rightIR, &leftDelay, &rightDelay);
    mysofa_close(hrtf);
#endif
    return err;
}

bool vas_fir_read_set2ExistingFilter(vas_fir *x, char *fullpath, int segmentSize, int offset, int end)
{
    vas_fir *existingFilter = NULL;
    
#ifndef VAS_USE_MULTITHREADED_LOADING
    existingFilter = vas_fir_list_find1(&IRs, fullpath, segmentSize, offset, end);
#endif
    
    if(existingFilter != NULL)
    {
        vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
        vas_fir_setInitFlag((vas_fir *)x);
        vas_util_debug("%s: Using existing filter", __FUNCTION__);
        return true;
    }
    else
        return false;
}

void vas_fir_read_singleImpulseFromFloatArray(vas_fir *x, char *name, float *left, float *right, float length, int segmentSize, int offset, int end, bool filterSharingActive)
{
    if(offset > length)
        return;
    
    vas_fir_idData *idData = NULL;
     
    if(filterSharingActive && vas_fir_getInitFlag(x))
    {
        idData = vas_fir_idData_new(x->metaData.fullPath, x->metaData.segmentSize, x->metaData.filterOffset, x->metaData.filterEnd);
        vas_util_debug("%s: Filter was already initialized, saving old identifying meta data.", __FUNCTION__);
        vas_fir_list_removeNode2(&IRs, idData);
        vas_fir_idData_free(idData);
        vas_util_debug("%s: Removing old filter node.", __FUNCTION__);
    }
    
    if(filterSharingActive && vas_fir_read_set2ExistingFilter(x, name, segmentSize, offset, end))
        vas_util_debug("%s: Using existing filter", __FUNCTION__);
    else
    {
        vas_fir_setMetaData_manually1(x, name, length, segmentSize, VAS_IR_DIRECTIONFORMAT_SINGLE, 1, 1, VAS_IR_AUDIOFORMAT_STEREO, VAS_IR_LINEFORMAT_IR, offset, end);
        vas_dynamicFirChannel_prepareFilter(x->left, left+offset, 0, 0);
        vas_dynamicFirChannel_prepareFilter(x->right, right+offset, 0, 0);
        
        if(filterSharingActive)
            vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
        
        vas_fir_setInitFlag((vas_fir *)x);
    }
}

void vas_fir_read_checkFileExtensionAndReadMetaData(vas_fir *x, char *fullpath, vas_fir_idData **idData, void **filter, FILE **file)
{
    const char *fileExtension = vas_util_getFileExtension(fullpath);
    
    if(vas_fir_getInitFlag(x))
    {
        *idData = vas_fir_idData_new(x->metaData.fullPath, x->metaData.segmentSize, x->metaData.filterOffset, x->metaData.filterEnd);
        vas_util_debug("%s: Filter was already initialized, saving old identifying meta data.", __FUNCTION__);
    }
    
    if(!strcmp(fileExtension, "txt"))
        *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
    else
    {
        if(!strcmp(fileExtension, "sofa"))
        {
#ifdef VAS_USE_LIBMYSOFA
            *filter = vas_fir_readSofa_getMetaData(x, fullpath);
#else
            vas_util_debug("%s: Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA", __FUNCTION__);
#endif
        }
    }
}

bool vas_fir_read_impulseFromHeader(vas_fir *x, char *fullpath)
{
#ifdef VAS_USE_NEUMANNHEADERS
    if(!strcmp(fullpath, "neumann48"))
    {
        if(neumannHrtfInit_48kHz == 1)
        {
            vas_fir_prepareChannelsWithSharedFilter((vas_fir *)neumannHrtf_48kHz, x->left, x->right);
            vas_fir_setInitFlag((vas_fir *)x);
            vas_fir_setMetaDataForNeumannHeader(x);
            vas_util_debug("%s: Using included Neumann filter", __FUNCTION__);
            return true;
        }
        return true;
    }
    else
        return false;
#else
    return false;
#endif
}

void vas_fir_read_impulseFromFile(vas_fir *x, char *fullpath, int segmentSize, int offset, int end)
{
    vas_fir_idData *idData = NULL;
    void *filter = NULL;
    FILE *file = NULL;
    
    vas_util_debug("%s", __FUNCTION__);
    
    if(vas_fir_read_impulseFromHeader(x, fullpath))
        return;
    
    vas_fir_read_checkFileExtensionAndReadMetaData(x, fullpath, &idData, &filter, &file);
    
    if(filter || file)
    {
        if(vas_fir_getInitFlag(x))
        {
#ifndef VAS_USE_MULTITHREADED_LOADING
            vas_fir_list_removeNode2(&IRs, idData);
            vas_util_debug("%s: Removing old filter node from list.", __FUNCTION__);
#endif
        }
        
        if(vas_fir_read_set2ExistingFilter(x, fullpath, segmentSize, offset, end))
            vas_util_debug("%s: Using existing filter", __FUNCTION__);
        else
        {
            vas_util_debug("%s: Loading new filter from File with segmenSize: %d", __FUNCTION__, segmentSize);
            if(file)
            {
                vas_fir_setAdditionalMetaData(x, fullpath, segmentSize, offset, end);
                vas_fir_readText_Ir2((vas_fir *)x, file); // move fclose out of this function..
            }
            else
            {
#ifdef VAS_USE_LIBMYSOFA
                vas_fir_setMultiDirection3DegreeGridResoluion(x, fullpath, x->metaData.filterLength, segmentSize, offset, end);
                vas_fir_readSofa_getFilter(x, filter);
#endif
            }
            
#ifndef VAS_USE_MULTITHREADED_LOADING
            vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
#endif
            vas_fir_setInitFlag((vas_fir *)x);
        }
    }
    
    if(idData)
        free(idData);
    if(file)
        fclose(file);
    if(filter)
    {
#ifdef VAS_USE_LIBMYSOFA
        mysofa_close(filter);
#endif
    }
}


