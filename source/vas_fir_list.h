/**
 * @file vas_fir_list.h
 * @author Thomas Resch <br>
 * Audio Communication Group, TU-Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based virtual acoustics (mainly dynamic binaural synthesis) <br>
 * <br>
 * @brief Minimal linked list for fir objects.<br>
 * Allows for sharing an IR set with another fir object if <br>
 * the corresponding IR set is already calculated and loaded<br>
 * <br>
 */

#ifndef vas_linkedlist_h
#define vas_linkedlist_h

#include <stdio.h>
#include "vas_fir.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_fir_idData
{
    char *name;
    int segmentSize;
    int offset;
    int end;
} vas_fir_idData;

vas_fir_idData *vas_fir_idData_new(const char *name, int segmentSize, int offset, int end);

void vas_fir_idData_free(vas_fir_idData *x);

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

void vas_fir_list_removeNodeByAdress(vas_fir_list *x, vas_fir *match);

void vas_fir_list_removeNode2(vas_fir_list *x, vas_fir_idData *idData);

void vas_fir_list_addNode(vas_fir_list *x, vas_fir_listNode *node);

vas_fir *vas_fir_list_find1(vas_fir_list *x, const char *match, int segmentSize, int offset, int end);

#ifdef __cplusplus
}
#endif

#endif /* vas_linkedlist_h */


