#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
// #include <sys/syslimits.h> //for me to check, works in unix based systems
#include <linux/limits.h> //for testing in codecrafters, works in linux based systems

#define validCommands 5
char* commands[validCommands] = {"echo", "type", "exit", "pwd", "cd"};

void checkUserInput(char* input);
void echo(char* input);
void type(char* input);
int isValidCommand(char* command);
char* findCommandInPath(char* input, char* path);
void executablesInPath(char* input);
void pwd();
void cd(char* input);
char** parse_arguments(const char* input, int* argc_out);

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  while(1) {
      // Uncomment this block to pass the first stage
      printf("$ ");

      // Wait for user input
      char input[100];
      fgets(input, 100, stdin);
      checkUserInput(input);
  }

  return 0;
}

void checkUserInput(char* input) {
    if(input == NULL || input[0] == '\n') {
        return;
    }

    size_t inputSize = strlen(input);
    if (inputSize > 0 && input[inputSize - 1] == '\n') {
        input[inputSize - 1] = '\0';
    }

    if(strcmp(input, "exit 0") == 0) {
        exit(0);
    }

    char *duplicateInput = strdup(input);
    if(!duplicateInput) {
        perror("strdup in checkUserInput");
        return;
    }
    char* command = strtok(duplicateInput, " ");
    if(strcmp(command, "echo") == 0) {
        echo(input);
        free(duplicateInput);
        return;
    } else if(strcmp(command, "type") == 0) {
        type(input);
        free(duplicateInput);
        return;
    } else if(strcmp(command, "pwd") == 0){
        pwd();
        return;
    } else if(strcmp(command, "cd") == 0){
        cd(input);
    }else {
        executablesInPath(input);
        return;
    }
    free(duplicateInput);
}

void echo(char* input) {
    int argc;
    char** argv = parse_arguments(input, &argc);

    for (int i = 1; i < argc; i++) { // skip "echo"
        printf("%s", argv[i]);
        if (i < argc - 1) printf(" ");
    }
    printf("\n");

    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
}

int isValidCommand(char* input) {
    for(int i = 0; i < validCommands; i++) {
        if(strcmp(commands[i], input) == 0) {
            return 1;
        }
    }
    return 0;
}

void type(char* input) {
    char *duplicateInput = strdup(input);
    if(!duplicateInput) {
        perror("strdup in type");
        return;
    }
    char* command = strtok(duplicateInput, " ");
    int argumentCount = 0;
    char* argument = strtok(NULL, " ");
    char* argumentArray[100];
    while(argument != NULL) {
        argumentArray[argumentCount] = argument;
        argumentCount++;
        argument = strtok(NULL, " ");
    }
    char *path = getenv("PATH");
    for(int iteration = 0; iteration < argumentCount; iteration++) {
        if(isValidCommand(argumentArray[iteration])) {
            printf("%s is a shell builtin\n", argumentArray[iteration]);
        } else {
            char* commandPath = findCommandInPath(argumentArray[iteration], path);
            if(commandPath != NULL) {
                printf("%s is %s\n", argumentArray[iteration], commandPath);
            } else {
                printf("%s: not found\n", argumentArray[iteration]);
            }
        }
    }
    free(duplicateInput);
    return;
}

char* findCommandInPath(char* input, char* path) {
    if(input == NULL || path == NULL) {
        return NULL;
    }
    char* pathCopy = strdup(path);
    if(!pathCopy) {
        perror("strdup in findCommandInPath");
        return NULL;
    }
    char* pathToken = strtok(pathCopy, ":");
    while(pathToken != NULL) {
        char* commandPath = malloc(strlen(pathToken) + strlen(input) + 2);
        if(!commandPath) {
            perror("malloc in findCommandInPath");
            free(pathCopy);
            return NULL;
        }
        sprintf(commandPath, "%s/%s", pathToken, input);
        if(access(commandPath, X_OK) == 0 || access(commandPath, F_OK) == 0) {
            // printf("%s is %s\n", input, commandPath);
            free(pathCopy);
            return commandPath;
        }
        free(commandPath);
        pathToken = strtok(NULL, ":");
    }
    free(pathCopy);
    // printf("%s: not found\n", input);
    return NULL;
}

void executablesInPath(char* input) {
    char* duplicateInput = strdup(input);
    char* path = getenv("PATH");
    if(!duplicateInput) {
        perror("strdup in executablesInPath");
        return;
    }
    char* command = strtok(duplicateInput, " ");
    char* commandPath = findCommandInPath(command, path);
    if(commandPath != NULL) {
        if(!duplicateInput) {
            perror("strdup in executablesInPath");
            return;
        }

        int argc;
        char** argv = parse_arguments(input, &argc);
        argv[argc] = NULL;

        pid_t pid = fork();
        if (pid == 0) {
            execv(commandPath, argv);
            perror("execv in executablesInPath");
            exit(1);
        } else if (pid < 0) {
            perror("fork in executablesInPath");
        } else {
            wait(NULL);
        }

        for (int i = 0; i < argc; i++) free(argv[i]);
        free(argv);

    } else {
        printf("%s: command not found\n", command);
    }
    return;
}

void pwd() {
    char currentWorkingDirectory[PATH_MAX];
    char* result = getcwd(currentWorkingDirectory, PATH_MAX);
    if(result != NULL) {
        printf("%s\n", result);
    }
    return;
}

void cd(char* input) {
    char* duplicateInput = strdup(input);
    if(!duplicateInput) {
        perror("strdup in cd");
        return;
    }
    char* command = strtok(duplicateInput, " ");
    char* argument = strtok(NULL, " ");

    if(argument[0] == '~') {
        char* home = getenv("HOME");
        if(home == NULL) {
            perror("HOME not set");
            free(duplicateInput);
            return;
        }
        size_t newSize = strlen(home) + strlen(argument);
        char* newPathWithHome = malloc(newSize);
        if(!newPathWithHome) {
            perror("Malloc in cd");
        }
        strcpy(newPathWithHome, home);
        strcat(newPathWithHome, argument + 1);

        int successful = chdir(newPathWithHome);
        if(successful != 0) {
            fprintf(stderr, "cd: %s: No such file or directory\n", argument);
        }
    } else {
        int successful = chdir(argument);
        if(successful != 0) {
            fprintf(stderr, "cd: %s: No such file or directory\n", argument);
        }
    }

    free(duplicateInput);
    return;
}


char** parse_arguments(const char* input, int* argc_out) {
    char** argv = malloc(sizeof(char*) * 100);  // Max 100 args
    int argc = 0;

    int i = 0;
    while (input[i] != '\0') {
        while (isspace(input[i])) i++; // skip leading spaces
        if (input[i] == '\0') break;

        char buffer[1024] = {0};
        int j = 0;
        int in_quote = 0;
        char quote_char = '\0';

        while (input[i] != '\0' && (in_quote || !isspace(input[i]))) {
            if ((input[i] == '\'' || input[i] == '"')) {
                if (!in_quote) {
                    in_quote = 1;
                    quote_char = input[i++];
                } else if (input[i] == quote_char) {
                    in_quote = 0;
                    i++;
                } else {
                    buffer[j++] = input[i++];
                }
            } else if (input[i] == '\\') {
                if (in_quote && quote_char == '\'') {
                    // In single quotes, backslash is literal
                    buffer[j++] = input[i++];
                }else if (in_quote && quote_char == '"') {
                    i++; // skip the backslash
                    if (input[i] == '"' || input[i] == '\\' || input[i] == '$' || input[i] == '`') {
                        buffer[j++] = input[i++];
                    } else if (input[i] != '\0') {
                        buffer[j++] = '\\';
                        buffer[j++] = input[i++];
                    }
                } else {
                    i++; // skip the backslash
                    if (input[i] != '\0') {
                        buffer[j++] = input[i++];
                    }
                }
            } else {
                buffer[j++] = input[i++];
            }
        }

        buffer[j] = '\0';
        argv[argc++] = strdup(buffer);
    }

    argv[argc] = NULL;
    if (argc_out) *argc_out = argc;
    return argv;
}
