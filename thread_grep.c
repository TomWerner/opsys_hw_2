#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int performSingularGrep(char *filename, char *regex);

void checkLine(int lineNum, char *line, char *regex);

bool matchString(char *line, int offset, char *regex);

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
//sem_t bufferFull;
//sem_t bufferEmpty;
//sem_open()
//sem_init(&cFull, 0, 0); /* Initialize full semaphore */
//sem_init(&cEmpty, 0, BUFFER_SIZE); /* Initialize empty semaphore */

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
    printf("Starting child %d with regex %s\n", args->childNum, args->regex);
    while (true) {
        pthread_mutex_lock(&mutex);

        while (bufferSize == 0 && !finishedReadingFile) {
            pthread_cond_wait(&bufferFull, &mutex); // Wait until the buffer has stuff to process
        }

        if (finishedReadingFile && bufferSize <= 0) {
            printf("Exiting child %d\n", args->childNum);
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
        }

        char* line = linesBuffer[bufferSize - 1];
        int lineNum = lineNumBuffer[bufferSize - 1];
        printf("Child %d: line %d - '%s'\n", args->childNum, lineNum, line);
        bufferSize--;

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

    int lineNum = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        pthread_mutex_lock(&mutex); // protect the buffer and bufferSize
        while (bufferSize == MAX_BUFFER_SIZE) // While the buffer is full wait
            pthread_cond_wait(&bufferEmpty, &mutex);

        lineNumBuffer[bufferSize] = lineNum;

        linesBuffer[bufferSize] = malloc(strlen(line) * sizeof(char));
        strcpy(linesBuffer[bufferSize], line);
        bufferSize++; // Increase the buffer size

        pthread_cond_signal(&bufferFull); // let the consumers know its no longer empty
        pthread_mutex_unlock(&mutex); // Unprotect the variables

        lineNum++;
    }

    pthread_mutex_lock(&mutex);
    printf("Finished reading file\n");
    finishedReadingFile = true;
    pthread_cond_broadcast(&bufferFull);
    pthread_mutex_unlock(&mutex);

    fclose(file);
    free(line);

    printf("Exiting producer.\n");

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

    int lineNum = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        checkLine(lineNum, line, regex);
        lineNum++;
    }

    fclose(file);
    free(line);

    return 0;
}

void checkLine(int lineNumber, char *line, char *regex) {
    int offset = 0;
    size_t lineLength = strlen(line);
    do {
        if (matchString(line, offset, regex)) {
            printf("%d:%d - %s\n", lineNumber, offset, line);
        }
        offset++;
    } while (offset < lineLength);
}

bool matchString(char *line, int offset, char *regex) {
    usleep(100);
    if (offset % 8 == 0) return true;
    return false;
} 

// 10 process: real	0m12.020s
// 1 process: real 1m26.915s

// 1 thread: real	1m31.257s
// 10 threads: real	0m10.773s