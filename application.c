#include "allIncludes.h"

#define MAX_SLAVES 10

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
    int slaveAmmount;

    size_t shm_size = PAGE_SIZE*cant_files;
    char first_run[MAX_SLAVES];
    memset(first_run, 0, MAX_SLAVES);

    //Calculamos cantidad de esclavos y recalculamos tamano de memoria segun si hay carga incial de 2 paths
    if(files > MAX_SLAVES){
        slaveAmmount = MAX_SLAVES;
        if (files > MAX_SLAVES*2){
            memset(first_run, 1, MAX_SLAVES);
            shm_size = PAGE_SIZE * (cant_files - MAX_SLAVES);
        }
        else{
            for (int i = 0; i < files - MAX_SLAVES; i++){
                first_run[i] = 1;
                shm_size -= PAGE_SIZE;
            }
        }
        
    }
    else {
        slaveAmmount = cant_files;
    }
    
    slave slaves[slaveAmmount];

    for (int i = 0; i < slaveAmmount;i++) {

        
        pipe(slaves[i].masterToSlave);
        /*Cierro el extremo de lectura pues leere del de escritura del slave*/
        close(slaves[i].masterToSlave[0]);

        pipe(slaves[i].slaveToMaster);
        /*Cierro el extremo de lectura pues leere del de escritura del master*/
        close(slaves[i].slaveToMaster[0]);
        
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


    fd_set readFromSlaves;

    FD_ZERO(readFromSlaves);

    for(int i = 0;i < slaveAmmount;i++){
        FD_SET(slaves[i].slaveToMaster[1],&readFromSlaves);
    }

}