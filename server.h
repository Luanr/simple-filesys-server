#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
 
#define PORT 8080
 
#define THREADS_N 200
 
int CONNECTIONS = 0;
pthread_t threads[THREADS_N];
pthread_mutex_t lock;

void socket_setup();
void * client_connection(void * args);
char * task(char * command);