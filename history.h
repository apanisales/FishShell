#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id;
    double run_time;
    char cmd_line[4096];
};

void add_to_history(struct history_entry entry);
void execute(char * tokens[]); 
void history_execution(char * tokens[]);
void print_history();
void redirection_handler(char * tokens[], int fd);
void retokenize(char line[], char *** old_tokens);

#endif
