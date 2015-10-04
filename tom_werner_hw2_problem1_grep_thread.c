#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

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
    RegexItem* pattern = preproccessRegex(regex);
    ArrayList* result = alCreateList(16);

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

int performSingularGrep(char *filename, char *regex);

void checkLine(int lineNum, char *line, char *regex, int i);

int performParallelGrep(char *filename, char *regex, int numThreads);

void* producerFunction(void* ptr) ;

void* consumerFunction(void* ptr) ;

int main(int argc, char** argv) {
    if (argc != 3 && argc != 5) {
        printf("Not enough arguments for program2.\nUsage: thread_grep <filename> <regex search> (-N #threads, optional)\n");
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

#define MAX_BUFFER_SIZE 1000
pthread_mutex_t mutex;
pthread_cond_t bufferFull, bufferEmpty;

char* linesBuffer[MAX_BUFFER_SIZE];
int lineNumBuffer[MAX_BUFFER_SIZE];
int bufferSize;
bool finishedReadingFile = false;

typedef struct producerArgs {
    char* filename;
} ProducerArgs;
typedef struct consumerArgs {
    char* regex;
    int childNum;
} ConsumerArgs;


int performParallelGrep(char *filename, char *regex, int numThreads) {
    pthread_t producer;
    pthread_t consumers[numThreads];

    ProducerArgs producerArgs;
    ConsumerArgs consumerArgs[numThreads];
    producerArgs.filename = filename;

    // set up our mutex
    pthread_mutex_init(&mutex, NULL); // NULL -> normal mutex

    // set up our conditionals
    pthread_cond_init(&bufferFull, NULL);
    pthread_cond_init(&bufferEmpty, NULL);

    // set up our threads
    pthread_create(&producer, NULL, producerFunction, &producerArgs);

    for (int i = 0; i < numThreads; i++) {
        consumerArgs[i].regex = regex;
        consumerArgs[i].childNum = i;

        pthread_create(&consumers[i], NULL, consumerFunction, &consumerArgs[i]);
    }

    // Now we need to join our threads back together. This waits for children to finish
    pthread_join(producer, NULL);
    for (int i = 0; i < numThreads; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Destroy the mutex and conditions
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&bufferFull);
    pthread_cond_destroy(&bufferEmpty);

    exit(EXIT_SUCCESS);
}

void* consumerFunction(void* ptr) {
    ConsumerArgs* args = ptr;
    while (true) {
        pthread_mutex_lock(&mutex);

        while (bufferSize == 0 && !finishedReadingFile) {
            pthread_cond_wait(&bufferFull, &mutex); // Wait until the buffer has stuff to process
        }

        if (finishedReadingFile && bufferSize <= 0) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
        }

        char* line = linesBuffer[bufferSize - 1];


        char* local = malloc((strlen(line) + 1) * sizeof(char));
        if (strlen(line) > 0) {
            strcpy(local, line);
            free(line);
            int lineNum = lineNumBuffer[bufferSize - 1];
            bufferSize--;

            pthread_cond_signal(&bufferEmpty);
            pthread_mutex_unlock(&mutex);

            checkLine(lineNum, local, args->regex, args->childNum);
            free(local);
        }
        else {
            free(line);
            pthread_cond_signal(&bufferEmpty);
            pthread_mutex_unlock(&mutex);
        }

    }
}

void* producerFunction(void* ptr) {
    ProducerArgs* args = ptr;
    FILE* file = fopen(args->filename, "r");
    if (file == NULL) {
        printf("Could not open file '%s'.\nExiting program.", args->filename);
        pthread_exit((void *) 1);
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int lineNum = 1;
    while ((read = getline(&line, &len, file)) != -1) {
        pthread_mutex_lock(&mutex); // protect the buffer and bufferSize
        while (bufferSize == MAX_BUFFER_SIZE) // While the buffer is full wait
            pthread_cond_wait(&bufferEmpty, &mutex);

        lineNumBuffer[bufferSize] = lineNum;

        linesBuffer[bufferSize] = malloc(strlen(line) * sizeof(char) + 1);
        strcpy(linesBuffer[bufferSize], line);
        linesBuffer[bufferSize][strlen(line)] = '\0';
        bufferSize++; // Increase the buffer size

        pthread_cond_signal(&bufferFull); // let the consumers know its no longer empty
        pthread_mutex_unlock(&mutex); // Unprotect the variables

        lineNum++;
    }

    pthread_mutex_lock(&mutex);
    finishedReadingFile = true;
    pthread_cond_broadcast(&bufferFull);
    pthread_mutex_unlock(&mutex);

    fclose(file);
    free(line);


    pthread_exit(0);
}

int performSingularGrep(char *filename, char *regex) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file '%s'.\nExiting program.", filename);
        return 1;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int lineNum = 1;
    while ((read = getline(&line, &len, file)) != -1) {
        checkLine(lineNum, line, regex, 0);
        lineNum++;
    }

    fclose(file);
    free(line);

    return 0;
}

void checkLine(int lineNumber, char *line, char *regex, int childNum) {
    ArrayList* result = matchingPositions(line, regex);
    for (int i = 0; i < result->size; i++) {
        int position = alGet(result, i);
        printf("%d:%d\n", lineNumber, position);
//        for (int k = 0; k < position; k++) printf(" ");
//        printf("^\n");
    }

    alDelete(result);
    usleep(100);
}