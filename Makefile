CC = gcc
FLAGS = -O2 -pthread

filesys:
	$(CC) server.c $(FLAGS)
	./a.out