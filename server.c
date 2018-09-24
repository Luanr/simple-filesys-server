#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#define PORT 8080

#define THREADS_N 10

int CONNECTIONS = 0;

pthread_t threads[THREADS_N];

void * client_connection(void * args) {
    char buffer[1024] = {0};
	int valread;
	int connection = (int *) args;
    char * server_response = "Server Message!";
	char exit[] = "exit";

	while(1) {
		valread = read(connection , buffer, 1024); 
		printf("%s\n",buffer ); 
		send(connection , server_response , strlen(server_response) , 0 ); 
		printf("Server response \n");
		
		/*
		if(strcmp(buffer, exit) == 1) {
			CONNECTIONS--;
			break;
		}*/
	}
}

void socket_setup() {
    int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 

	/* Trying to create socket */
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	/* Attaching  */
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	

	for(int i = 0; i < THREADS_N; i++) {

		if (listen(server_fd, 3) < 0) { 
			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 

		pthread_create(&threads[i], NULL, client_connection, (void *) accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen));
	}
	
}

int main(int argc, char const *argv[])  { 
	socket_setup();
	return 0; 
} 
