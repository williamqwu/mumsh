#include "mumsh.h"

int main(){
    while(1){
        prompt("mumsh $ ");
        /**
         * initialization     
         */
        int isInRed=0, isOutRed=0, isOutApp=0; // redirection status
        int desIn, desOut; // file descriptor
        int pipedCnt = 1;
        int pipedInitAddr[MAX_PIPED];
        memset(pipedInitAddr,0,MAX_PIPED);
        promptInit();

        if (fgets(line, MAX_LINE, stdin) == NULL){
            debugMsg("Error: fgets crashed.\n");
            free(line);
            promptExit();
            // exit(1);
            continue;
        }
        /**
         * pre-processing LINE     
         */
        char *tmpLine = (char *)malloc(sizeof(char)*MAX_LINE*2);
        memset(tmpLine,0,MAX_LINE*2);
        for(unsigned int i=0,j=0;i<strlen(line);){
            if(line[i]!='<' && line[i]!='>') tmpLine[j++]=line[i++];
            else{
                tmpLine[j++]=' ';
                tmpLine[j++]=line[i++];
                if(i<strlen(line)&&line[i]=='>') tmpLine[j++]=line[i++];
                if(i<strlen(line)&&line[i]!=' ') tmpLine[j++]=' ';
            }
        }
        free(line);
        /**
         * pre-processing pipeline     
         */
        // TODO: check whether the pipe is valid
        for(unsigned int i=0;i<strlen(tmpLine);i++){
            if(tmpLine[i]=='|'){
                pipedInitAddr[pipedCnt]=i+1;
                pipedCnt++;
            }
        }
        /**
         * processing pipeline     
         */
        for(int k=1;k<=pipedCnt;k++){
            int pipeFd[2]; // pipe file descriptor
            char pipeBuf;  // pipe reading buffer
            /**
             * extracting segmant     
             */
            int initAddress=pipedInitAddr[k-1], offsetAddress=pipedInitAddr[k]-pipedInitAddr[k-1]-1;
            char *segLine = (char *)malloc(sizeof(char)*MAX_LINE*2);
            memset(segLine,0,MAX_LINE*2);
            strncpy(segLine,pipedCnt+initAddress,offsetAddress);

            /**
             * processing redirection     
             */
            char **parm = (char **)malloc(sizeof(char *)*MAX_LINE);
            for(int i=0;i<MAX_LINE;i++) parm[i]=NULL; // init the parameter array
            int parmCnt = 0;
            char *token;
            token = strtok(segLine, PARM_DELIM);
            while (token != NULL){
                // original redirection
                if (token[0]=='>'){
                    if(strlen(token)>1 && token[1]=='>'){
                        isOutApp=1;
                        if(strlen(token)==2){
                            token = strtok(NULL, PARM_DELIM);
                            memset(outFileName, 0, MAX_FILENAME);
                            strcpy(outFileName, token);
                            token = strtok(NULL, PARM_DELIM);
                            continue;
                        }
                        else{
                            memset(outFileName, 0, MAX_FILENAME);
                            strcpy(outFileName, token);
                            memmove(outFileName, outFileName+2, strlen(outFileName));
                            token = strtok(NULL, PARM_DELIM);
                            continue;
                        }
                    }
                    else{
                        isOutRed=1;
                        if(strlen(token)==1){
                            token = strtok(NULL, PARM_DELIM);
                            memset(outFileName, 0, MAX_FILENAME);
                            strcpy(outFileName, token);
                            token = strtok(NULL, PARM_DELIM);
                            continue;
                        }
                        else{
                            memset(outFileName, 0, MAX_FILENAME);
                            strcpy(outFileName, token);
                            memmove(outFileName, outFileName+1, strlen(outFileName));
                            token = strtok(NULL, PARM_DELIM);
                            continue;
                        }
                    }
                }
                else if (token[0]=='<'){
                    isInRed=1;
                    if(strlen(token)==1){
                        token = strtok(NULL, PARM_DELIM); // TODO: Error: emtpy inFileName
                        memset(inFileName, 0, MAX_FILENAME);
                        strcpy(inFileName, token);
                        token = strtok(NULL, PARM_DELIM);
                        continue;
                    }
                    else{
                        memset(inFileName, 0, MAX_FILENAME);
                        strcpy(inFileName, token);
                        memmove(inFileName, inFileName+1, strlen(inFileName));
                        token = strtok(NULL, PARM_DELIM);
                        continue;
                    }
                } // TODO: clean the code

                parm[parmCnt] = (char *)malloc(sizeof(char)*(strlen(token)+1));
                memset(parm[parmCnt], 0, strlen(token)+1); // init parameter
                strcpy(parm[parmCnt], token);
                parmCnt++;
                token = strtok(NULL, PARM_DELIM);
            }
            if(isInRed) debugMsg("inred\n");
            if(isOutApp) debugMsg("outapp\n");
            if(isOutRed) debugMsg("outred\n");
            
            if(k<pipedCnt){
                if(pipe(pipeFd) == -1){
                    errMsg("Error: pipe failure.\n"); // handle error
                    exit(0);
                }
            }

            /*
             * exe
             */
            if (!strcmp(parm[0],"exit")){
                stdoutMsg("exit\n");
                free(tmpLine);
                promptExit();
                for(int i=0;i<parmCnt;i++) free(parm[i]);
                free(parm);
                exit(0);
            }
            else {
                /**
                 * redirection     
                 */
                if(isInRed){
                    desIn = open(inFileName,O_RDONLY);
                    dup2(desIn, 0); // replace stdin(0) with desIn
                    close(desIn);
                } // TODO: check the parameters
                if(isOutRed){
                    desOut = open(outFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                    dup2(desOut, 1); // replace stdout(1) with desOut
                    close(desOut);
                }
                if(isOutApp){
                    desOut = open(outFileName, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
                    dup2(desOut, 1);
                    close(desOut);
                }
                /**
                 * execution     
                 */

                pid_t childPid;
                int childStatus = 0;
                if ((childPid = fork()) < 0){ // fork failed
                    errMsg("Error: fork failed.\n");
                    exit(0);
                }
                else if (childPid == 0){ // executed by child process
                    if (execvp(parm[0],parm) < 0){
                        errMsg("Error: execvp not working.\n");
                        exit(0);
                    }
                    else{
                        exit(0);
                    }
                }
                else{ // executed by parnet process
                    /* wait for child process */
                    pid_t tmpPid;
                    do{
                        tmpPid = wait(&childStatus);
                        if(tmpPid != childPid){
                            char tmpMsg[1024];
                            sprintf(tmpMsg, "Error: The background process [%d] need to be terminated!\n", (int)tmpPid);
                            errMsg(tmpMsg); // TODO: background process
                        }
                    } while (tmpPid != childPid);
                }
                char tmpMsg[108];
                sprintf(tmpMsg,"Child process status: %d\n",childStatus);
                debugMsg(tmpMsg);
            }
            /* pipe fork */
            if(k<pipedCnt){
                pid_t tmpPid = fork();
                if(tmpPid==-1){
                    errMsg("Error: fork failure.\n");
                    exit(0);
                }
                else if(tmpPid==0){ // child reads from pipe; only the child will enter the loop
                    close(pipeFd[1]); // child closes write
                    while (read(pipeFd[0], &pipeBuf, 1) > 0) write(STDOUT_FILENO, &pipeBuf, 1);
                    write(STDOUT_FILENO, "\n", 1);
                    close(pipeFd[0]);
                    // go on
                }
                else { // parent writes to pipe
                    close(pipeFd[0]);
                    write(pipeFd[1],"something",length("something"));
                    close(pipeFd[1]); // reader sees EOF
                    wait(NULL); // wait for child
                    if(k<pipedCnt) exit(0);
                    else if(k==pipedCnt) break;
                }
            }
            /* releasing memory */
            for(int i=0;i<parmCnt;i++) free(parm[i]);
            free(parm);            
            free(segLine);
        }
        free(tmpLine);
        promptExit();
    }
    return 0;
}
