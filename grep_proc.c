#define _GNU_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct arrayList {
    int size;
    int capacity;
    int* data;
} ArrayList;

ArrayList* alCreateList(int initialSize);
bool alAdd(ArrayList* list, int data);
bool alAddAt(ArrayList* list, int data, int index);
int alGet(ArrayList* list, int index);
int alSize(ArrayList* list);
void alDelete(ArrayList *list);

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

int alSize(ArrayList *list) {
    if (list == NULL) {
        perror("Cannot get the size of a null list.");
        return 0;
    }
    return list->size;
}

void alDelete(ArrayList *list) {
    free(list->data);
    free(list);
}

typedef enum regexType {
    TYPE_SINGLE,
    TYPE_STAR,
    TYPE_PLUS
} RegexType;

typedef enum regexChar {
    CHAR_NORMAL,
    CHAR_WHITESPACE,
    CHAR_STAR,
    CHAR_PLUS,
    CHAR_BACKSLASH
} RegexChar;

typedef struct regexItem {
    struct regexItem* next;
    char chr;
    RegexType regexType;
    RegexChar regexChar;
    bool satisfied;
} RegexItem;

RegexItem *preproccessRegex(char *regex);
ArrayList* matchingPositions(char* line, char* regex);

RegexItem *createRegexItem(char chr, RegexType regexType, RegexChar regexChar);

bool characterSatisfies(RegexItem *pattern, char chr);

ArrayList *matchingPositions(char *line, char *regex) {
    ArrayList* result = alCreateList(16);
    RegexItem* pattern = preproccessRegex(regex);

    for (int i = 0; i < strlen(line); i++) {
        int offset = i;
        RegexItem* iterator = pattern;
        bool noIssues = true;
        while (iterator != NULL && noIssues) {
            if (!characterSatisfies(iterator, line[offset])) { // This character doesn't work
//                printf("Pattern %c wasn't satisfied by %c in %s at %d\n", iterator->chr, line[offset], line, offset);
                if (iterator->satisfied) {// We already satisfied this part of the regex
                    iterator->satisfied = iterator->regexType == TYPE_STAR; // Reset satisfied
                    iterator = iterator->next;
                }
                else {
//                    printf("Exiting loop with pattern %c on character %c, %s %d\n", iterator->chr, line[offset], line, offset);
                    noIssues = false;
                }
            }
            else {// We satisfied this part, check the next character
                offset++;
                iterator->satisfied = true;
            }
        }
        if (noIssues)
            alAdd(result, i);
    }


    return result;
}

bool characterSatisfies(RegexItem *pattern, char chr) {
    switch (pattern->regexChar) {
        case CHAR_NORMAL:
            return chr == pattern->chr;
        case CHAR_WHITESPACE:
            return chr == ' ' || chr == '\t';
        case CHAR_BACKSLASH:
            return chr == '\\';
        case CHAR_PLUS:
            return chr == '+';
        case CHAR_STAR:
            return chr == '*';
    }
    return false;
}

RegexItem *preproccessRegex(char *regex) {
    size_t len = strlen(regex);
    RegexItem* result = NULL;
    RegexItem* newest = NULL;

    for (int i = 0; i < len; i++) {
        RegexItem* newItem = NULL;
        if (regex[i] == '\\') {
            RegexChar regChr = CHAR_NORMAL;
            if (regex[i + 1] == '\\')
                regChr = CHAR_BACKSLASH;
            else if (regex[i + 1] == 's')
                regChr = CHAR_WHITESPACE;
            else if (regex[i + 1] == '*')
                regChr = CHAR_STAR;
            else if (regex[i + 1] == '+')
                regChr = CHAR_PLUS;
            else
                printf("ERROR: regex '%s' not properly formatted at index %d.\n", regex, i);

            newItem = createRegexItem('\\', TYPE_SINGLE, regChr);
            if (result == NULL) {
                result = newItem;
                newest = newItem;
            }
            else {
                newest->next = newItem;
                newest = newItem;
            }
            i += 1; // Skip the character the \ escaped
        }
        else if (regex[i] == '*') {
            newest->regexType = TYPE_STAR;
            newest->satisfied = true;
        }
        else if (regex[i] == '+') {
            newest->regexType = TYPE_PLUS;
        }
        else { // Regular character
            newItem = createRegexItem(regex[i], TYPE_SINGLE, CHAR_NORMAL);
            if (result == NULL) {
                result = newItem;
                newest = newItem;
            }
            else {
                newest->next = newItem;
                newest = newItem;
            }
        }
    }

    return result;
}

RegexItem *createRegexItem(char chr, RegexType regexType, RegexChar regexChar) {
    RegexItem* result = malloc(sizeof(RegexItem));
    result->chr = chr;
    result->regexType = regexType;
    result->regexChar = regexChar;
    result->next = NULL;
    result->satisfied = false;
    return result;
}

// -------------------------------------------------------


int performParallelGrep(const char *filename, char *regex, const int numChildren);

void checkLine(int lineNum, char *line, char *regex, int childNum);

void consumerProcess(const int childNumber, const int numThreads, char *regex, int pipeHandles[2]);

void producerProcess(int numChildren, int pipeHandles[][2], FILE *file);

int performSingularGrep(const char *filename, char *regex) ;

int main(int argc, char** argv) {
    if (argc != 3 && argc != 5) {
        printf("Not enough arguments for program2.\nUsage: process_grep <filename> <regex search> (-N #processes, optional)\n");
        return 1;
    }
    int numThreads = 1;
    if (argc == 5) {
        numThreads = atoi(argv[4]);
    }
    char* filename = argv[1];
    char* regex = argv[2];

    if (numThreads > 1)
        return performParallelGrep(filename, regex, numThreads - 1);
    else
        return performSingularGrep(filename, regex);
}

int performSingularGrep(const char *filename, char *regex) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file '%s'.\nExiting program.", filename);
        exit(EXIT_FAILURE);
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNumber = 1;

    while ((read = getline(&line, &len, file)) != -1) {
        checkLine(lineNumber, line, regex, 0); // Check the string
        lineNumber++;
    }

    exit(EXIT_SUCCESS);
}


int performParallelGrep(const char *filename, char *regex, const int numChildren) {
    int pipeHandles[numChildren][2];
    pid_t cpid[numChildren];

    int numSuccessfulChildren = numChildren;
    for (int i = 0; i < numChildren; i++) {
        pipe((int *) pipeHandles[i]);
        cpid[i] = fork();

        if (cpid[i] == -1) {
            printf("Error forking child %d!\n", i);
        }
        else if (cpid[i] == 0) {
            // I am the child
            close(pipeHandles[i][1]); // close the write-end of the pipe, I'm not going to use it
            consumerProcess(i, numChildren, regex, pipeHandles[i]);
            close(pipeHandles[i][0]); // close the read-end of the pipe
            exit(EXIT_SUCCESS);
        }
        else {
            if (pipeHandles[i][0] < 3 || pipeHandles[i][1] < 3 || pipeHandles[i][0] > 255 || pipeHandles[i][1] > 255) {
                numSuccessfulChildren--;
            }
        }
    }

    // Now I know I'm the parent
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file '%s'.\nExiting program.", filename);
        exit(EXIT_FAILURE);
    }

    producerProcess(numSuccessfulChildren, pipeHandles, file);

    // Close it up boys
    for (int i = 0; i < numChildren; i++) {
        close(pipeHandles[i][0]);
        close(pipeHandles[i][1]);
    }

    wait(NULL); // wait for the child process to exit before I do the same
    exit(EXIT_SUCCESS);
}

void producerProcess(int numChildren, int pipeHandles[][2], FILE *file) {
    char * line = NULL;
    char* newlineFix = NULL;
    size_t len = 0;
    ssize_t read;

    int lineNum = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        if (line[strlen(line) - 1] != '\n') {
            newlineFix = malloc(strlen(line) + 2); // \n and \0
            strcpy(newlineFix, line);
            newlineFix[strlen(line)] = '\n';
            newlineFix[strlen(line) + 1] = '\0';
            write(pipeHandles[lineNum % numChildren][1], newlineFix, strlen(newlineFix)); // send the line to the reader
        }
        else {
            write(pipeHandles[lineNum % numChildren][1], line, strlen(line)); // send the line to the reader
        }
        lineNum++;
    }

    fclose(file);
    if (line != NULL) free(line);
    if (newlineFix != NULL) free(newlineFix);
}

void consumerProcess(const int childNumber, const int numThreads, char *regex, int pipeHandles[2]) {
    char buf;

    ArrayList* charBuffer = alCreateList(100);
    int lineNumber = childNumber + 1;
    while (read(pipeHandles[0], &buf, 1) > 0) {// read while EOF
        if (buf != '\n' && buf != '\r') { // If we're on the same line, keep adding it to our list
            alAdd(charBuffer, buf);
        }
        else {
            // We hit the end of the line
            alAdd(charBuffer, '\n');
            alAdd(charBuffer, '\0');

            // Combine all the characters into a string
            char* line = malloc(alSize(charBuffer) * sizeof(char));
            for (int i = 0; i < alSize(charBuffer); i++) {
                line[i] = (char) alGet(charBuffer, i);
            }

            checkLine(lineNumber, line, regex, childNumber); // Check the string
            lineNumber += numThreads; // Round robin process assignment, so the i'th process gets all lines %i == 0
            charBuffer->size = 0; // Reset the list, it will get overwritten
            free(line); // return the memory
        }
    }
    alDelete(charBuffer);
}

void checkLine(int lineNumber, char *line, char *regex, int childNum) {
    ArrayList* result = matchingPositions(line, regex);
    char buffer[100];
    for (int i = 0; i < result->size; i++) {
        int position = alGet(result, i);
        sprintf(buffer, "%d:%d\n", lineNumber, position);
        write(STDOUT_FILENO, buffer, strlen(buffer));
    }

    alDelete(result);
    usleep(25);
}