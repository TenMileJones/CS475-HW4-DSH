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
			free(cmdline);
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
		while (terms[numTerms] != NULL) {
			numTerms++;
		}

		// exit
		if (!strcmp(terms[0], "exit")){
			free(cmdline);
			free2DArray(terms);
			exit(0);
		}
		
		// pwd
		if (!strcmp(terms[0], "pwd")){
			char *pwd = getcwd(NULL, MAXBUF);
			printf("%s\n", pwd);
			free(pwd);
			free2DArray(terms);
			continue;
		}

		// cd
		if (!strcmp(terms[0], "cd")){
			if(terms[1] == NULL){
				chdir("/home/bhjones");
				printf("/home/bhjones\n");
				free2DArray(terms);
				continue;
			}
			if(chdir(terms[1])){
				// If path is invalid
				printf("dsh: cd: %s: No such file or directory\n", terms[1]);
				free2DArray(terms);
				continue;
			}
			printf("%s\n", terms[1]);
			free2DArray(terms);
			continue;
		}

		// command execution
		if (terms[0][0] == '/'){
			// full path
			if(access(terms[0], F_OK | X_OK)){
				// File doesn't exist or is not executable
				printf("Command '%s' not found.\n", terms[0]);
				free2DArray(terms);
				continue;
			} else {
				// File exists and is executable
				execute(terms, numTerms);
				free2DArray(terms);
			}
		} else {
			// path construction necessary
			// is it in current directory?
			char *executable = getcwd(NULL, (MAXBUF-2-strlen(terms[0])));
			if (executable == NULL){
				printf("Error: input too long. Shorten input.\n");
				free2DArray(terms);
				continue;
			}
			strcat(executable, "/");
			strcat(executable, terms[0]);
			// try it
			if(access(executable, F_OK | X_OK) == 0){
				// File is in CWD. Execute
				terms[0] = executable;
				execute(terms, numTerms);
				free2DArray(terms);
			} else {
				// path construction NECESSARY
				executable = (char*) realloc(executable, MAXBUF);
				executable[0] = '\0';
				strcpy(executable, getenv("PATH"));

				// tokenize paths
				char **paths = split(executable, ":");

				// attempt to execute on each path
				int i = 0;
				int foundpath = FALSE;
				while (paths[i] != NULL) {
					executable = (char*) realloc(executable, (MAXBUF-2-strlen(terms[0])));
					if (executable == NULL){
						printf("Error: input too long. Shorten input.\n");
						break;
					}
					executable[0] = '\0';
					strcpy(executable, paths[numTerms]);
					strcat(executable, "/");
					strcat(executable, terms[0]);
					i++;

					if(access(executable, F_OK | X_OK) == 0){
						// File is in CWD. Execute and end search
						terms[0] = executable;
						execute(terms, numTerms);
						foundpath = TRUE;
						break;
					}
				}
				if(!foundpath){
					// File doesn't exist or is not executable
					printf("Command '%s' not found.\n", terms[0]);
				}
				free2DArray(terms);
				free2DArray(paths);
			}
			free(executable);
		}
	}
	free(cmdline);
}
