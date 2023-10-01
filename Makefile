CC = gcc
CFLAGS = -Wall -lm
all: application slave 
application: application.c 
	$(CC) $(CFLAGS) -o application application.c  

slave: slave.c
	$(CC) $(CFLAGS) -o slave slave.c

clean:
	rm -f application slave  Md5Results.txt
