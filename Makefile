CC = gcc
CFLAGS = -Wall
all: application slave view

application: application.c shm.h
	$(CC) $(CFLAGS) -o application application.c shm.c

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c

view: view.c shm.c
	$(CC) $(CFLAGS) -o view view.c shm.c

clean:
	rm -f application slave view resultado.txt
