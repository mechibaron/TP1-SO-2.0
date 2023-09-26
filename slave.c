#include "allIncludes.h"

int calculateMd5(char *filePath,char * ansBuffer){

    int pid = getpid();
    char *mypid = malloc(6);   
    sprintf(mypid, "%d", pid);

    char *command = malloc(sizeof(char) * 300);
    strcpy(command,"md5sum ");
    strcpy(command,"\"");
    strcpy(command,filePath);
    strcpy(command,"\" 2>/dev/null");

    FILE *fp = popen(command,"r");
    if(fp == NULL){
        perror("Md5 ERROR!");
        return 1;
    }

    char buffer[560];

    fgets(buffer,sizeof(buffer),fp);

    strcpy(ansBuffer,mypid);
    strcat(ansBuffer,"-");
    strcat(ansBuffer,buffer);
    strcat(ansBuffer,"-");
    strcat(ansBuffer,filePath);
    
    pclose(fp);
    free(command);
    free(mypid);
    
    size_t output_dim = write(STD_OUT, output, strlen(output));
        if(output_dim == ERROR){
            perror("write");
            exit(EXIT_FAILURE);
        }
    
}

int main(){
    
}