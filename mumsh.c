#include "mumsh.h"

int main(){
    char *line = (char *)malloc(sizeof(char)*MAX_LINE);
    if (fgets(line, MAX_LINE, stdin) == NULL){
        printf("Error: fgets crashed.\n");
        exit(1);
    }
    char **parm = (char **)malloc(sizeof(char *)*MAX_LINE);
    int parmCnt = 0;
    char *token;

    token = strtok(line, PARM_DELIM);
    while (token != NULL){
        parm[parmCnt] = (char *)malloc(sizeof(char)*(strlen(token)+1));
        memset(parm[parmCnt],0,strlen(token)+1);
        strcpy(parm[parmCnt], token);
        parmCnt++;
        token = strtok(NULL, PARM_DELIM);
    }


    
    free(line);
    for(int i=0;i<parmCnt;i++) free(parm[i]);
    free(parm);
    return 0;
}
