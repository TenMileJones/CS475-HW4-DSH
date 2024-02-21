/*
 * dsh.c
 *
 *  Version: Feb 20 2024
 *      Author: Branson
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>






/**
 * Trims white space from an input string in place.
 * Credit: https://www.delftstack.com/howto/c/trim-string-in-c/
 * @param str pointer to string to be trimmed
*/
void trimwhitespace(char *str) {
    int start = 0, end = strlen(str) - 1;

    // Remove leading whitespace
    while (isspace(str[start])) {
        start++;
    }

    // Remove trailing whitespace
    while (end > start && isspace(str[end])) {
        end--;
    }
    // If the string was trimmed, adjust the null terminator
    if (start > 0 || end < (strlen(str) - 1)) {
        memmove(str, str + start, end - start + 1);
        str[end - start + 1] = '\0';
    }
}

/**
 * Splits string into array of strings split by a delimiter.
 * Delimiter must be string of length 1 or behavior is undefined.
 * 
 * @param str string to be split. This string will be destroyed.
 * @param delim string of length 1: delimiter to split str by
 * @return char** array of string tokens, with last element 'NULL'
*/
char** split(char *str, char *delim){
    // First, determine number of tokens
    // Credit: https://stackoverflow.com/questions/9052490/find-the-count-of-substring-in-string
    int numTokens = 1;
    const char *tmp = str;
    while((tmp = strstr(tmp, delim))){
        numTokens++;
        tmp++;
    }

    // Create 2D array of string tokens
    char **array = (char**) malloc((numTokens+1) * sizeof(char*));

    int capacity = strlen(str);
    for (int i = 0; i < numTokens; i++) {
        array[i] = (char*) malloc(capacity * sizeof(char));
        array[i][0] = '\0';
    }

    // assign token strings to individual array elements
    strcpy(array[0], strtok(str, delim));
    for (int i = 1; i < numTokens; i++) {
        strcpy(array[i], strtok(NULL, delim));
    }
    array[numTokens] = NULL; // lets us find end of array later

    return array;
}

/**
 * Executes program at args[0] with arguments **args.
 * @param args array of arguments
 * @param numArgs number of arguments in args
*/
void execute(char **args, int numArgs){
    int runInBackground = FALSE;
    if(args[numArgs-1][0] == '&'){
        runInBackground = TRUE;
        args[numArgs-1] = NULL;
    }
    
    int isParent = fork();
    if(!isParent){
        execv(args[0], args);
    }
    
    if(!runInBackground){
        wait(NULL);
    }
}

/**
 * Frees 2D array and all contents inside of it
 * @param freeMe 2D array
*/
void free2DArray(char **freeMe){
    int i = 0;
    while (freeMe[i] != NULL) {
		free(freeMe[i]);
        i++;
	}
    free(freeMe);
}
