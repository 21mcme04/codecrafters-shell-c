#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define validCommands 3
char* commands[validCommands] = {"echo", "type", "exit"};

void checkUserInput(char* input);
void echo(char* input);
void type(char* input);
int isValidCommand(char* command);

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
    long inputSize = strlen(input);
    if (inputSize > 0 && input[inputSize - 1] == '\n') {
        input[inputSize - 1] = '\0';
    }
    if(strcmp(input, "exit 0") == 0) {
        exit(0);
    }

    char *duplicateInput = strdup(input);
    char* command = strtok(duplicateInput, " ");
    if(strcmp(command, "echo") == 0) {
        echo(input);
        return;
    } else if(strcmp(command, "type") == 0) {
        type(input);
        return;
    }
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
    char* command = strtok(duplicateInput, " ");
    char* argument = strtok(NULL, " ");
    while(argument != NULL) {
        if(isValidCommand(argument)) {
            printf("%s is a shell builtin\n", argument);
        } else {
            printf("%s: not found\n", argument);
        }
        argument = strtok(NULL, " ");
    }
    return;
}
