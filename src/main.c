#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void checkUserInput(char* input);

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
    printf("%s: command not found\n", input);
}
