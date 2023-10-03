#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "shm.h"

#define MAX_BUFF_LENGTH 1024
#define MAX_SHM_NAME_LENGTH 1024
#define MD5_SIZE 32

int main(int argc, char *argv[]) {
    char shared_memory_name[MAX_SHM_NAME_LENGTH];
    int number_of_files = 0;
    int filesPrinted = 0;

    // Si el número de archivos se proporciona como argumento
    if (argc == 2) {
        // Copiar el nombre de la memoria compartida
        strncpy(shared_memory_name, argv[1], sizeof(shared_memory_name));
    } else if (argc == 1) {
        // Leer el nombre de la memoria compartida y el número de archivos desde STDIN
        char input[MAX_BUFF_LENGTH] = {0};
        fgets(input, sizeof(input), stdin);
        printf("%s",input);

        // Parsear la entrada para obtener el nombre de la memoria compartida y el número de archivos
        if (sscanf(input, "%s %d", shared_memory_name, &number_of_files) != 2) {
            perror("Entrada incorrecta. Debe proporcionar el nombre de la memoria compartida y el número de archivos.");
            exit(1);
        }
    } else {
        perror("Error. Debe pasar el número de archivos procesados\n");
        exit(1);
    }

    shm_ADT shared_memory = malloc(sizeof(struct shm_CDT));

    // Conectar a la memoria compartida
    if (connect_shm(shared_memory, shared_memory_name, number_of_files)) {
        char buffer[number_of_files * FILE_SIZE_SHM + 1];

        printf("files printed: %d  & numeber of files: %d\n",filesPrinted, number_of_files); 
        read_shm(shared_memory, buffer);

        while(read_shm(shared_memory, buffer) && (filesPrinted != number_of_files)) {
            printf("hoaldska\n");
            printf("%s\n", buffer);
            filesPrinted += 1;
        }
    }

    delete_semaphores(shared_memory);
    delete_shm(shared_memory);

    return 0;
}
