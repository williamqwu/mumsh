#include "mumsh.h"

void prompt(const char *message){
    printf("%s", message);
    fflush(stdout);
}

void errMsg(const char *message){
    printf("%s", message);
    fflush(stdout);
}

void stdoutMsg(const char *message){
    printf("%s", message);
    fflush(stdout);
}

void debugMsg(const char *message){
#ifdef DEBUG
    printf("%s", message);
    fflush(stdout);
#endif // DEBUG
    char a = message[0];
    a++;
}
