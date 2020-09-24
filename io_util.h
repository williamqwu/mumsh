#ifndef IO_UTIL_H
#define IO_UTIL_H

#define MAX_LINE 1030 // maxima size of a command line (1024)
#define MAX_FILENAME 256 // maxima length of the file name (255)
#define PARM_DELIM " \t\n" // delim used for input parsing

void prompt(const char *message);

// void errMsg(const char *message);

// void stdoutMsg(const char *message);

#endif // IO_UTIL_H
