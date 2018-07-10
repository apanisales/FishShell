/**
 * history.c
 *
 * Contains functions needed to execute shell commands.
 *
 * Author: Anthony Panisales
 */

#include "history.h"

#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int i = -1;
int min = 0;
int max = 0;
int output_index = 0;
struct history_entry command_history[HIST_MAX];
struct passwd * user_data;

/**
 * retokenize
 *
 * Splits a line from the command line, that was previously executed, into tokens.
 *
 * @param line
 *           line from the command line
 * @param old_tokens
 *           a pointer to an array of previously executed string tokens
 */
void retokenize(char line[], char *** old_tokens) {
	char *token = strtok(line, " \t\n");
	char *tokens[PATH_MAX] = { 0 };
    	int index = 0;
	while (token != NULL) {
		if (strstr(token, "#") != NULL) {
		    break;
		}
		tokens[index++] = token;
		token = strtok(NULL, " \t\n");
	}
	tokens[index] = (char *) 0;
	*old_tokens = tokens;
}

/**
 * redirection_handler
 *
 * Checks if a line from the command line is calling
 * for output and pipe redirection, and executes commands
 * according to what the line is calling for.
 *
 * @param tokens
 *           an array of String tokens from the line from
 *           the command line
 * @param fd_in
 *           file descriptor to be used when executing commands 
 */
void redirection_handler(char * tokens[], int fd_in) {
	int index = 0;
	char ** rest_of_tokens;
	
	while (tokens[index] != (char *) 0) {
		if (strcmp(tokens[index], "|") == 0) {
			break;
		}
		if (strcmp(tokens[index], ">") == 0) {
			output_index = index+1;
		}
		index++;
	}

	if (tokens[index] == (char *) 0) {
		execute(tokens);
		fflush(stdout);
		return;
	} else {
		rest_of_tokens = &tokens[index+1];
		tokens[index] = (char *) 0;
	}
	
	int fd[2];
    pipe(fd);
    
    pid_t pid = fork();
    if (pid == -1) {
    	perror("fork");
    	exit(1);
    } else if (pid == 0) { 
    	/* Child */
		close(fd[0]);
		dup2(STDOUT_FILENO, fd_in);
		dup2(fd[1], STDOUT_FILENO);
		execute(tokens);
		fflush(stdout);
		close(fd[1]);
		exit(0);
    } else {
    	/* Parent */
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		redirection_handler(rest_of_tokens, fd[0]);
		close(fd[0]);
		exit(0);
    }
}

/**
 * execute
 *
 * Executes the command entered. Calls execvp if the command is not a
 * built-in one.
 *
 * @param tokens
 *           an array of String tokens from the line from
 *           the command line
 */
void execute(char * tokens[]) {
	if (output_index > 0) {
	    dup2(open(tokens[output_index], O_CREAT | O_WRONLY | O_TRUNC, 0666), STDOUT_FILENO);
	    tokens[output_index-1] = (char *) 0;
	    output_index = 0;
	}

    if (strcmp("cd", tokens[0]) == 0) {
        if (tokens[1] != NULL) {
            if (chdir(realpath(tokens[1], NULL)) < 0) {
                printf("-fish: cd: %s: No such file or directory\n", tokens[1]);
            }
        } else {
        	if ((user_data = getpwuid(getuid())) != NULL) {
            	chdir(user_data->pw_dir);
        	}
        }
    } else if (strcmp("history", tokens[0]) == 0) {
        print_history();
    } else if (tokens[0][0] == '!' && strlen(tokens[0]) != 1) {
        history_execution(tokens);
    } else if (execvp(tokens[0], tokens) < 0) {
	    if (strstr(tokens[0], "/") == NULL) {
	        printf("-fish: %s: command not found\n", tokens[0]);
	    } else {
	        printf("-fish: ");
	        fflush(stdout);
	        perror(tokens[0]);
	    }
	}
}

/**
 * add_to_history
 *
 * Adds a history_entry struct to command_history.
 *
 * @param entry
 *           a newly created struct made for the
 *           newest command executed
 */
void add_to_history(struct history_entry entry) {
	if (i == -1) {
		i = 0;
	}

	command_history[i++] = entry;

	if (i % HIST_MAX == 0) {
		i = 0;
	}

	max = entry.cmd_id;
	if (max - HIST_MAX >= 0) {
		min = max - HIST_MAX;
	}
}

/**
 * history_execution
 *
 * Executes a command that was previously entered
 *
 * @param tokens
 *           an array of String tokens from the line from
 *           the command line
 */
void history_execution(char * tokens[]) {
	char ** old_tokens;
	char * command = NULL;
	int index = 1;
	int hist_num = atoi(tokens[0]+1);

	if (strcmp("!!", tokens[0]) != 0) {
		for (index = 1; index < strlen(tokens[0]); index++) {
			if (!isdigit(tokens[0][index])) {
				command = tokens[0]+1;
				break;
			}
		}
		if (command == NULL && (hist_num > max || hist_num < min)) {
			printf("-fish: %s: event not found\n", tokens[0]);
			return;
		}
	}

	if ((i-1) % HIST_MAX >= 0) {
		index = (i-1) % HIST_MAX;
	} else {
		index = HIST_MAX - 1;
	}

	while (index != i) {
		retokenize(command_history[index].cmd_line, &old_tokens);
		if (strcmp(tokens[0], "!!") == 0) {
			if (strcmp(old_tokens[0], "!!") != 0) {
				redirection_handler(old_tokens, STDOUT_FILENO);
				return;
			}
		} else if (command != NULL) {
			if (strcmp(old_tokens[0], command) == 0) {
				redirection_handler(old_tokens, STDOUT_FILENO);
				return;
			}
		} else if (command_history[index].cmd_id == hist_num) {
			redirection_handler(old_tokens, STDOUT_FILENO);
			return;
		}
		--index;
		if (index == -1) {
			index = HIST_MAX-1;
		}
	}

	if (strcmp(tokens[0], "!!") == 0) {
		retokenize(command_history[index].cmd_line, &old_tokens);
		if (strcmp(old_tokens[0], "!!") != 0) {
			redirection_handler(old_tokens, STDOUT_FILENO);
			return;
		}
	} else if (command != NULL)  {
		if (strcmp(old_tokens[0], command) == 0) {
			redirection_handler(old_tokens, STDOUT_FILENO);
			return;
		}
	} else if (command_history[i].cmd_id == hist_num) {
		redirection_handler(old_tokens, STDOUT_FILENO);
		return;
	}
}

/**
 * print_history
 *
 * Prints the last 100 commands entered with their command numbers.
 */
void print_history() {
    if (i > -1) {
	    int start;
	    int end;

	    if (command_history[i+1].cmd_id == 0 && i+1 != 0) {
	    	start = 0;
	    	end = i;
	    } else {
	    	start = (i+1) % HIST_MAX;
	    	end = i;
	    }

	    if (start > end) {
			printf("[%lu|%.2f] %s", command_history[end].cmd_id, command_history[end].run_time, command_history[end].cmd_line);
	    }

		while (start != end) {
			printf("[%lu|%.2f] %s", command_history[start].cmd_id, command_history[start].run_time, command_history[start].cmd_line);
			fflush(stdout);
			start++;
			if (start % HIST_MAX == 0) {
				start = 0;
			}
		}
	}
}
