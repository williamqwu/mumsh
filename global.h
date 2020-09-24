#ifndef GLOBAL_H
#define GLOBAL_H

#include "mumsh.h"

char *inFileName;   // input file name related with redirection
char *outFileName;  // output file name related with redirection
char *line;         // original input line
int fdStdIn;        // file descriptor for stdin
int fdStdOut;       // file descriptor for stdout

void promptInit();
void promptExit();

#endif
