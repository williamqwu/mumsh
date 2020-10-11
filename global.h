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
char *bgCommand[MAX_BGPROC];    // background process command 
int fdStdIn;        // file descriptor for stdin
int fdStdOut;       // file descriptor for stdout

int nodeStatus;     // current status
int lastPid[MAX_PIPED];

char *lastDir;      // last directory
char *lastPendingDir;   // last pending directory
const char *homedir;

#define PROC_DONE     1
#define PROC_RUNNING  2
int isBackground;   // background status
int bgCnt;          // current working background process; NOTE: job ID starts from 1
int bgJob[2*MAX_BGPROC]; // status Array

void promptInit();
void promptExit();

#endif
