#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define BLOCK_SIZE 32
#define n_inodes 4
#define comeco_ponteiros (2 + (sizeof(struct inodo) * n_inodes)) // 46

struct superblock {
	//uint8_t block_size;
	//uint8_t number_blocks;
	//uint8_t number_inodes;
	uint16_t prox_disponivel;
} super_bloco;

struct inodo { // tam inodo = 1+1+8+2+32+2 = 
	uint8_t status; // 1 byte
	uint8_t pasta; // 0 para sim, 1 para não
	char nome[8];
	uint16_t tam;
	char dados[32]; // se for um arquivo, poderá ter 4 pastas dentro
	// se for mais terá de alocar nos indiretos, tam representará a quantidade de pastas
 	uint16_t ponteiro_indireto;
};

void escreveBlocoDados(FILE * so);
void readInode(FILE * so, int inode_num);

int main(int argc, char const *argv[])  { 
	
	FILE * arq;

	arq = fopen("SO.bin", "wb");

	// tamanho de blocos extra, cada inode poderá ter um bloco indireto armazenado idealmente
	super_bloco.prox_disponivel = (uint16_t) 10; // ultimo item da pilha

	fwrite(&super_bloco, sizeof(super_bloco), 1, arq);
	
	for(int i = 0; i < n_inodes; i++) {
		escreveBlocoDados(arq);
	}

	// ponteiros de blocos livres para dados

	for(uint16_t i = 0; i < 12; i++) {
		fwrite(&(i), 2, 1, arq);	
	}

	uint8_t vazio = 0;

	// bloco de dados, deve ser o suficiente para guardar
	// blocos de 32 bytes suficiente p todos os blocos diretos
	// pelo menos

	for(uint16_t i = 0; i < 12*3; i++) {
		fwrite(&(vazio), 32, 1, arq);	
	}

	readInode(arq, 5);


	fclose(arq);
	return 0; 
}

void escreveBlocoDados(FILE * so) {
	struct inodo novo;
	novo.status = 0;
	novo.pasta = 0; // nao e pasta por enquanto
	strcpy(novo.nome, "null");
	novo.tam = 0;
	novo.ponteiro_indireto = 0;

	fwrite(&novo, sizeof(novo), 1, so);
}

void readInode(FILE * so, int inode_num) {
	struct inodo aux;
	int pos = 2+ (sizeof(aux)*inode_num);

	fseek(so, pos,SEEK_SET);

	fread(&aux, sizeof(aux), 1, so);

	printf("inodo n.%d: \nStatus: %d\nPasta: %d\n Nome: %s \nDados direto: %s",inode_num, aux.status,aux.pasta, aux.nome, aux.dados);
}

