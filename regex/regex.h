//
// Created by Test on 9/25/15.
//

#ifndef TESTPROJ_REGEX_H
#define TESTPROJ_REGEX_H

#include "../list/arraylist.h"

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

#endif //TESTPROJ_REGEX_H
