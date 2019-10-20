//
//  vas_linkedlist.h
//  rwa_binauralsimple~
//
//  Created by Harvey Keitel on 08.10.19.
//  Copyright © 2019 Intrinsic Audio. All rights reserved.
//

#ifndef vas_linkedlist_h
#define vas_linkedlist_h

#include <stdio.h>
#include "vas_fir.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_fir_listNode {
    vas_fir *data;
    struct vas_fir_listNode* next;
} vas_fir_listNode;

typedef struct vas_fir_list {
    vas_fir_listNode *firstElement;
    vas_fir_listNode *lastElement;
} vas_fir_list;



vas_fir_listNode *vas_fir_listNode_new(vas_fir *data);

void vas_fir_list_clear(vas_fir_list *x);

void vas_fir_listNode_free(vas_fir_listNode *node);

void vas_fir_list_removeNode1(vas_fir_list *x, vas_fir *match);

void vas_fir_list_addNode(vas_fir_list *x, vas_fir_listNode *node);

vas_fir *vas_fir_list_find(vas_fir_list *x, const char *match);

void vas_fir_list_removeNode(vas_fir_list *x, const char *match);

#ifdef __cplusplus
}
#endif

#endif /* vas_linkedlist_h */


