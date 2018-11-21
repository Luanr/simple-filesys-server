#include "server.h"

int main(int argc, char const *argv[])  { 
	pthread_mutex_init(&lock, NULL);
	socket_setup();
	pthread_mutex_destroy(&lock);
	return 0; 
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
	
	// Binds the socket to the adress and port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) { 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	

	while(1) {
		for(int i = 0; i < THREADS_N; i++) {
			if (listen(server_fd, 3) < 0) { 
				perror("listen"); 
				exit(EXIT_FAILURE); 
			} 

			pthread_create(&threads[i], NULL, client_connection, (void *) accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen));
		}
	}
}

void * client_connection(void * args) {
    
    char path[1024] = {0};
	char * token;
	const char * delimiter = " \n";
	char * output = malloc(sizeof(char) * 500);

	while(1) {
		char buffer[1024] = {0};
		int valread;
		int connection = (int *) args;
		char server_response[1024] = {0};
		char aux[500] = {0};
		char command[500] = {0};
		char command_aux[500] = {0};
		char aux_token[20] = {0};

		
		valread = read(connection , buffer, 1024);
		
		if(valread == 0) {
			break;
		} 

		//printf("%s\n", buffer);
		strcat(command, buffer);
		strcpy(command_aux,  "./filesys ");
		printf("LOCOH : %s\n", command_aux);
		strcpy(aux, command);
		token = strtok(aux, delimiter);

		if(!strcmp(token, "mkdir") || !strcmp(token, "rmdir") || !strcmp(token, "rm") || !strcmp(token, "cd") || !strcmp(token, "ls") || !strcmp(token, "touch") || !strcmp(token, "cat") || !strcmp(token, "echo") || !strcmp(token, "formata")) {
			pthread_mutex_lock(&lock);

			FILE * fp;

			int str_len = 1000;
			char str[str_len];

			strcat(command_aux, command);

			printf("COMO NOIS TA: %s\n", command_aux);

			fp = popen(command_aux, "r");

			getcwd(path, sizeof(path));	        

	        while(fgets(str, str_len, fp) != NULL) {
	            strcat(server_response, str);
	        }

			fflush(stdout);
			fflush(stdin);
			fflush(fp);
			
			pclose(fp);

	        pthread_mutex_unlock(&lock);

	    } else if (!strcmp(token, "help")) {
	        strcpy(server_response, "Comandos disponíveis: mkdir, rm, cd, ls, touch, cat, echo \n");
	    } else {
	        strcpy(server_response, "Comando não encontrado, digite help para saber mais \n");
	    }

		send(connection , server_response , strlen(server_response) , 0 );
	}
}
