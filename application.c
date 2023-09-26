#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

/*
*****NOTES TO IMPLEMENT (delete when TP is finished)*****
1) arreglar el FD_SET
*/

typedef struct slave
{
    int pidNum;
    int masterToSlave[2];
    int slaveToMaster[2];
}slave;


int main(int argc, char * argv[]){

    if(argc < 2){
        perror("No files or directory passed to analize");
        exit(EXIT_FAILURE);
    }

    int files = argc - 1; 
    int slaveAmmount = (int) log2((double) files);

    slave slaves[slaveAmmount];

    /*en caso de que pasen solo un arg log2(1) = 0 entonces
    necestio al menos un slave porque sino no funciona*/
    if(slaveAmmount< 1){
        slaveAmmount++;
    }


    fd_set readFromSlaves[slaveAmmount];

    for (int i = 0; i < 2;i++) {

        
        pipe(slaves[i].masterToSlave);
        /*Cierro el extremo de lectura pues leere del de escritura del slave*/
        close(slaves[i].masterToSlave[0]);

        pipe(slaves[i].slaveToMaster);
        /*Cierro el extremo de lectura pues leere del de escritura del master*/
        close(slaves[i].slaveToMaster[0]);

        /*Me guardo el extremo de escritura del slave en un set para utilizar luego con 
        el select*/
        //FD_SET((*slaves[i]).slaveToMaster[1],&readFromSlaves);
        

        /*Fork salio bien*/
        if((slaves[i].pidNum = fork()) == 0){
            char* vars = {"./slave",NULL};
            
            close(STDIN_FILENO);
            dup(slaves[i].slaveToMaster[0]);

            close(STDOUT_FILENO);
            dup(slaves[i].slaveToMaster[1]);

            /*Antes de irme con el exceve como el fork
            me duplica todos los pipes debo cerrar aquellos que no utilizare*/
            for (int w = 0; w < i+1; w++) {
                for (int j = 0; j < 2; j++) {
                    close(slaves[w].masterToSlave[j]);
                    close(slaves[w].slaveToMaster[j]);
                }
            }

            execve("./slave",vars,NULL);

            
            perror("execve");
            exit(EXIT_FAILURE);    

        }
    }




}