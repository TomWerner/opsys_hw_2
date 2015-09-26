//
// Created by Test on 9/25/15.
//

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "regex.h"

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
