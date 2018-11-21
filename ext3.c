#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define BLOCK_SIZE 32
#define INODE_SIZE sizeof(INODE)
#define N_INODES 10
#define N_BLOCKS 20

#define INODE_REGION N_BLOCKS
#define BLOCKS_REGION N_BLOCKS + (N_INODES * INODE_SIZE)

#define ANSI_BOLDBLUE    "\033[1m\033[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"


void writeInodeBlock(FILE * so);
int findFreeBlock(FILE * so);
int findFreeInode(FILE * so);
void handleCommand(int n_command, char* command[]);

struct _inode { // tam inodo = 1 + 1 + 8 + 2 + 1 + 1 = 14
	uint8_t status; // 1 byte: 0 livre, 1 ocupado
	uint8_t pasta; // 0 para sim, 1 para não
	char nome[8]; // nome do arquivo
	uint16_t tam; // tam do arquivo
	uint8_t data_block; // ponteiro para bloco de dados
	uint8_t pointers_block; // ponteiro indireto para bloco de ponteiros
};
typedef struct _inode INODE;

struct _data_block {
	unsigned char data[BLOCK_SIZE];
};
typedef struct _data_block DATA_BLOCK;

unsigned char block_manager[N_BLOCKS] = { 0 };

FILE * arq;
int current_dir = 0; 	// dir atual inicial no inodo 0 -> pasta home		

int main(int argc, char* argv[]) {
	
	
	arq = fopen("SO.bin", "rb+");
	formata(arq);
	if(arq == NULL) {
		formata(arq);
	}

	//handleCommand(argc, argv);	

	createFile(arq, current_dir, "teste.c", "ABCDEFGHIJKLMNOPQRSTUVXZWYABCDEFGHIJKL");

	rm(arq, current_dir, "teste.c");
	ls(arq, current_dir);

	//printDataBlock(arq, 0, 0);
	//printDataBlock(arq, 1, 0);
	printDataBlock(arq, 2, 1);
	//printDataBlock(arq, 3, 0);

	/*
	createFolder(arq, current_dir, "home");

	ls(arq, current_dir);

	createFolder(arq, current_dir, "tijucas");
	createFolder(arq, current_dir, "zimba");

	ls(arq, current_dir);*/

	//createFile(arq, 0, "teste.c", "ABCDEFGHIJKLMNOPQRSTUVXZWYABCDEFGHIJKL");

	//createFile(arq, 0, "tijucas", "ABCDEFGHIJKLMNOPQRSTUVXZWYABCDEFGKOP");
	//createFile(arq, 0, "alabeto", "ABCDEFGHIJKLMNOPQRSTUVXZWYABCDEFG");

	//cat(arq, 0, "tijucas");

	//printDataBlock(arq, 3, 1);

	//readInode(arq, 2);
}

void rm(FILE * so, int inode_dest, char * name) {
	INODE file;
	int inodo_rm = procuraInodo(arq, inode_dest, name);
	if(inodo_rm == -1) {
		printf("\nNão foi possível deletar o arquivo, dele não existe!");
		return;
	}
	fseekInode(arq, inodo_rm);
	fread(&file, sizeof(file), 1, so);

	// se é arquivo
	if(file.pasta == 0) {
		fseekBlock(so, file.data_block);
		fwrite(&block_manager, sizeof(block_manager), 1, so);

		// desaloca os blocos indiretos tb
		if(file.tam > 32) {
			char block_aux[BLOCK_SIZE] = {0};
			fseekBlock(so, file.pointers_block);
			fread(&block_aux, sizeof(block_aux), 1, so);

			for(int i = 0; i < BLOCK_SIZE; i++) {
				fseekBlock(so, block_aux[i]);
				fwrite(&block_manager, sizeof(block_manager), 1, so);
			}
			
		}

	} else {
		return;
	}
	
	file.tam = 0;
	file.status = 0;
	strcpy(file.nome, "null");
	file.data_block = 0;
	file.pointers_block = 0;

	//insere alterações no inodo
	fseekInode(so, inode_dest);
	fwrite(&file, sizeof(file), 1, so);
}

int cd(FILE * so, int inode_dest, char * name) {
	int new_dir = procuraInodo(arq, inode_dest, name);
	if(new_dir != -1) {
		current_dir = new_dir;
		return 1;
	} else {
		return -1;
	}
}

void ls(FILE * so, int inode_dest) {
	INODE file, aux;

	char folder_inodes[BLOCK_SIZE];

	fseekInode(so, inode_dest);
	fread(&file, sizeof(file), 1, so);
	
	
	fseekBlock(so, file.data_block);
	fread(&folder_inodes, sizeof(folder_inodes), 1, so);

	int read = 0;

	// le os dados
	while(read < file.tam) {
		if(folder_inodes[read] != 0) {
			fseekInode(so, folder_inodes[read]);
			simpleReadInode(so, folder_inodes[read]);
		}
		read++;
	}
}

void simpleReadInode(FILE * so, int inode_num) {
	INODE aux;

	fseekInode(so, inode_num);


	fread(&aux, sizeof(aux), 1, so);

	if(aux.pasta) { // se é uma pasta, pintar de azul
		//printf(ANSI_BOLDBLUE "\n%s" ANSI_COLOR_RESET, aux.nome);
		printf("\n*%s", aux.nome);
	} else {
		printf("\n%s", aux.nome);
	}
}

int procuraInodo(FILE * so, int inode_dest, char * file_name) {
	INODE file, aux;

	char folder_inodes[BLOCK_SIZE];

	fseekInode(so, inode_dest);
	fread(&file, sizeof(file), 1, so);
	
	fseekBlock(so, file.data_block);
	fread(&folder_inodes, sizeof(folder_inodes), 1, so);

	int read = 0;

	// pelo menos vai ler todos os arquivos da pasta
	while(read < file.tam) {
		if(folder_inodes[read] != 0) {
			fseekInode(so, folder_inodes[read]);
			fread(&aux, sizeof(aux), 1, so);

			if(strcmp(aux.nome, file_name) == 0) {
				return folder_inodes[read];
			}
		}

		read++;
	}
	return -1;
}


void cat(FILE * so, int inode_num, char * file_name) {
	INODE file;
	int inodo = procuraInodo(so, inode_num, file_name);

	if(inodo == -1) {
		printf("Arquivo Inexistente!");
		return;
	}
	fseekInode(so, inodo);
	fread(&file, sizeof(file), 1, so);

	// bytes lidos
	int read = 0;

	// blocos diretos primeiro
	printDataBlock(so, file.data_block, file.pasta);

	// depois blocs indiretos

	read += BLOCK_SIZE;
	int indirect_count =0;

	// pega endereço dos blocos indiretos
	char indirect_data[BLOCK_SIZE];

	// vai ate onde esta o bloco de ponteiros
	fseekBlock(so, file.pointers_block);
	fread(&indirect_data, sizeof(indirect_data), 1, so);
	

	while(read < file.tam) {
		printDataBlock(so, indirect_data[indirect_count], file.pasta);
		read += BLOCK_SIZE;
		indirect_count++;
	}
}

void readInode(FILE * so, int inode_num) {
	INODE aux;

	fseekInode(so, inode_num);

	fread(&aux, sizeof(aux), 1, so);

	printf("\n Inodo n.%d:", inode_num);
	printf("\n Nome: %s:", aux.nome);
	printf("\n Status: %d", aux.status);
	printf("\n Pasta: %d", aux.pasta);
	printf("\n Tamanho: %d", aux.tam);
	printf("\n Ponteiro bloco direto: %d", aux.data_block);
	printf("\n Dados: ");
	printDataBlock(so, aux.data_block, 0);


	//printf("\n Inodo n.%d: \nStatus: %d\nPasta: %d\n Nome: %s \nDados direto: %s \n",inode_num, aux.status,aux.pasta, aux.nome);
}

int findFreeBlock(FILE * so) {
	unsigned char block_aux[N_BLOCKS];
	
	fseek(so, 0, SEEK_SET);
	fread(&block_aux, sizeof(block_aux), 1, so);

	for(int i = 0; i < N_BLOCKS; i++) {
		if(block_aux[i] == 0) {
			block_aux[i] = 1;
			fseek(so, 0, SEEK_SET);
			fwrite(&block_aux, sizeof(block_aux), 1, so);
			return i;
		}
	}
	return -1;
}

int findFreeInode(FILE * so) {
	INODE inode_aux;
    fseek(so, INODE_REGION, SEEK_SET);
	
    for(int i = 0; i < N_BLOCKS; i++) {
		fread(&inode_aux, sizeof(inode_aux), 1, so);
		if(inode_aux.status == 0) return i;
	}
	return -1;
}

void writeInodeBlock(FILE * so) {
	INODE novo;
	novo.status = 0;
	novo.pasta = 0; // nao e pasta por enquanto
	strcpy(novo.nome, "null");
	novo.tam = 0;
	
	fwrite(&novo, sizeof(novo), 1, so);
}

void createFile(FILE * so, int inode_dest, char * name, char * dados) {

	INODE novo;
	INODE pai;
	int inode_num; // recebe o numero do inodo a ser alocado para a pasta


	// encontrou numero do inode a ser alocado
	inode_num = findFreeInode(so);
	
	strcpy(novo.nome, name);	// nome do inodo
	novo.pasta = 0;				// é uma pasta
	novo.status = 1;			// ocupado
	novo.tam = strlen(dados);				// tamanho começando em 0, pasta vazia
	novo.data_block = findFreeBlock(so);

	

	int n_inserted = 0;
	int aux_size = novo.tam;
	char dados_direto[BLOCK_SIZE];
	strncpy(dados_direto, dados, BLOCK_SIZE);

	// escreve até 32 bytes no dado direto
	writeDataBlock(so, novo.data_block, dados_direto);

	aux_size -= BLOCK_SIZE;
	n_inserted = BLOCK_SIZE;

	// se excedeu tamanho do direto, aloca indireto
	if(aux_size > 0) {
		novo.pointers_block = findFreeBlock(so);	
	}

	// vai até o inode filho e escreve ele
	fseekInode(so, inode_num);
	fwrite(&novo, sizeof(novo), 1, so);

	// enquanto tiver espaço para ser preenchido
	while(aux_size > 0) {

		//aloca novo bloco indireto
		int new_datablock = findFreeBlock(so);

		// adiciona ponteiro indireto
		append_pointerF(so, new_datablock,novo.pointers_block);
		
		char indirect_data[BLOCK_SIZE];

		// passa a quantidade de dados para um uma string que será escrita no bloco
		strncpy(indirect_data, dados + (n_inserted), BLOCK_SIZE);
		n_inserted += BLOCK_SIZE;


		writeDataBlock(so, new_datablock, indirect_data);

		aux_size -= BLOCK_SIZE;
	}


	// insere id do inodo criado dentro do inodo pai
	// insere dado dentro
	fseekInode(so, inode_dest);

	fread(&pai, sizeof(pai), 1, so);

	// se tam > 32, fazer isso com bloco indireto

	// insere o id do inode dentro do bloco de ponteiros do pai
	append_pointer(so, pai.data_block, inode_num);
	pai.tam++;

	// faz as alterações no pai
	fseekInode(so, inode_dest);
	fwrite(&pai, sizeof(pai), 1, so);

}

/* 
Funcoes do sistema
*/

void formata(FILE * so) {
	fclose(so);
	so = fopen("SO.bin", "wb+");

  	fwrite(&block_manager, sizeof(block_manager), 1, so);

    for(int i = 0; i < N_INODES; i++) {
        writeInodeBlock(so);
    }

	for(int i = 0; i < N_BLOCKS; i++) {
		fwrite(&block_manager, sizeof(block_manager), 1, so);
	}

	// cria pasta raiz
	createFolder(so, 0, "/");
}

void createFolder(FILE * so, int inode_dest, char * name) {
	INODE novo;
	INODE pai;
	int inode_num; // recebe o numero do inodo a ser alocado para a pasta


	// encontrou numero do inode a ser alocado
	inode_num = findFreeInode(so);
	
	strcpy(novo.nome, name);	// nome do inodo
	novo.pasta = 1;				// é uma pasta
	novo.status = 1;			// ocupado
	novo.tam = 0;				// tamanho começando em 0, pasta vazia
	novo.data_block = findFreeBlock(so);

	// vai até o inode filho e escreve ele
	fseekInode(so, inode_num);
	fwrite(&novo, sizeof(novo), 1, so);

	// insere id do inodo criado dentro do inodo pai
	if(inode_num != 0) { // caso não seja o diretório raiz "/"
		
		// insere dado dentro
		fseekInode(so, inode_dest);

		fread(&pai, sizeof(pai), 1, so);

		// se tam > 32, fazer isso com bloco indireto

		// insere o id do inode dentro do bloco de ponteiros do pai
		append_pointer(so, pai.data_block, inode_num);
		pai.tam++;

		// faz as alterações no pai
		fseekInode(so, inode_dest);
		fwrite(&pai, sizeof(pai), 1, so);
	}
}
// recebe o ponteiro e o id do bloco
// usado para files
void append_pointerF(FILE * so, int id_block, int pointer) {
	DATA_BLOCK data_block;
	fseekBlock(so, pointer);
	fread(&data_block, sizeof(data_block), 1, so);

	for(int i = 0; i < BLOCK_SIZE; i++) {
		if(data_block.data[i] == 0) {
			data_block.data[i] = id_block;
			break;
		}
	}

	fseekBlock(so, pointer);
	fwrite(&data_block, sizeof(data_block), 1, so);
}

/* incrementa ponteiro dentro de bloco de ponteiros
	é utilizado para dados de pastas e blocos indiretos, 
	utilizar append_pointerF para files
*/
void append_pointer(FILE * so, int id_block, int pointer) {
	DATA_BLOCK data_block;
	fseekBlock(so, id_block);
	fread(&data_block, sizeof(data_block), 1, so);

	for(int i = 0; i < BLOCK_SIZE; i++) {
		if(data_block.data[i] == 0) {
			data_block.data[i] = pointer;
			break;
		}
	}

	fseekBlock(so, id_block);
	fwrite(&data_block, sizeof(data_block), 1, so);
}


void printDataBlock(FILE * so, int datablock_pointer, int isPointer) {
	DATA_BLOCK data_block;
	fseekBlock(so, datablock_pointer);
	fread(&data_block, sizeof(data_block), 1, so);

	for(int i = 0; i < BLOCK_SIZE; i++) {
		//if(data_block.data[i] != 0) {
			if(isPointer == 1) {
				printf("%d ", data_block.data[i]);
			} else {
				printf("%c", data_block.data[i]);
			}
		//}	
	}
}

/* escreve uma quantidade de dados dentro de um bloco de dados
*/
void writeDataBlock(FILE * so, int id_block, char * data) {
	DATA_BLOCK new_data_block;
	fseekBlock(so, id_block);
	strncpy(new_data_block.data, data, BLOCK_SIZE);

	fwrite(new_data_block.data, 1, sizeof(DATA_BLOCK), so);		
}

void fseekInode(FILE * so, int id_inode) {
	fseek(so, INODE_REGION + (id_inode*INODE_SIZE),SEEK_SET);
}

void fseekBlock(FILE * so, int id_block) {
	fseek(so, BLOCKS_REGION + (id_block*BLOCK_SIZE),SEEK_SET);
}

void handleCommand(int n_command, char * command[]) {
	if(!strcmp(command[1], "mkdir")) {
		printf("MKDIR %s\n", command[2]);
		createFolder(arq, current_dir, command[2]);
	} else if(!strcmp(command[1], "rm")) {
		printf("RM %s\n", command[2]);
		// MANAO N FEZ AINDA SAPORRA
	} else if(!strcmp(command[1], "cd")) {
		printf("CD %s\n", command[2]);

		if(cd(arq, current_dir, command[2]) == -1) {
			printf("ERRO, diretório não existe! \n");
			return;
		}

		if(!strcmp(command[3], "mkdir")) {
			printf("MKDIR %s\n", command[4]);
			createFolder(arq, current_dir, command[4]);
		} else if(!strcmp(command[3], "rm")) {
			printf("RM %s\n", command[2]);
			// MANAO N FEZ AINDA SAPORRA
		} else if(!strcmp(command[3], "ls")) {
			printf("LS\n");
			ls(arq, current_dir);
		} else if(!strcmp(command[3], "touch")) {
			printf("TOUCH %s\n", command[4]);
			createFile(arq, current_dir, command[2], "");
		} else if(!strcmp(command[1], "cat")) {
			printf("CAT %s\n", command[2]);
			cat(arq, current_dir, command[2]);
		} else if(!strcmp(command[1], "echo")) {
			printf("ECHO %s\n", command[2]);
			createFile(arq, current_dir, command[2], command[3]);
		}
		
	} else if(!strcmp(command[1], "ls")) {
		printf("LS\n");
		ls(arq, current_dir);
	} else if(!strcmp(command[1], "touch")) {
		printf("TOUCH %s\n", command[2]);
		createFile(arq, current_dir, command[2], "");
	} else if(!strcmp(command[1], "cat")) {
		printf("CAT %s\n", command[2]);
		cat(arq, current_dir, command[2]);
	} else if(!strcmp(command[1], "echo")) {
		printf("ECHO %s\n", command[2]);
		createFile(arq, current_dir, command[2], command[3]);
	} else if(!strcmp(command[1], "formata")) {
		formata(arq);
		printf("Disco formatado!\n");
	}
}
