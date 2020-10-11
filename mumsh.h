#ifndef MEMSH_H
#define MEMSH_H
/* Header */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      // file control options
#include <unistd.h>     // UNIX Standard
#include <signal.h>     // signal handling
#include <errno.h>      // error number
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   
#include <sys/stat.h>
#include <pwd.h>        // current directory

#include "io_util.h"    // io-related functions
#include "global.h"     // global variables

// #define DEBUG

#endif // MEMSH_H
