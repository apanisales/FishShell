/**
 * shell.c
 *
 * A command line shell that can support built-in commands,
 * signal handling, timing, redirection, and scripting.
 *
 * Author: Anthony Panisales
 */

#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "history.h"
#include "timer.h"

char cwd[PATH_MAX];
char hostname[HOST_NAME_MAX + 1];
int executing_flag = 0;
int num_of_commands = 0;

void print_prompt(int num_of_commands, char * hostname, char * cwd);

/**
 * sigint_handler
 *
 * Handles the ctrl-C signal.
 *
 * @param signo
 *           the signal number
 */
void sigint_handler(int signo) {
    printf("\n");

    if (executing_flag == 0) {
        print_prompt(num_of_commands, hostname, cwd);
    }

    fflush(stdout);
}

/**
 * print_prompt
 *
 * Prints the shell prompt.
 *
 * @param num_of commands
 *          the command number (starting from 0) 
 * @param hostname
 *          the hostname of a user
 * @param cwd
 *          the current working directory
 */
void print_prompt(int num_of_commands, char * hostname, char * cwd) {
    printf("[%d|%s@%s:%s]$ ", num_of_commands, getlogin(), hostname, cwd);
}

int main(void) {
    signal(SIGINT, sigint_handler);

    getcwd(cwd, sizeof(cwd));

    chdir(cwd);

    char line[sysconf(_SC_ARG_MAX)];

    // Change to /home4/ for the lab computers
    char prefix[PATH_MAX] = "/home/";
    // char prefix[PATH_MAX] = "/home4/";

    if (isatty(STDIN_FILENO)) {
        char login_name[PATH_MAX];

        strcpy(login_name, getlogin());
        strcat(prefix, login_name);
    }

    hostname[HOST_NAME_MAX] = 0;

    while (true) {
        executing_flag = 0;

        if (isatty(STDIN_FILENO)) {
            gethostname(hostname, HOST_NAME_MAX);
            getcwd(cwd, sizeof(cwd));

            if (strncmp(prefix, cwd, strlen(prefix)) == 0) {
                strcpy(cwd, "~");
                strcat(cwd, cwd + strlen(prefix));
            }

            print_prompt(num_of_commands, hostname, cwd);
        }

        // fgets gets input from the user
        if (fgets(line, sysconf(_SC_ARG_MAX), stdin) == NULL) {
            exit(0);
        }

        char full_line[sysconf(_SC_ARG_MAX)];

        strcpy(full_line, line);

    	char *token = strtok(line, " \t\n");
        
        if (token == NULL) {
            continue;
        }

        if (strcmp("exit", token) == 0) {
            exit(0);
        }

    	char *tokens[PATH_MAX];
    	int i = 0;
    	while (token != NULL) {
            if (strstr(token, "#") != NULL)
                break;
            tokens[i++] = token;
    		token = strtok(NULL, " \t\n");
    	}
    	tokens[i] = (char *) 0;

        if (tokens[0] == (char *) 0) {
            continue;
        }

        executing_flag = 1;

        double start = get_time();

        if (strcmp("cd", tokens[0]) == 0 || strcmp("!cd", tokens[0]) == 0) {
            redirection_handler(tokens, STDOUT_FILENO);
        } else {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(1);
            } else if (pid == 0) {
                /* Child */
                redirection_handler(tokens, STDOUT_FILENO);
                exit(0);
            } else {
                /* Parent */
                int status;
                wait(&status);
            }
        }

        double end = get_time();
        double time_elapsed = end - start;

        struct history_entry current_command;
        current_command.cmd_id = num_of_commands++;
        current_command.run_time = time_elapsed;
        strcpy(current_command.cmd_line, full_line);

        add_to_history(current_command);

        start = 0.0;
        end = 0.0;
    }

    return 0;
}
