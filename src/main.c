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
    char *duplicateInput = strdup(input);
    char* command = strtok(duplicateInput, " ");
    char* argument = strtok(NULL, " ");
    while(argument != NULL) {
        printf("%s ", argument);
        argument = strtok(NULL, " ");
    }
    printf("\n");
    free(duplicateInput);
    return;
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
        char* argumentArray[100];
        int argumentCount = 0;
        char* argument = strtok(input, " ");
        while(argument != NULL) {
            if(!isspace(argument[0])){
                argumentArray[argumentCount] = argument;
                argumentCount++;
            }
            argument = strtok(NULL, " ");
        }
        argumentArray[argumentCount] = NULL;
        pid_t pid = fork();
        if(pid == 0) {
            execv(commandPath, argumentArray);
            perror("execv in executablesInPath");
        } else if(pid < 0) {
            perror("fork in executablesInPath");
        } else {
            wait(NULL);
        }
        free(duplicateInput);
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
