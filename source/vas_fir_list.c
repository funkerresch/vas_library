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

vas_fir_listNode *vas_fir_listNode_new(vas_fir *data)
{
    vas_fir_listNode *x = vas_mem_alloc(sizeof(vas_fir_listNode));
    x->data = data;
    x->next = NULL;
    return x;
}

void vas_fir_listNode_free(vas_fir_listNode *node)
{
    if(node != NULL)
        vas_mem_free(node);
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

vas_fir *vas_fir_list_find(vas_fir_list *x, const char *match)
{
    vas_fir_listNode *current = x->firstElement;
    while(current)
    {
        if(current->data->description.fullPath != NULL)
        {
            if(!strcmp(current->data->description.fullPath, match))
                return current->data;
        }
        
        current = current->next;
    }
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

void vas_fir_list_removeNode1(vas_fir_list *x, vas_fir *match)
{
    vas_fir_listNode *current = x->firstElement;
    vas_fir_listNode *last = NULL;
    
    while(current)
    {
        if(current->data == match)
        {
           // post("Remove: %s", current->data->description.fullPath);
            
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

void vas_fir_list_removeNode(vas_fir_list *x, const char *match)
{
    vas_fir_listNode *current = x->firstElement;
    vas_fir_listNode *last = NULL;
    
    while(current)
    {
        if(!strcmp(current->data->description.fullPath, match))
        {
           // post("Remove: %s", current->data->description.fullPath);
            
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



