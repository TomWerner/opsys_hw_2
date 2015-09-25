//
// Created by Tom Werner on 8/26/15.
//

#ifndef TESTPROJ_ARRAYLIST_H
#define TESTPROJ_ARRAYLIST_H

#include <stdbool.h>

typedef struct arrayList {
    int size;
    int capacity;
    int* data;
} ArrayList;

ArrayList* alCreateList(int initialSize);
bool alAdd(ArrayList* list, int data);
bool alAddAt(ArrayList* list, int data, int index);
bool alSet(ArrayList* list, int data, int index);
int alGet(ArrayList* list, int index);
int alIndexOf(ArrayList* list, int value);
int alRemove(ArrayList* list, int index);
int alSize(ArrayList* list);
int alCapacity(ArrayList* list);
bool alIsEmpty(ArrayList* list);

#endif //TESTPROJ_ARRAYLIST_H
