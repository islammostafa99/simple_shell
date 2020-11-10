#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#define ARG_MAX 256     // Maximum number of arguments in a line
#define MAX_LINE_LENGTH ARG_MAX * 256   // Maximum number of characters in a line
int is_background;      // Background processes flag
char *input();
char **parser(char *command_line);
void execute_command(char **arguments);
void interrupt_handler();
int main() {
    // Reset logger file by overwriting and emptying its contents
    FILE *f;
    f = fopen("logger.log", "w");
    fclose(f);
    // Shell keeps taking commands until user types exit command
    while (1) {
        is_background = 0;
        printf(">> ");
        char *command_line = input();
        char **arguments = parser(command_line);
        execute_command(arguments);
    }
}
// Read command line and add '\0' to convert it to C string
char *input() {
    // Allocating space to command line to be sent to parser
    char *command_line = malloc(sizeof(char) * MAX_LINE_LENGTH);
    int position = 0;
    // Taking each character and checks for newline or end of file
    while (1) {
        command_line[position] = getchar();
        if (command_line[position] == '\n' || command_line[position] == EOF) {
            command_line[position] = '\0';
            break;
        }
        position++;
    }
    return command_line;
}
// parse command line into arguments and check for background process flag
char **parser(char *command_line) {
    // Allocating space for arguments to be sent to execute_command
    char **arguments = malloc(sizeof(char *) * ARG_MAX);
    int position = 0;    // Position of first argument
    // Split line into tokens according to spaces
    char *split = strtok(command_line, " ");
    while (split != NULL) {     // Looping through tokens to put them in arguments
        arguments[position++] = split;
        split = strtok(NULL, " ");
    }
    // Check for background process flag
    if (strcmp(arguments[position - 1], "&") == 0) {
        is_background = 1;
        arguments[position - 1] = NULL;   // Remove that flag
    }
    return arguments;
}
// Executing command arguments by forking child process
void execute_command(char **arguments) {
    // Interrupting signal
    signal(SIGCHLD, interrupt_handler);
    if (arguments[0] == NULL) {
        return;
    }
    // Exit shell if user typed command "exit"
    if (strcmp(arguments[0], "exit") == 0) {
        free(arguments);     // Free allocated memory by arguments
        exit(0);
    }
    int status;     // status for checking the change in child process
    pid_t pid = fork();      // Forking current process to child process
    if (pid < 0) {  // Failed to fork
        perror("Cannot fork child process\n");
    } else if (pid == 0) {  // Child process
        if (execvp(arguments[0], arguments) < 0) {
            printf("Cannot execute command\n");
        }
    } else {    // Parent process
        if (!is_background) {   // Don't wait if it's a background process
            waitpid(pid, &status, 0);
        }
    }
}
// Logging child process ended into logger when receiving SIGCHLD
void interrupt_handler() {
    FILE *f;
    f = fopen("logger.log", "a");
    fprintf(f, "Child process was terminated\n");
}
