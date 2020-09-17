#include "mumsh.h"

int main(){
    while(1){
        printf("mumsh $ ");
        char *line = (char *)malloc(sizeof(char)*MAX_LINE);
        if (fgets(line, MAX_LINE, stdin) == NULL){
            printf("Error: fgets crashed.\n");
            exit(1);
        }
        // for(int i=0;i<(int)strlen(line);i++)printf("%d ",line[i]);
        printf("\n");
        char **parm = (char **)malloc(sizeof(char *)*MAX_LINE);
        for(int i=0;i<MAX_LINE;i++) parm[i]=NULL; // init the parameter array
        int parmCnt = 0;
        char *token;

        token = strtok(line, PARM_DELIM);
        while (token != NULL){
            // for(int i=0;i<(int)strlen(token);i++)printf("%d ",token[i]);
            // printf("||%d\n",parmCnt);
            parm[parmCnt] = (char *)malloc(sizeof(char)*(strlen(token)+1));
            memset(parm[parmCnt],0,strlen(token)+1); // init parameter
            strcpy(parm[parmCnt], token);
            parmCnt++;
            token = strtok(NULL, PARM_DELIM);
        }
        // for(int i=0;i<parmCnt;i++){
        //     printf("[%d]: %s\n",i,parm[i]);
        // }
        if (!strcmp(parm[0],"exit")){
            printf("exit\n");
            free(line);
            for(int i=0;i<parmCnt;i++) free(parm[i]);
            free(parm);
            exit(0);
        }
        else {
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
                        printf("The background process [%d] need to be terminated!\n", (int)tmpPid); // TODO: background process
                    }
                } while (tmpPid != childPid);
            }
            printf("Child process status: %d\n",childStatus);
        }
        
        free(line);
        for(int i=0;i<parmCnt;i++) free(parm[i]);
        free(parm);

    }
    return 0;
}
