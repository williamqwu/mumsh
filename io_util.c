#include "mumsh.h"

void prompt(const char *message){
    pid_t pid = fork();
    if (pid<0){
        printf("Error: fork failed.\n");
        exit(0);
    }
    else if (pid==0){ // child
        printf("%s", message);
        exit(0);
    }
    else{ // parent
        wait(NULL);
    }
}
