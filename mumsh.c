#include "mumsh.h"

int main(){
    while(1){
        printf("mumsh $ ");
        /**
         * initialization     
         */
        int isInRed=0, isOutRed=0, isOutApp=0;
        int desIn, desOut;
        int stdIn = dup(0), stdOut = dup(1);
        char *inFileName = (char *)malloc(sizeof(char)*FILENAME_MAX);
        char *outFileName = (char *)malloc(sizeof(char)*FILENAME_MAX);
        char *line = (char *)malloc(sizeof(char)*MAX_LINE);
        if (fgets(line, MAX_LINE, stdin) == NULL){
            printf("Error: fgets crashed.\n");
            free(line);
            exit(1);
        }
        // for(int i=0;i<(int)strlen(line);i++)printf("%d ",line[i]);
        // printf("\n");
        char **parm = (char **)malloc(sizeof(char *)*MAX_LINE);
        for(int i=0;i<MAX_LINE;i++) parm[i]=NULL; // init the parameter array
        int parmCnt = 0;
        char *token;

        token = strtok(line, PARM_DELIM);
        while (token != NULL){
            // for(int i=0;i<(int)strlen(token);i++)printf("%d ",token[i]);
            // printf("||%d\n",parmCnt);
            if (token[0]=='>'){
                if(strlen(token)>1 && token[1]=='>'){
                    isOutApp=1;
                    if(strlen(token)==2){
                        token = strtok(NULL, PARM_DELIM);
                        memset(outFileName, 0, FILENAME_MAX);
                        strcpy(outFileName, token);
                        token = strtok(NULL, PARM_DELIM);
                        continue;
                    }
                    else{
                        memset(outFileName, 0, FILENAME_MAX);
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
                        memset(outFileName, 0, FILENAME_MAX);
                        strcpy(outFileName, token);
                        token = strtok(NULL, PARM_DELIM);
                        continue;
                    }
                    else{
                        memset(outFileName, 0, FILENAME_MAX);
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
                    memset(inFileName, 0, FILENAME_MAX);
                    strcpy(inFileName, token);
                    token = strtok(NULL, PARM_DELIM);
                    continue;
                }
                else{
                    memset(inFileName, 0, FILENAME_MAX);
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
        // for(int i=0;i<parmCnt;i++){
        //     printf("[%d]: %s\n",i,parm[i]);
        // }
        // if(isInRed)printf("inred\n");
        // if(isOutApp)printf("outapp\n");
        // if(isOutRed)printf("outred\n");

        if (!strcmp(parm[0],"exit")){
            printf("exit\n");
            free(line);
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
            }
            if(isOutRed){
                desOut = open(outFileName, O_WRONLY | O_CREAT | O_TRUNC);
                dup2(desOut, 1); // replace stdout(1) with desOut
                close(desOut);
            }
            if(isOutApp){
                desOut = open(outFileName, O_WRONLY | O_CREAT | O_APPEND);
                dup2(desOut, 1);
                close(desOut);
            }
            /**
             * execution     
             */

            pid_t childPid;
            int childStatus = 0;
            if ((childPid = fork()) < 0){ // fork failed
                printf("Error: fork failed.\n");
                exit(0);
            }
            else if (childPid == 0){ // executed by child process
                if (execvp(parm[0],parm) < 0){
                    printf("Error: execvp not working.\n");
                    exit(0);
                }
            }
            else{ // executed by parnet process
                /* wait for child process */
                pid_t tmpPid;
                do{
                    tmpPid = wait(&childStatus);
                    if(tmpPid != childPid){
                        printf("Error: The background process [%d] need to be terminated!\n", (int)tmpPid); // TODO: background process
                    }
                } while (tmpPid != childPid);
            }
            // printf("Child process status: %d\n",childStatus);
        }
        
        dup2(stdIn, 0);
        dup2(stdOut, 1);

        free(line);
        for(int i=0;i<parmCnt;i++) free(parm[i]);
        free(parm);

    }
    return 0;
}
