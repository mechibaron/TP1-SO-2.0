// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/select.h>
#include "shmManager.h"
#include "semManagement.h"
#include <stdio.h>

#define NUM_SLAVES 5
#define FILES_PER_SLAVE 2

typedef struct {
    int pidNum;
    int toSlavePipe[2];
    int fromSlavePipe[2];
} SlaveInfo;


// Función para distribuir archivos a los esclavos
void distributeFilesToSlaves(SlaveInfo slaves[], char *argv[], int numFiles, int numSlaves, int initialFilesPerSlave) {
    static int filesSend = 0;
    for (int i = 0; i < numSlaves; i++) {
        for (int j = 0; j < initialFilesPerSlave; j++) {
            if (filesSend >= numFiles) {
                return; // Todos los archivos han sido distribuidos
            }

            write(slaves[i].toSlavePipe[1], argv[filesSend + 1], strlen(argv[filesSend + 1]));
            write(slaves[i].toSlavePipe[1], "\n", 1);
            filesSend++;
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int numFiles = argc - 1;
    int numSlaves = NUM_SLAVES;
    int initialFilesPerSlave = FILES_PER_SLAVE;

    int totalFilesToDistribute = numFiles - 1;

    if (numSlaves * initialFilesPerSlave > totalFilesToDistribute) {
        initialFilesPerSlave = totalFilesToDistribute / numSlaves;
    }
    int filesLeftToDistribute = totalFilesToDistribute - (numSlaves * initialFilesPerSlave);

    SlaveInfo slaves[NUM_SLAVES];
    char resultBuffer[1024];

    fd_set readFromSlaves;
    FD_ZERO(&readFromSlaves);

    int maxFd = 0;

    for (int i = 0; i < numSlaves; i++) {
        pipe(slaves[i].toSlavePipe);
        pipe(slaves[i].fromSlavePipe);

        if ((slaves[i].pidNum = fork()) == 0) {
            // Código para el proceso hijo (slave)
            close(slaves[i].toSlavePipe[1]);
            close(slaves[i].fromSlavePipe[0]);

            // Redirecciona las entradas/salidas estándar según sea necesario
            dup2(slaves[i].toSlavePipe[0], STDIN_FILENO);
            dup2(slaves[i].fromSlavePipe[1], STDOUT_FILENO);

            execve("./slave", argv, NULL);

            perror("execve");
            exit(EXIT_FAILURE);
        } else if (slaves[i].pidNum == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else {
            // Código para el proceso padre
            close(slaves[i].fromSlavePipe[1]);
            close(slaves[i].toSlavePipe[0]);

            FD_SET(slaves[i].fromSlavePipe[0], &readFromSlaves);
            if (slaves[i].fromSlavePipe[0] > maxFd) {
                maxFd = slaves[i].fromSlavePipe[0];
            }
        }
    }

    FILE *resultFile = fopen("resultado.txt", "w");
    if (resultFile == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    // for (int i = 1; i < argc; i++) {
    //     // printf("filePaht: %s\n",argv[i]);
    //     write(slaves[i - 1].toSlavePipe[1], argv[i], strlen(argv[i]));
    //     write(slaves[i - 1].toSlavePipe[1], "\n", 1); 
    // }
    distributeFilesToSlaves(slaves, argv, numFiles, numSlaves, initialFilesPerSlave);

    int filesRead = 0; 

    while (filesRead < numFiles) {
        fd_set readSet = readFromSlaves;
        int selectResult = select(maxFd + 1, &readSet, NULL, NULL, NULL);

        if (selectResult == -1) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < numSlaves; i++) {
            if (FD_ISSET(slaves[i].fromSlavePipe[0], &readSet)) {
                int bytesRead = read(slaves[i].fromSlavePipe[0], resultBuffer, sizeof(resultBuffer));
                if (resultBuffer[bytesRead - 1] == '\n') {
                    resultBuffer[bytesRead - 1] = '\0';
                }
                if (bytesRead > 0) {
                    printf("resultBuffer: %s\n", resultBuffer);
                    fprintf(resultFile, "%s\n", resultBuffer);
                    filesRead++;
                    if (filesLeftToDistribute > 0) {
                        distributeFilesToSlaves(slaves, argv, numFiles, numSlaves, initialFilesPerSlave);
                        // Distribuir un archivo adicional al esclavo actual
                        // write(slaves[i].toSlavePipe[1], argv[filesRead], strlen(argv[filesRead]));
                        // write(slaves[i].toSlavePipe[1], "\n", 1);
                        filesLeftToDistribute--;
                    }
                }
            }
        }

    }

    fclose(resultFile);

    return 0;
}
