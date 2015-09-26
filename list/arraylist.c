//
// Created by Tom Werner on 8/26/15.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "arraylist.h"

ArrayList *alCreateList(int initialSize) {
    ArrayList *result = malloc (sizeof (ArrayList));
    if (result == NULL) {
        perror("Could not initialize list memory.");
        return NULL;
    }
    if (initialSize <= 0) {
        perror("Cannot initialize a list with size <= 0");
        return NULL;
    }

    int* dataResult = malloc(sizeof(int) * initialSize);
    if (dataResult == NULL) {
        perror("Could not initialize list data memory.");
        return NULL;
    }

    result->size = 0;
    result->capacity = initialSize;
    result->data = dataResult;

    return result;
}

bool alAdd(ArrayList *list, int data) {
    if (list == NULL) {
        return false;
    }
    return alAddAt(list, data, list->size);
}

bool doubleCapacity(ArrayList* list) {
    list->capacity *= 2; // Double capacity
    int* dataResult = malloc(sizeof(int) * list->capacity);
    if (dataResult == NULL) {
        perror("Could not increase list memory.");
        return false;
    }
    memcpy(dataResult, list->data, sizeof(int) * list->size);
    free(list->data);
    list->data = dataResult;
    return true;
}

bool alAddAt(ArrayList *list, int data, int index) {
    if (list == NULL) {
        perror("Cannot add data to a null list.");
        return false;
    }
    if (list->size < list->capacity) {
        if (index > list->size) {
            perror("Cannot add data to size + 1 index.");
            return false;
        }
        else if (index == list->size) { // Normal add to end
            list->data[list->size] = data;
        }
        else { // Adding in the middle
            // We need to shift everything down
            for (int i = list->size; i > index; i--) {
                list->data[i] = list->data[i - 1];
            }
            // Set the new value
            list->data[index] = data;
        }
        list->size++;
    }
    else {
        bool worked = doubleCapacity(list);
        if (!worked)
            return false;
        return alAddAt(list, data, index);
    }
    return true;
}

bool alSet(ArrayList *list, int data, int index) {
    if (list == NULL) {
        return false;
    }
    if (index < list->size) {
        list->data[index] = data;
        return true;
    }

    return false;
}

int alGet(ArrayList *list, int index) {
    if (list == NULL) {
        perror("Cannot get data from a null list.");
        return 0;
    }
    if (index < list->size) {
        return list->data[index];
    }
    perror("Out of bounds exception.");
    return 0;
}

int alIndexOf(ArrayList *list, int value) {
    if (list == NULL) {
        return -1;
    }
    for (int i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return i;
        }
    }
    return -1;
}

int alRemove(ArrayList *list, int index) {
    if (list == NULL) {
        return 0;
    }
    int oldData = list->data[index];
    for (int i = index; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }
    list->size--;

    return oldData;
}

int alSize(ArrayList *list) {
    if (list == NULL) {
        perror("Cannot get the size of a null list.");
        return 0;
    }
    return list->size;
}

int alCapacity(ArrayList *list) {
    if (list == NULL) {
        perror("Cannot get the capacity of a null list.");
        return 0;
    }
    return list->capacity;
}

bool alIsEmpty(ArrayList *list) {
    return alSize(list) == 0;
}

void alDelete(ArrayList *list) {
    free(list->data);
    free(list);
}
