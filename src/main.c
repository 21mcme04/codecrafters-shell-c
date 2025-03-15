#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define validCommands 3
char* commands[validCommands] = {"echo", "type", "exit"};

void checkUserInput(char* input);
void echo(char* input);
void type(char* input);
int isValidCommand(char* command);
void findCommandInPath(char* input, char* path);

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
    }
    free(duplicateInput);
    printf("%s: command not found\n", input);
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
            findCommandInPath(argumentArray[iteration], path);
        }
    }
    free(duplicateInput);
    return;
}

void findCommandInPath(char* input, char* path) {
    if(input == NULL || path == NULL) {
        return;
    }
    char* pathCopy = strdup(path);
    if(!pathCopy) {
        perror("strdup in findCommandInPath");
        return;
    }
    char* pathToken = strtok(pathCopy, ":");
    while(pathToken != NULL) {
        char* commandPath = malloc(strlen(pathToken) + strlen(input) + 2);
        if(!commandPath) {
            perror("malloc in findCommandInPath");
            free(pathCopy);
            return;
        }
        sprintf(commandPath, "%s/%s", pathToken, input);
        if(access(commandPath, X_OK) == 0) {
            printf("%s is %s\n", input, commandPath);
            free(commandPath);
            free(pathCopy);
            return;
        }
        free(commandPath);
        pathToken = strtok(NULL, ":");
    }
    free(pathCopy);
    printf("%s: not found\n", input);
    return;
}
