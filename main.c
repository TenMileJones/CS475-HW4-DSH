/*
 * main.c
 *
 *  Created on: Mar 17 2017
 *      Author: david
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include "dsh.h"

#define MAX_PROC 250

int main(int argc, char *argv[]) {

	// DO NOT REMOVE THE BLOCK BELOW (FORK BOMB PREVENTION) //
	struct rlimit limit;
	limit.rlim_cur = MAX_PROC;
	limit.rlim_max = MAX_PROC;
	setrlimit(RLIMIT_NPROC, &limit);
	// DO NOT REMOVE THE BLOCK ABOVE THIS LINE //


	char *cmdline = (char*) malloc(MAXBUF); // stores user input from commmand line

	while(TRUE){
		printf("dsh> ");
		// reads up to 256 characters into the buffer
		if (fgets(cmdline, MAXBUF, stdin) == NULL) {
			exit(0);  // exit the program if EOF is input
		}
		
		if (cmdline[strlen(cmdline)-1] != '\n'){
			// reprompt if last char isn't '\n'; user input was too long 
			printf("Error: input > 256 characters: Shorten input.\n");
			continue;
		} 
		// remove '\n' from end of input and trim white space
		cmdline[strlen(cmdline)-1] = '\0';
		trimwhitespace(cmdline);

		if (!strlen(cmdline)){
			continue; // reprompt if entry is empty or just spaces
		}

		// split command into terms
		char **terms = split(cmdline, " ");
		// get number of terms
		int numTerms = 0;
		printf("Print out all tokens:\n");
		while (terms[numTerms] != NULL) {
			printf("%s\n", terms[numTerms]);
			numTerms++;
		}

		// exit
		if (!strcmp(terms[0], "exit")){
			exit(0);
		}
		
		// pwd
		if (!strcmp(terms[0], "pwd")){
			char *pwd = getcwd(NULL, MAXBUF);
			printf("%s\n", pwd);
			free(pwd);
			continue;
		}

		// cd
		if (!strcmp(terms[0], "cd")){
			if(terms[1] == NULL){
				chdir("/home/bhjones");
				printf("/home/bhjones\n");
				continue;
			}
			if(chdir(terms[1])){
				// If path is invalid
				printf("dsh: cd: %s: No such file or directory\n", terms[1]);
				continue;
			}
			printf("%s\n", terms[1]);
			continue;
		}

		// command execution
		if (terms[0][0] == '/'){
			// full path
			if(access(terms[0], F_OK | X_OK)){
				// File doesn't exist or is not executable
				printf("Command '%s' not found.\n", terms[0]);
				continue;
			} else {
				// File exists and is executable
				int runInBackground = FALSE;
				if(terms[numTerms-1][0] == '&'){
					runInBackground = TRUE;
					terms[numTerms-1] = NULL;
				}
				
				// Determine if multiple args exist
				if(terms[1] == NULL){
					// No args, just run
					execute(terms[0], NULL, runInBackground);
				} else {
					execute(terms[0], terms, runInBackground);
				}

			}
		} else {
			// path construction necessary
			
		}

		free(terms);


	}
	
	free(cmdline);
}
