#ifndef GLOBAL_H
#define GLOBAL_H

#include "mumsh.h"
#define PARENT_NORMAL 1
#define PARENT_EXIT   2
#define CHILD_NORMAL  3

char *inFileName;   // input file name related with redirection
char *outFileName;  // output file name related with redirection
char *line;         // original input line
char *conjLine;     // recombinant line
int fdStdIn;        // file descriptor for stdin
int fdStdOut;       // file descriptor for stdout

int nodeStatus;     // current status

void promptInit();
void promptExit();

#endif
