#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "list/arraylist.h"
#include "regex/regex.h"

int performSingularGrep(char *filename, char *regex);

void checkLine(int lineNum, char *line, char *regex, int i);

int performParallelGrep(char *filename, char *regex, int numThreads);

void* producerFunction(void* ptr) ;

void* consumerFunction(void* ptr) ;

int main(int argc, char** argv) {
    if (argc != 3 && argc != 5) {
        printf("Not enough arguments for program2.\nUsage: process_grep <filename> <regex search> (-N #threads, optional)\n");
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
        int lineNum = lineNumBuffer[bufferSize - 1];
        checkLine(lineNum, line, args->regex, args->childNum);
        bufferSize--;
        free(line);

        pthread_cond_signal(&bufferEmpty);

        pthread_mutex_unlock(&mutex);
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
        printf("%d:%d\t%s", lineNumber, position, line);
//        for (int k = 0; k < position; k++) printf(" ");
//        printf("^\n");
    }

    alDelete(result);
}