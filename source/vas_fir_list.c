//
//  vas_linkedlist.c
//  rwa_binauralsimple~
//
//  Created by Harvey Keitel on 08.10.19.
//  Copyright © 2019 Intrinsic Audio. All rights reserved.
//

#include "vas_fir_list.h"
#include "vas_mem.h"

#ifdef __cplusplus
extern "C" {
#endif
vas_fir_list IRs;
#ifdef __cplusplus
}
#endif

vas_fir_idData *vas_fir_idData_new(const char *name, int segmentSize, int offset, int end)
{
    size_t size = strlen(name) + 1;
    vas_fir_idData *x = malloc(sizeof(vas_fir_idData));
    x->name = malloc(size);
    strcpy(x->name, name);
    x->segmentSize = segmentSize;
    x->offset = offset;
    x->end = end;
    return x;
}

void vas_fir_idData_free(vas_fir_idData *x)
{
    free(x->name);
    free(x);
}

vas_fir_listNode *vas_fir_listNode_new(vas_fir *data)
{
    vas_fir_listNode *x = malloc(sizeof(vas_fir_listNode));
    x->data = data;
    x->next = NULL;
    return x;
}

void vas_fir_listNode_free(vas_fir_listNode *node)
{
    if(node != NULL)
        free(node);
}

void vas_fir_list_addNode(vas_fir_list *x, vas_fir_listNode *node)
{
    if(x->firstElement == NULL)
    {
        x->firstElement = node;
        x->lastElement = node;
    }
    else
    {
        x->lastElement->next = node;
        x->lastElement = node;
    }
}

vas_fir *vas_fir_list_find1(vas_fir_list *x, const char *match, int segmentSize, int offset, int end)
{
    int strcmpResult = -1;
    vas_fir_listNode *current = x->firstElement;
    while(current)
    {
        if(current->data->metaData.fullPath != NULL)
        {
            vas_util_debug("%s: Comparing %s & %s", __FUNCTION__, match, current->data->metaData.fullPath);
            vas_util_debug("%s: Comparing segment size: %d & %d", __FUNCTION__, current->data->metaData.segmentSize, segmentSize);
            vas_util_debug("%s: Comparing filter offset: %d & %d", __FUNCTION__, current->data->metaData.filterOffset, offset);
            vas_util_debug("%s: Comparing filter end: %d & %d", __FUNCTION__,  current->data->metaData.filterEnd, end);
            if(!(strcmpResult = strcmp(current->data->metaData.fullPath, match))
               && current->data->metaData.segmentSize == segmentSize
               && current->data->metaData.filterOffset == offset
               && current->data->metaData.filterEnd == end)
            {
                vas_util_debug("%s: Found Match for  %s", __FUNCTION__, current->data->metaData.fullPath);
                return current->data;
            }
        }
        
        current = current->next;
    }
    vas_util_debug("%s: Did not find Match: %d", __FUNCTION__, strcmpResult);
    return NULL;
}

void vas_fir_list_clear(vas_fir_list *x)
{
    vas_fir_listNode *current = x->firstElement;
    vas_fir_listNode *tmp = NULL;
    
    while(current)
    {
        tmp = current;
        current = current->next;
        vas_fir_listNode_free(tmp);
    }
    x->firstElement = NULL;
    x->lastElement = NULL;
}

void vas_fir_list_removeNodeByAdress(vas_fir_list *x, vas_fir *match)
{
    vas_fir_listNode *current = x->firstElement;
    vas_fir_listNode *last = NULL;
    
    while(current)
    {
        if(current->data == match)
        {
            if(last != NULL)
                last->next = current->next;
            else
                x->firstElement = NULL;
            
            if(current->next == NULL)
                x->lastElement = last;
                 
            vas_fir_listNode_free(current);
            
            return;
        }
        else
        {
            last = current;
            current = current->next;
        }
    }
}

void vas_fir_list_removeNode2(vas_fir_list *x, vas_fir_idData *idData)
{
    vas_fir_listNode *current = x->firstElement;
    vas_fir_listNode *last = NULL;
    
    while(current)
    {
        if(!strcmp(current->data->metaData.fullPath, idData->name)
           && current->data->metaData.segmentSize == idData->segmentSize
           && current->data->metaData.filterOffset == idData->offset
           && current->data->metaData.filterEnd == idData->end)
        {
            vas_util_debug("%s: Remove: %s", __FUNCTION__, current->data->metaData.fullPath);
            
            if(last != NULL)
                last->next = current->next;
            else
                x->firstElement = NULL;
            
            if(current->next == NULL)
                x->lastElement = last;
                 
            vas_fir_listNode_free(current);
            
            return;
        }
        else
        {
            last = current;
            current = current->next;
        }
    }
}




