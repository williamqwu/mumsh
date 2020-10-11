#ifndef IO_UTIL_H
#define IO_UTIL_H

#define MAX_LINE 1030       // maxima size of a command line (1024)
#define MAX_FILENAME 256    // maxima length of the file name (255)
#define PARM_DELIM " \t\n"  // delim used for input parsing
#define MAX_PIPED 400       // maxima count of pipeline
#define MAX_PATH 1050       // maxima path length    
#define MAX_BGPROC 50       // maxima count of background process

/* special quoted characters */
#define Q_NON_SPECIAL 0
#define Q_GREATER 1         // >
#define Q_SMALLER 2         // <
#define Q_PIPE 3            // |
#define Q_BLANK 4           // 
#define Q_NEWLINE 5         // \n

#define Q_REPLACER 20       // Device Control 4

#include "mumsh.h"

void prompt(const char *message);
void errMsg(const char *message);
void stdoutMsg(const char *message);
void debugMsg(const char *message);

#endif // IO_UTIL_H
