#include "allIncludes.h"
#include <stdio.h>
#include <string.h>


int calculateMd5(char *filePath, char *ansBuffer) {
    int pid = getpid();
    char mypid[10];
    sprintf(mypid, "%d", pid);

    char command[300];
    snprintf(command, sizeof(command), "md5sum %s", filePath);
    // printf("command: %s\n", command);

    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Md5 ERROR!");
        return 1;
    }

    char buffer[560];
    size_t bytesRead = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Append the MD5 and filename to the ansBuffer
        strcat(ansBuffer, mypid);
        strcat(ansBuffer, "-");
        strcat(ansBuffer, buffer);

        // Reset the buffer
        memset(buffer, 0, sizeof(buffer));

        bytesRead++;
    }

    // Close the stream and check the exit status
    int status = pclose(fp);
    if (status == -1) {
        perror("pclose");
        return 1;
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        // The command exited with an error status
        fprintf(stderr, "Command failed with exit status %d\n", WEXITSTATUS(status));
        return 1;
    }

    return 0;
}


int main(){
    // printf("Buenas estoy en slave\n");
    ssize_t nbytes;
    char filePath[256];
    char ansBuffer[256];
    while((nbytes = read(STDIN_FILENO, filePath, sizeof(filePath))) > 0){
        strcpy(ansBuffer,"");
        if (filePath[nbytes - 1] == '\n') {
            filePath[nbytes - 1] = '\0';
        }
        // printf("filepaht: %s\n",filePath);
        if(calculateMd5(filePath, ansBuffer) == 0){
            size_t output_dim = write(STDOUT_FILENO, ansBuffer, strlen(ansBuffer));
            if(output_dim == -1){
                perror("write");
                exit(EXIT_FAILURE);
            }
        } else {
            // fprintf(stderr, "Error calculating MD5\n");
            // return 1;
        }
    }
    return 0;
}
