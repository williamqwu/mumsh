#include "mumsh.h"
// void cHandler(){
//     // signal(SIGINT, cHandler);
//     debugMsg("Info: sending Ctrl-C\n");
//     if(nodeStatus == PARENT_NORMAL) {
//         debugMsg("Info: Node Status-Parent Normal.\n");
//         nodeStatus = PARENT_EXIT;
//     }
//     else if(nodeStatus == CHILD_NORMAL){
//         debugMsg("Info: Node Status-Child Normal.\n");
//         exit(0);
//     }
// }
struct sigaction old_action;
struct sigaction action;
void sigint_handler()
{
    sigaction(SIGINT, &old_action, NULL);
    debugMsg("Info: sending Ctrl-C\n");
    if(nodeStatus == PARENT_NORMAL) {
        debugMsg("Info: Node Status-Parent Normal.\n");
        stdoutMsg("\n");
        nodeStatus = PARENT_EXIT;
    }
    else if(nodeStatus == CHILD_NORMAL){
        debugMsg("Info: Node Status-Child Normal.\n");
        exit(0);
    }
}

int main(){
    // signal(SIGINT, cHandler);
    action.sa_handler = &sigint_handler;
    bgCnt = 0;
    for(int i=0;i<MAX_BGPROC;i++) bgCommand[i]=NULL;
    lastDir = NULL;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    lastPendingDir = (char *)malloc(sizeof(char)*MAX_PATH);
    memset(lastPendingDir,0,MAX_PATH);
    // strcpy(lastPendingDir,homedir); // TODO: current directory
    while(1){
        sigaction(SIGINT, &action, &old_action);
        nodeStatus = PARENT_NORMAL;
        prompt("mumsh $ ");
        
        /** Initialization
         * @isInRed, isOutRed, isOutApp: redirection status
         */
        int isInRed=0, isOutRed=0, isOutApp=0; // redirection status
        int desIn, desOut; // file descriptor
        int pipedInitAddr[MAX_PIPED];
        memset(pipedInitAddr,0,MAX_PIPED);
        promptInit();
        /** Input
         * @line: original line of commands
         * @conjLine: pre-processed line of commands
         */
        int isInputNotEnd = 0, isFirstfgets = 1;
        int isSQuoNotClosed = 0, isDQuoNotClosed = 0;
        int fgetsErrorFlag = 0;
        while(isInputNotEnd || isFirstfgets){
            isInputNotEnd = 0;
            if(isFirstfgets) isFirstfgets = 0;
            else prompt("> ");
            if(fgets(line, MAX_LINE, stdin) == NULL){
                debugMsg("Info: fgets quit.\n");
                // if(feof(stdin)) {fgetsErrorFlag=1; break;}
                if(errno == EINTR) {errno=0; fgetsErrorFlag=1; break;} // fgets meet SIGINT
                stdoutMsg("exit\n");
                free(line);
                free(conjLine);
                promptExit();
                for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
                if(lastDir!=NULL) free(lastDir);
                free(lastPendingDir);
                exit(0);
            }
            /* check incomplete quotation mark and update the flag */
            for(unsigned int i=0;i<strlen(line);i++){
                if(line[i]=='\''){
                    if(!isSQuoNotClosed && !isDQuoNotClosed) isSQuoNotClosed = 1;
                    else if(isSQuoNotClosed) isSQuoNotClosed = 0;
                }
                else if(line[i]=='\"'){
                    if(!isSQuoNotClosed && !isDQuoNotClosed) isDQuoNotClosed = 1;
                    else if(isDQuoNotClosed && (i==0 || (i>0 && line[i-1]!='\\'))) isDQuoNotClosed = 0;
                }
            }
            if(isSQuoNotClosed || isDQuoNotClosed){
                strcat(conjLine, line);
                isInputNotEnd = 1;
                continue;
            }
            if(strlen(line)>1) for(unsigned int i=strlen(line)-2;i>=0;i--){ // assume that the last character is newline
                if(line[i]==' ') continue;
                else if(line[i]=='>' || line[i]=='<' || line[i]=='|'){
                    isInputNotEnd = 1;
                    break;
                }
                else break;
            }
            if(isInputNotEnd){
                line[strlen(line)-1] = ' '; // replace newline with blank
                strcat(conjLine, line);
                /* check for syntax error */
                for(unsigned int k=strlen(conjLine)-1;k>0;k--){
                    int inBranch = 0;
                    if(conjLine[k]=='>' || conjLine[k]=='<' || conjLine[k]=='|'){
                        if(k<=0)break;
                        inBranch = 1;
                        char kk = conjLine[k];
                        k--;
                        if(k<=0)break;
                        while(k>0 && (conjLine[k]==' ' || conjLine[k]=='\n')) k--;
                        if(conjLine[k]=='>' || conjLine[k]=='<'){
                            char tmpMsg[MAX_LINE];
                            sprintf(tmpMsg,"syntax error near unexpected token `%c\'\n",kk);
                            errMsg(tmpMsg);
                            fgetsErrorFlag=1;
                        }
                    }
                    if(inBranch) break;
                }
                if(fgetsErrorFlag) break;
                continue;
            }
            /* concatenation */
            strcat(conjLine, line);
            isInputNotEnd = 0; // break;
        }
        free(line);
        if(fgetsErrorFlag==1 || nodeStatus==PARENT_EXIT){
            free(conjLine);
            promptExit();
            continue;
        }
        fflush(stdin);

        /** Parsing conjLine
         * EFFECTS: validate whether the input is emtpy
         */
        int isEmptyLine=1;
        for(unsigned int i=0;i<strlen(conjLine);i++){
            if(conjLine[i]!=32 && conjLine[i]!=10) isEmptyLine=0;
        }
        if(isEmptyLine){
            debugMsg("Info: Empty line.\n");
            free(conjLine);
            promptExit();
            continue;
        }

        /** Parsing background character
         */
        if(conjLine[strlen(conjLine)-2] == '&'){ // TODO: move downwards
            isBackground = 1;
            bgCommand[bgCnt] = (char *)malloc(sizeof(char)*MAX_LINE);
            memset(bgCommand[bgCnt],0,MAX_LINE);
            strcpy(bgCommand[bgCnt], conjLine);
            bgCommand[bgCnt][strlen(bgCommand[bgCnt])-1] = '\0';
        }

        /** Parsing quotation mark
         * 
         */
        isSQuoNotClosed = 0;
        isDQuoNotClosed = 0;
        int specialCnt = 0;
        char specialList[MAX_LINE];
        memset(specialList,0,MAX_LINE);
        unsigned int deleteCnt = 0;
        unsigned int deleteList[MAX_LINE];
        for(unsigned int i=0;i<strlen(conjLine)-1;i++){ // omitted the newline in the end
            if(isSQuoNotClosed || isDQuoNotClosed){
                if(conjLine[i]=='>') {conjLine[i]=Q_REPLACER;specialList[specialCnt++]='>';}
                if(conjLine[i]=='<') {conjLine[i]=Q_REPLACER;specialList[specialCnt++]='<';}
                if(conjLine[i]=='|') {conjLine[i]=Q_REPLACER;specialList[specialCnt++]='|';}
                if(conjLine[i]==' ') {conjLine[i]=Q_REPLACER;specialList[specialCnt++]=' ';}
                if(conjLine[i]=='\n') {conjLine[i]=Q_REPLACER;specialList[specialCnt++]='\n';}
            }
            if(conjLine[i]=='\''){
                if(!isSQuoNotClosed && !isDQuoNotClosed){
                    deleteList[deleteCnt++] = i; // DEL '
                    isSQuoNotClosed = 1;
                }
                else if(isSQuoNotClosed){
                    deleteList[deleteCnt++] = i; // DEL '
                    isSQuoNotClosed = 0;
                }
            }
            else if(conjLine[i]=='\"'){
                if(!isSQuoNotClosed && !isDQuoNotClosed){
                    deleteList[deleteCnt++] = i; // DEL "
                    isDQuoNotClosed = 1;
                }
                else if(isDQuoNotClosed && i>0 && conjLine[i-1]=='\\'){
                    deleteList[deleteCnt++] = i; // DEL slash
                }
                else if(isDQuoNotClosed && (i==0 || (i>0 && conjLine[i-1]!='\\'))){
                    deleteList[deleteCnt++] = i; // DEL "
                    isDQuoNotClosed = 0;
                }
            }
        }
        char tmpLine[MAX_LINE];
        memset(tmpLine,0,MAX_LINE);
        for(unsigned int i=0,j=0,k=0;i<strlen(conjLine);i++){
            if(j>=deleteCnt || (j<deleteCnt && i!=deleteList[j])) tmpLine[k++]=conjLine[i];
            else if(j<deleteCnt && i==deleteList[j]) j++;
        }
        memset(conjLine,0,MAX_LINE);
        strcpy(conjLine,tmpLine);
        /** Parsing redirection
         * @Sline: conjLine with keywords >, <, >> separated by space
         */
        char *sLine = (char *)malloc(sizeof(char)*MAX_LINE*2);
        memset(sLine,0,MAX_LINE*2);
        for(unsigned int i=0,j=0;i<strlen(conjLine);){
            if(conjLine[i]!='<' && conjLine[i]!='>' && conjLine[i]!='|') sLine[j++]=conjLine[i++];
            else{
                sLine[j++]=' ';
                sLine[j++]=conjLine[i++];
                if(i<strlen(conjLine)&&conjLine[i]=='>') sLine[j++]=conjLine[i++];
                if(i<strlen(conjLine)&&conjLine[i]!=' ') sLine[j++]=' ';
            }
        }
        free(conjLine);
        char *dupsLine = (char *)malloc(sizeof(char)*MAX_LINE*2);
        memset(dupsLine,0,MAX_LINE*2);
        strcpy(dupsLine,sLine);
        /** Tokenize the line; parsing redirection symbols
         * @mArgv: arguments from the input line
         * @mArgc: argument count in the input line
         */
        char **mArgv = (char **)malloc(sizeof(char *)*MAX_LINE); // arguments from the input line
        for(int i=0;i<MAX_LINE;i++) mArgv[i]=NULL; // init the parameter array
        int mArgc = 0; // argument count in the input line
        char *token;
        token = strtok(sLine, PARM_DELIM);
        int ioErrorFlag = 0;
        while (token != NULL){
            // original redirection
            if (token[0]=='>'){
                if(isOutApp==1 || isOutRed==1){
                    if(!ioErrorFlag) errMsg("error: duplicated output redirection\n");
                    ioErrorFlag = 1;
                }
                if(strlen(token)>1 && token[1]=='>'){
                    if(strlen(token)>2 && token[2]=='>'){
                        if(!ioErrorFlag) errMsg("syntax error near unexpected token `>\'\n");
                        ioErrorFlag = 1;
                    }
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
                if(isInRed==1){
                    if(!ioErrorFlag) errMsg("error: duplicated input redirection\n");
                    ioErrorFlag = 1;
                }
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
            }

            mArgv[mArgc] = (char *)malloc(sizeof(char)*(strlen(token)+1));
            memset(mArgv[mArgc], 0, strlen(token)+1); // init parameter
            strcpy(mArgv[mArgc], token);
            mArgc++;
            token = strtok(NULL, PARM_DELIM);
        }
        free(sLine);
        if(isInRed) debugMsg("inred\n");
        if(isOutApp) debugMsg("outapp\n");
        if(isOutRed) debugMsg("outred\n");

        /* I/O error handling */
        char **tmpArgv = (char **)malloc(sizeof(char *)*MAX_LINE); // arguments from the input line
        for(int i=0;i<MAX_LINE;i++) tmpArgv[i]=NULL; // init the parameter array
        int tmpArgc = 0; // argument count in the input line
        char *tmpToken;
        tmpToken = strtok(dupsLine, PARM_DELIM);
        while (tmpToken != NULL){
            tmpArgv[tmpArgc] = (char *)malloc(sizeof(char)*(strlen(tmpToken)+1));
            memset(tmpArgv[tmpArgc], 0, strlen(tmpToken)+1); // init parameter
            strcpy(tmpArgv[tmpArgc], tmpToken);
            tmpArgc++;
            tmpToken = strtok(NULL, PARM_DELIM);
        }
        int dupFlag=0;
        if(!ioErrorFlag) for(int i=0;i<tmpArgc;i++){
            if(tmpArgv[i][0]=='|') dupFlag=1;
            if(tmpArgv[i][0]=='<' && dupFlag){
                errMsg("error: duplicated input redirection\n");
                ioErrorFlag = 1;
                break;
            }
        }
        dupFlag=0;
        for(int i=0;i<tmpArgc;i++){
            if(ioErrorFlag) break;
            if(i==0 && tmpArgv[i][0]=='|'){
                errMsg("error: missing program\n");
                ioErrorFlag = 1;
                break;
            }
            if(tmpArgv[i][0]=='|' && i<tmpArgc-1 && tmpArgv[i+1][0]=='|'){
                errMsg("error: missing program\n");
                ioErrorFlag = 1;
                break;
            }
            if(tmpArgv[i][0]=='>' && i<tmpArgc-1 && tmpArgv[i+1][0]=='>'){
                errMsg("syntax error near unexpected token `>\'\n");
                ioErrorFlag = 1;
                break;
            }
            if(tmpArgv[i][0]=='>' && i<tmpArgc-1 && tmpArgv[i+1][0]=='<'){
                errMsg("syntax error near unexpected token `<\'\n");
                ioErrorFlag = 1;
                break;
            }
            if(tmpArgv[i][0]=='>' && i<tmpArgc-1 && tmpArgv[i+1][0]=='|'){
                errMsg("syntax error near unexpected token `|\'\n");
                ioErrorFlag = 1;
                break;
            }
            if(tmpArgv[i][0]=='>') dupFlag=1;
            if(tmpArgv[i][0]=='|' && dupFlag){
                errMsg("error: duplicated output redirection\n");
                ioErrorFlag = 1;
                break;
            }
        }
        for(int i=0;i<tmpArgc;i++) if(tmpArgv[i]!=NULL) free(tmpArgv[i]);
        free(tmpArgv);
        free(dupsLine);
        if(ioErrorFlag){
            for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
            free(mArgv);
            promptExit();
            continue;
        }

        /** Parsing pipe
         * @pipeCnt: number of pipes in the line
         * @cmdHeadDict: location dictionary of command heads
         * @cmdCnt: number of commands in the line
         */
        int pipeCnt=0; // number of pipes in the line
        int cmdHeadDict[MAX_PIPED]; // location dictionary of command heads
        cmdHeadDict[0]=0;
        for(int i=0,j=1;i<mArgc;i++){
            if(!strcmp(mArgv[i], "|")){
                pipeCnt++;
                free(mArgv[i]);
                mArgv[i]=NULL;
                cmdHeadDict[j++]=i+1;
            }
            else if(!strcmp(mArgv[i], "&")) {free(mArgv[i]); mArgv[i]=NULL;}
        }
        int cmdCnt = pipeCnt + 1; // number of commands in the line
        cmdHeadDict[cmdCnt] = mArgc + 1; // for the purpose of calculating offset
        
        /** Recreating special characters
         */ 
        int spIndex = 0;
        for(int i=0;i<mArgc;i++){
            if(mArgv[i]==NULL) continue;
            for(unsigned int j=0;j<strlen(mArgv[i]);j++){
                if(mArgv[i][j] == Q_REPLACER && spIndex<specialCnt) mArgv[i][j] = specialList[spIndex++];
            }
        }

        /** Creating pipe
         * @pipeFd: file descriptor for read/write ends of pipes
         */
        int pipeFd[pipeCnt*2+2]; // file descriptor for read/write ends of pipes
        for(int i=0;i<pipeCnt;i++){
            if(pipe(pipeFd + i*2) < 0){
                debugMsg("Error: pipe failure.\n");
                for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
                free(mArgv);
                promptExit();
                for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
                if(lastDir!=NULL) free(lastDir);
                exit(0);
            }
        }

        /** Executing commands
         * @childStatus
         */
        int childStatus;
        for(int index=0;index<cmdCnt;index++){
            int cmdHead = cmdHeadDict[index];
            int cmdOffset = cmdHeadDict[index+1] - cmdHead - 1;
            /* checking exit */
            if(!strcmp(mArgv[cmdHead],"exit")){
                stdoutMsg("exit\n");
                for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
                free(mArgv);
                promptExit();
                for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
                if(lastDir!=NULL) free(lastDir);
                exit(0);
            }
            /* checking build-in*/
            if(!strcmp(mArgv[cmdHead],"cd")){
                fflush(NULL);
                if(cmdOffset==1 || (cmdOffset==2 && !strcmp(mArgv[cmdHead+1],"~"))){
                    chdir(homedir);
                    if(lastDir==NULL) lastDir = (char *)malloc(sizeof(char)*MAX_PATH);
                    strcpy(lastDir,lastPendingDir);
                    strcpy(lastPendingDir,homedir);
                }
                else{
                    if(!strcmp(mArgv[cmdHead+1],"-")){ // FIXME: - (last two dir)
                        if(lastDir==NULL){
                            debugMsg("No last dir!\n");
                        }
                        else{
                            chdir(lastDir);
                            stdoutMsg(lastDir);
                            stdoutMsg("\n");
                        }
                    }
                    else{
                        int cdStatus = chdir(mArgv[cmdHead+1]);
                        if(cdStatus < 0){
                            debugMsg("Error: cd not working.\n");
                            errMsg(mArgv[cmdHead+1]);
                            errMsg(": No such file or directory\n");
                        }
                        else{
                            if(lastDir==NULL) lastDir = (char *)malloc(sizeof(char)*MAX_PATH);
                            memset(lastDir,0,MAX_PATH);
                            strcpy(lastDir,lastPendingDir);
                            strcpy(lastPendingDir,mArgv[cmdHead+1]);
                        }
                    }
                }
                continue;
            }
            if(!strcmp(mArgv[cmdHead],"jobs")){
                // if(bgCnt==0)stdoutMsg("\n");
                for(int i=0;i<bgCnt;i++){
                    char tmpMsg[MAX_LINE];
                    if(waitpid(bgJob[i*2],NULL,WNOHANG)==0) sprintf(tmpMsg,"[%d] running %s\n",i+1,bgCommand[i]);
                    else sprintf(tmpMsg,"[%d] done %s\n",i+1,bgCommand[i]);
                    stdoutMsg(tmpMsg);
                }
                continue;
            }

            /* forking */
            pid_t pid = fork(); // TODO: check cd running in background
            lastPid[index] = pid;
            if(pid > 0 && isBackground==1 && index==0){
                bgJob[bgCnt*2] = pid;
                bgJob[bgCnt*2+1] = PROC_RUNNING;
                bgCnt++;
                char tmpMsg[MAX_LINE];
                sprintf(tmpMsg,"[%d] %s\n",bgCnt,bgCommand[bgCnt-1]);
                stdoutMsg(tmpMsg);
            }
            
            if(pid < 0){ // fork error
                debugMsg("Error: fork failed.\n");
                for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
                free(mArgv);
                promptExit();
                for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
                if(lastDir!=NULL) free(lastDir);
                exit(0);
            }
            else if (pid == 0){ // child process
                sigaction(SIGINT, &action, &old_action);
                nodeStatus = CHILD_NORMAL;
                /* connecting child pipeFd */
                if(index+1 < cmdCnt){ // not the last command
                    if(pipeCnt>0 && dup2(pipeFd[index*2+1], 1) <= 0){
                        debugMsg("Error: dup2-stdout failure.\n");
                        for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
                        free(mArgv);
                        promptExit();
                        for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
                        if(lastDir!=NULL) free(lastDir);
                        exit(0);
                    }
                }
                if(index!=0){ // not the first command
                    if(pipeCnt>0 && dup2(pipeFd[index*2-2], 0) < 0){
                        debugMsg("Error: dup2-stdin failure.\n");
                        for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
                        free(mArgv);
                        promptExit();
                        for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
                        if(lastDir!=NULL) free(lastDir);
                        exit(0);
                    }
                }
                /* checking redirection */
                if(index==0 && isInRed){
                    desIn = open(inFileName,O_RDONLY); // TODO: check the parameters
                    if(desIn<=0){
                        if(errno == ENOENT){
                            errMsg(inFileName);
                            errMsg(": No such file or directory\n");
                            exit(0);
                        }
                    }
                    dup2(desIn, 0); // replace stdin(0) with desIn
                    close(desIn);
                }
                if(index+1==cmdCnt && isOutRed){
                    desOut = open(outFileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
                    if(desOut<=0){
                        if(errno==EPERM || errno==EROFS){ // not permitted
                            errMsg(outFileName);
                            errMsg(": Permission denied\n");
                            // TODO
                            exit(0);
                        }
                    }
                    dup2(desOut, 1); // replace stdout(1) with desOut
                    close(desOut);
                }
                if(index+1==cmdCnt && isOutApp){
                    desOut = open(outFileName, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
                    if(desOut<=0){
                        if(errno==EPERM || errno==EROFS){ // not permitted
                            errMsg(outFileName);
                            errMsg(": Permission denied\n");
                            exit(0);
                        }
                    }
                    dup2(desOut, 1);
                    close(desOut);
                }
                /* closing child pipeFd */
                for(int i=0;i<2*pipeCnt;i++){
                    close(pipeFd[i]);
                }
                /* running build-in */
                if(!strcmp(mArgv[cmdHead],"pwd")){
                    char cwd[MAX_PATH];
                    if(getcwd(cwd, sizeof(cwd)) != NULL){
                        debugMsg("Info: pwd working.\n");
                        stdoutMsg(cwd);
                        stdoutMsg("\n");
                    }
                    else{
                        debugMsg("Error: pwd not working.\n");
                    }
                    exit(0);
                }
                
                /* running bash command */
                if(execvp(mArgv[cmdHead], mArgv+cmdHead) < 0){
                    debugMsg("Error: execvp not working.\n");
                    errMsg(mArgv[cmdHead]);
                    errMsg(": command not found\n");
                    // exit(0);
                }
                // a successful call to execvp doesn't have a return value, so code after this line will not be reached.
                exit(0);
            }
            else{ // parent process
                
            }
        }
        for(int i=0;i<2*pipeCnt;i++){
            close(pipeFd[i]); // parent closing pipes
        }
        if(isBackground == 0){
            for(int i=0;i<pipeCnt+1;i++){ // parent waiting for child process
                // wait(&childStatus);
                // char tmpMsg[108];
                // sprintf(tmpMsg,"Child process status: %d\n",childStatus);
                // debugMsg(tmpMsg);
                waitpid(lastPid[i],NULL,WUNTRACED);

                /* pid_t tmpPid;
                do{
                    tmpPid = wait(&childStatus);
                    if(tmpPid != pid){
                        char tmpMsg[1024];
                        sprintf(tmpMsg, "Error: The background process [%d] need to be terminated!\n", (int)tmpPid);
                        debugMsg(tmpMsg); // background process // errMsg
                    }
                } while (tmpPid != pid); */
            }
        }
        else if(isBackground == 1){ // The process is running in the background
            waitpid(bgJob[(bgCnt-1)*2],&childStatus,WNOHANG);
        }
        
        for(int i=0;i<mArgc;i++) if(mArgv[i]!=NULL) free(mArgv[i]);
        free(mArgv);
        promptExit();
    }
    for(int i=0;i<bgCnt;i++) free(bgCommand[i]);
    if(lastDir!=NULL) free(lastDir);
    return 0;
}
