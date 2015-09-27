#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "list/arraylist.h"
#include "regex/regex.h"

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

    int lineNum = 1;
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
    int lineNumber = childNumber;
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
    free(charBuffer->data);
    free(charBuffer);
}

void checkLine(int lineNumber, char *line, char *regex, int childNum) {
    ArrayList* result = matchingPositions(line, regex);
    for (int i = 0; i < result->size; i++) {
        int position = alGet(result, i);
        printf("%d:%d\t%s", lineNumber, position, line);
    }

    alDelete(result);
}