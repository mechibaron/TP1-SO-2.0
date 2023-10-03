#ifndef SHM_H
#define SHM_H

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define SEM_NAME            "name_sem"
#define SEM_INIT_VALUE      1
#define SHM_READ_SEM       "shm_sem_read"
#define SEM_READ_INIT_VALUE 0
#define ERROR          -1 
#define SEM_MODE 0666
#define SHM_NAME "shm_name"


#define PAGE_SIZE 4096
#define FILE_SIZE_SHM 64

struct shm_CDT{
    char *shm_name;
    char *shm_ptr;

    int size;
    int w_idx;
    int r_idx;

    sem_t *sem;
    sem_t *sem_read;
}; 

typedef struct shm_CDT *shm_ADT;

shm_ADT create_shm(size_t shm_size);
void write_shm(shm_ADT shm, char *buffer, size_t size);
int read_shm(shm_ADT shm, char *buff);
void close_shm(shm_ADT shm);
void delete_shm(shm_ADT shm);
void delete_semaphores(shm_ADT shm);
int connect_shm(shm_ADT shared_memory, char *shared_memory_name, int file_qty);

#endif // SHM_H
