#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define ARG_MAX 256     // Maximum number of arguments in a line
#define MAX_LINE_LENGTH ARG_MAX * 256   // Maximum number of characters in a line

int is_background;      // Background processes flag

void interrupt_handler();

void execute_cmd(char **args);

char **split_line(char *cmd_line);

char *read_line();


int main() {
    // Reset logger file by overwriting and emptying its contents
    FILE *f;
    f = fopen("logger.log", "w");
    fclose(f);

    // Shell keeps taking commands until user types exit command
    while (1) {
        is_background = 0;  // Reset background process flag
        printf(">> ");
        char *cmd_line = read_line();           // Reading command line
        char **args = split_line(cmd_line);     // Splitting command line into arguments
        execute_cmd(args);                      // Executing the arguments
    }
}

// Logging child process ended into logger when receiving SIGCHLD
void interrupt_handler() {
    FILE *f;
    f = fopen("logger.log", "a");
    fprintf(f, "Child process was terminated\n");
}

// Executing command arguments by forking child process
void execute_cmd(char **args) {
    // Interrupting signal
    signal(SIGCHLD, interrupt_handler);

    // Return to take more commands if first argument is NULL
    if (args[0] == NULL) {
        return;
    }

    // Exit shell if user typed command "exit"
    if (strcmp(args[0], "exit") == 0) {
        free(args);     // Free allocated memory by args
        exit(0);
    }

    int status;     // status for checking the change in child process
    pid_t pid = fork();      // Forking current process to child process

    if (pid < 0) {  // Failed to fork
        perror("Cannot fork child process\n");
    } else if (pid == 0) {  // Child process
        if (execvp(args[0], args) < 0) {
            printf("Cannot execute command\n");
        }
    } else {    // Parent process
        if (!is_background) {   // Don't wait if it's a background process
            waitpid(pid, &status, 0);
        }
    }
}

// Split command line into arguments and check for background process flag
char **split_line(char *cmd_line) {
    // Allocating space for arguments to be sent to execute_cmd
    char **args = malloc(sizeof(char *) * ARG_MAX);
    int pos = 0;    // Position of first argument

    // Split line into tokens according to spaces
    char *split = strtok(cmd_line, " ");
    while (split != NULL) {     // Looping through tokens to put them in args
        args[pos++] = split;
        split = strtok(NULL, " ");
    }

    // Check for background process flag
    if (strcmp(args[pos - 1], "&") == 0) {
        is_background = 1;
        args[pos - 1] = NULL;   // Remove that flag
    }

    return args;
}

// Read command line and add '\0' to convert it to C string
char *read_line() {
    // Allocating space to command line to be sent to split_line
    char *cmd_line = malloc(sizeof(char) * MAX_LINE_LENGTH);
    int pos = 0;

    // Taking each character and checks for newline or end of file
    while (1) {
        cmd_line[pos] = getchar();
        if (cmd_line[pos] == '\n' || cmd_line[pos] == EOF) {
            cmd_line[pos] = '\0';
            break;
        }
        pos++;
    }

    return cmd_line;
}
