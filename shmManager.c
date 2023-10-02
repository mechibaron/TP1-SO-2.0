#include "shmManager.h"

int create_shm(size_t shm_size){

    int shm_fd;

    /*Creo la memoria compartida*/
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == ERROR){
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    
    /*Ahora que la tenemos definida, asigno un tamaño*/
    if((ftruncate(shm_fd, shm_size) == ERROR)){
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    return shm_fd;

}

/*Recibe como parametro fd asociado a la memoria compartida previamente asignada con create_shm y su correspondiente tamaño.
Retorna el puntero al inicio del espacio de direcciones otorgado para cubrir la memoria compartida*/
char * mmap_shm(int shm_fd, size_t shm_size){

     char * shm_base_ptr;

     /*Al pasar NULL como primer argumento le damos la libertad al SO para que eliga donde mapear.
     En el 3er parametro asignamos permisos de escritura, mientras que en el 4to aclaramos que la memoria sera compartida entre procesos,
     El ultimo argumento es el offset. En este caso se lee/escribe desde la posicion cero del conjunto de paginas*/
     shm_base_ptr = mmap(NULL, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, INITIAL_MEM_ADDRESS);
     if (shm_base_ptr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
     }

    return shm_base_ptr;
}

/*Cierra la memoria (no la elimina) a traves de fd, el puntero a su base y su tamaño*/
void close_shm(int shm_fd, char * shm_ptr, size_t shm_size){

    if(close(shm_fd) == ERROR){
        perror("close");
        exit(EXIT_FAILURE);
    }

    if(munmap(shm_ptr, shm_size) == ERROR){
        perror("munmap");
        exit(EXIT_FAILURE);
    }

}

/*Eliminamos la memoria*/
void unlink_shm(){
    if(shm_unlink(SHM_NAME) == ERROR){
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }
}

/*Utilizado en view para tener acesso a la memoria creada en application*/
char * ropen_shm(int * shm_fd, size_t shm_size){
    *shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDONLY, 00400);
    if (*shm_fd == ERROR)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    char * shm_base_ptr = mmap(NULL, shm_size, PROT_READ, MAP_SHARED, *shm_fd, INITIAL_MEM_ADDRESS);
    if (shm_base_ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    return shm_base_ptr;
}
