#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 256
#define DELIMITERS " \t\r\n\a"

int main() {
    char *line;
    char **args;
    bool status = true;

    do {
        printf("$mysh> ");
        line = read_command();
        args = parse_command(line);
        if (args[0] == NULL) {
            free(line);
            free(args);
            continue;
        }
        if (is_builtin_command(args[0])) {
            execute_builtin_command(args[0], args);
        } else {
            execute_command(args);
        }
        free(line);
        free(args);
    } while (status);
    
    return 0;
}

char *read_command(void) {
    char *line = NULL;
    ssize_t bufsize = 0;
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS);
        } else {
            perror("Failed to read command");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char **parse_command(char *line) {
    int bufsize = MAX_COMMAND_LENGTH, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, *saveptr;

    if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok_r(line, DELIMITERS, &saveptr);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MAX_COMMAND_LENGTH;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok_r(NULL, DELIMITERS, &saveptr);
    }
    tokens[position] = NULL;
    return tokens;
}

bool is_builtin_command(char *command) {
    return (strcmp(command, "cd") == 0 || strcmp(command, "exit") == 0 || strcmp(command, "pwd") == 0);
}

void execute_builtin_command(char *command, char *args[]) {
    if (strcmp(command, "cd") == 0) {
        change_directory(args[1]);
    } else if (strcmp(command, "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(command, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd() error");
        }
    }
}

void change_directory(char *path) {
    if (chdir(path) != 0) {
        if (errno == ENOENT) {
            fprintf(stderr, "No such directory\n");
        } else {
            fprintf(stderr, "Change directory failed\n");
        }
    }
}

void execute_command(char *args[]) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        
        if (execvp(args[0], args) == -1) {
            perror("Failed to execute command");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        
        perror("Failed to create process");
    } else {
        
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}
