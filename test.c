#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
    printf("Program was passed %d args (including program name).\n", argc);
    for(int i = 0; i < argc; i++) {
        if(i==0) {
            printf("Arg #0 (program name): %s\n", argv[i]);
        } else {
            printf("Arg #%d: %s\n", i, argv[i]);
        }
    }
}
