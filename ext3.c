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

struct inodo { // tam inodo = 1+1+8+2+32+2 = 46 bytes
	uint8_t status; // 1 byte
	uint8_t pasta; // 0 para sim, 1 para não
	char nome[8]; // nome do arquivo
	uint16_t tam; // tam do arquivo
	char dados[32]; // se for um arquivo, poderá ter 4 pastas dentro
	// se for mais terá de alocar nos indiretos, tam representará a quantidade de pastas
 	uint16_t ponteiro_indireto; // ponteiro indireto para o bloco
	// de dados de ponteiro
};

void escreveBlocoDados(FILE * so);
void readInode(FILE * so, int inode_num);
int createFolder(FILE * so, char * name);
void createFile(FILE * so, char * name, char * dados, int inode_dest);


int main(int argc, char const *argv[])  { 
	
	FILE * arq;

	arq = fopen("SO.bin", "wb+");

	// tamanho de blocos extra, cada inode poderá ter um bloco indireto armazenado idealmente
	super_bloco.prox_disponivel = (uint16_t) 10; // ultimo item da pilha

	fwrite(&super_bloco, sizeof(super_bloco), 1, arq);
	char oi[2];
	//fwrite(&oi, sizeof(oi), 1, arq);

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

	// cria a pasta master, /
	createFolder(arq, "/");

	readInode(arq, 0);

	// esta criado o sistema, agora é só criar pastas
	// e arquivos dentro dele




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

// cria uma pasta, retorna o numero do inodo
int createFolder(FILE * so, char * name) {
	int inode_num;
	struct inodo novo;

	// vai para regiao de inodes
	fseek(so, sizeof(super_bloco),SEEK_SET);

	for(int i = 0; i < n_inodes; i++) {
		fread(&novo, sizeof(novo), 1, so);
		if(novo.status == 0) { // encontrou um bloco vazio
			inode_num = i;
			break;
		}
	}

	int pos = 2+ (sizeof(novo)*inode_num);

	fseek(so, pos,SEEK_SET);

	novo.status=1; // ocupado
	novo.pasta=1; // diz que é uma pasta
	strcpy(novo.nome, name); // altera nome da pasta
	novo.tam = 0;

	fwrite(&novo, sizeof(novo), 1, so);
	
	return inode_num;
}

// cria um arquivo e retorna o inode dele
void createFile(FILE * so, char * name, char * dados, int inode_dest) {
	// @todo: colocar .. como endereço do diretorio pai
	int inode_novo;
	struct inodo novo, aux;

	for(int i = 0; i < n_inodes; i++) {
		fread(&novo, sizeof(novo), 1, so);
		if(novo.status == 0) { // encontrou um bloco vazio
			inode_novo = i;
			break;
		}
	}

	int pos = 2+ (sizeof(novo)*inode_novo);

	fseek(so, pos,SEEK_SET);

	novo.status=1; // ocupado
	novo.pasta=0; // diz que é arquivo
	strcpy(novo.nome, name); // altera nome da arquivo
	novo.tam = strlen(dados);
	
	/* */

	if(novo.tam > 32) { // aloca blocos indiretos

	} else { // aloca normalmente
		strcpy(novo.dados, dados);
	}

	fwrite(&novo, sizeof(novo), 1, so);
	
	// coloca o endereço do inode atual na pasta

	pos = 2+ (sizeof(novo)*inode_dest);
	fseek(so, pos,SEEK_SET);

	fwrite(&aux, sizeof(aux), 1, so);
	
	aux.tam += 1;
	

	// percorre os dados encontrando espaço vazio

}
