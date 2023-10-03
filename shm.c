#include "shm.h"
#include <stdio.h>

// Function that creates the shared memory
shm_ADT create_shm(size_t shm_size)
{
    shm_ADT new_shm = malloc(sizeof(struct shm_CDT));
    if (new_shm == NULL)
    {
        perror("Creating the shared memory failed: allocate memory for shm failed");
        return 0;
    }

    new_shm->size = shm_size;
    new_shm->w_idx = 0;
    new_shm->r_idx = 0;

    //Generate the unique name for the shm based on the PID
    int pid = getpid();
    char pid_str[16];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    new_shm->sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_MODE, 1);
    new_shm->shm_name = (char *)malloc(strlen(SHM_NAME) + strlen(pid_str)+ 1);
    if (new_shm->shm_name == NULL){
        perror("Creating the shared memory failed: allocate memory for name failed");
        free(new_shm);
        return 0;
    }
    strcpy(new_shm->shm_name, SHM_NAME);
    strcat(new_shm->shm_name, pid_str);

    //Create the write semaphore
    new_shm->sem = sem_open(new_shm->shm_name, O_CREAT | O_EXCL, SEM_MODE, 1);
    if (new_shm->sem == SEM_FAILED){
        perror("Creating the semaphore for write failed");
        free(new_shm->shm_name);
        free(new_shm);
        return NULL;
    }

    int shm_fd;
    if ((shm_fd = shm_open(new_shm->shm_name, O_CREAT | O_RDWR, 0666)) == -1)
    {
        perror("Creating the shared memory failed: shm_open failed");
        sem_close(new_shm->sem);
        sem_close(new_shm->sem_read);
        sem_unlink(new_shm->shm_name);
        sem_unlink(SEM_NAME);
        free(new_shm->shm_name);
        free(new_shm);
        return NULL;
    }

    if (ftruncate(shm_fd, shm_size) == -1)
    {
        perror("Creating the shared memory failed: ftruncate failed");
        close(shm_fd);
        sem_close(new_shm->sem);
        sem_close(new_shm->sem_read);
        sem_unlink(new_shm->shm_name);
        sem_unlink(SEM_NAME);
        free(new_shm->shm_name);
        free(new_shm);
        return NULL;
    }

    new_shm->shm_ptr = (char *)mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (new_shm->shm_ptr == MAP_FAILED)
    {
        perror("Creating the shared memory failed: mmap failed");
        close(shm_fd);
        sem_close(new_shm->sem);
        sem_close(new_shm->sem_read);
        sem_unlink(new_shm->shm_name);
        sem_unlink(SEM_NAME);
        shm_unlink(new_shm->shm_name);
        free(new_shm->shm_name);
        free(new_shm);
        return NULL;
    }

    close(shm_fd);
    return new_shm;
}

// Write to shared memory
void write_shm(shm_ADT shm, char *buffer, size_t size)
{
    // takes the write sem
    if (sem_wait(shm->sem) == -1)
    {
        perror("Failed to lock semaphore for write");
        return;
    }

    // write in the shm
    if (size <= shm->size - shm->w_idx)
    {
        
        strncpy(shm->shm_ptr + shm->w_idx, buffer, size);
        shm->w_idx += size;
    }
    else
    {
       
        int remaining_space = shm->size - shm->w_idx;
        strncpy(shm->shm_ptr + shm->w_idx, buffer, remaining_space);
        shm->w_idx += remaining_space;
    }

  
    if (sem_post(shm->sem) == -1)
    {
        perror("Failed to unlock semaphore for write");
        return;
    }
}
int connect_shm(shm_ADT shared_memory, char *shared_memory_name, int file_qty)
{
    // Asign the name
    shared_memory->shm_name = shared_memory_name;

    // open the shm
    int shm_fd = shm_open(shared_memory_name, O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == ERROR)
    {
        perror("Opening existing shared memory failed: shm_open failed");
        return 0;
    }

    // Maps the memory
    shared_memory->shm_ptr = mmap(NULL, file_qty * FILE_SIZE_SHM, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory->shm_ptr == MAP_FAILED)
    {
        perror("Opening existing shared memory failed: mmap failed");
        return 0;
    }
    close(shm_fd);

    // open semaphores
    shared_memory->sem = sem_open(shared_memory->shm_name, 0);
    if (shared_memory->sem == SEM_FAILED)
    {
        perror("Opening existing shared memory failed: semaphore error");
        return 0;
    }
    shared_memory->sem_read = sem_open(SEM_NAME, 0);
    if (shared_memory->sem_read == SEM_FAILED)
    {
        perror("Failed opening existing read semaphore");
        return 0;
    }

    
    shared_memory->r_idx = 0;
    shared_memory->size = file_qty * FILE_SIZE_SHM;

    return 1;
}

// Reads the shared memory. BUG NEEDED FIXING
int read_shm(shm_ADT shm, char *buff)
{
    // printf("i am in the read shm\n");
    // When the whole shm was read, returns 0
    // Wait for the semaphore
    if (sem_wait(shm->sem) == -1)
    {
        perror("Failed to lock semaphore");
        return 0;
    }
    // if (shm->size == shm->r_idx)
    //     return 0;
    int shm_idx = shm->r_idx;

    // // Special semaphores for read to avoid race condition and data overlaps
    // sem_wait(shm->sem_read);
    // sem_wait(shm->sem);

    // // printf("i am  waiting in the read shm\n");
    // int shm_idx = shm->r_idx;

    // Read the shm
    int i = 0;
    for (; i < FILE_SIZE_SHM; i++)
    {
        buff[i] = shm->shm_ptr[shm_idx + i];
    }
    shm->r_idx += FILE_SIZE_SHM;
    buff[i] = '\0';

    // Enable the write process or read process
    if (sem_post(shm->sem) == -1)
    {
        shm->r_idx -= FILE_SIZE_SHM;
        perror("Failed in read function");
        // sem_close(shm->sem_read);
        // sem_unlink(SEM_NAME);
        // delete_shm(shm);
        return 0;
    }
    return 1;
}
void close_shm(shm_ADT shm)
{
    // Liberar recursos y cerrar la memoria compartida
    munmap(shm->shm_ptr, shm->size);
    sem_close(shm->sem);
    sem_close(shm->sem_read);
    sem_unlink(shm->shm_name);
    sem_unlink(SEM_NAME);
    shm_unlink(shm->shm_name);
    free(shm->shm_name);
    free(shm);
}


// Deletes the shared memory
void delete_shm(shm_ADT shm)
{
    munmap(shm->shm_ptr, shm->size);
    shm_unlink(shm->shm_name);
    free(shm);
}

// Deletes semaphores
void delete_semaphores(shm_ADT shm)
{
    sem_close(shm->sem);
    sem_unlink(SEM_NAME);
    sem_close(shm->sem_read);
    sem_unlink(SEM_NAME);
}
