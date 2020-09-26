#include "global.h"

void promptInit(){
    inFileName = (char *)malloc(sizeof(char)*MAX_FILENAME);
    outFileName = (char *)malloc(sizeof(char)*MAX_FILENAME);
    line = (char *)malloc(sizeof(char)*MAX_LINE);
    fdStdIn = dup(0);
    fdStdOut = dup(1);
}

void promptExit(){
    dup2(fdStdIn, 0);
    dup2(fdStdOut, 1);
    free(inFileName);
    free(outFileName);
}