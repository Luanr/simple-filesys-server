#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#define BLOCK_SIZE 32
#define n_inodes 8
#define comeco_ponteiros (sizeof(super_block) + (sizeof(inodo) * n_inodes)) // 46

#define ANSI_BOLDBLUE    "\033[1m\033[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

struct superblock {
	//uint8_t block_size;
	//uint8_t number_blocks;
	//uint8_t number_inodes;
	uint16_t prox_disponivel;
} super_bloco;

struct inodo { // tam inodo = 1+1+8+2+32+2 = 46 bytes
	uint8_t status; // 1 byte
	uint8_t pasta; // 1 para sim, 0 para não
	char nome[8]; // nome do arquivo
	uint16_t tam; // tam do arquivo
	char dados[32]; // se for um arquivo, poderá ter 4 pastas dentro
	// se for mais terá de alocar nos indiretos, tam representará a quantidade de pastas
 	uint16_t ponteiro_indireto; // ponteiro indireto para o bloco
	// de dados de ponteiro
	/* 
	char timestamp[16];
	uint8_t permissions[3]; // r-w-x

	*/
};

void escreveBlocoDados(FILE * so);
void readInode(FILE * so, int inode_num);
void createFolder(FILE * so, char * name);
void createFile(FILE * so, char * name, char * dados, int inode_dest);
void simpleReadInode(FILE * so, int inode_num);
void ls(FILE * so, int inode);
int procuraInodo(FILE * so, int inodo_pai, char * name);

int main(int argc, char const *argv[])  { 
	
	FILE * arq;

	arq = fopen("SO.bin", "wb+");

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

	// cria a pasta master, /
	createFolder(arq, "/");

	createFile(arq, "ex.txt", "loremips" ,0);
	createFile(arq, "HEHE", "@@" ,0);
	createFile(arq, "HAHA", "!!!!" ,0);


	// esta criado o sistema, agora é só criar pastas
	// e arquivos dentro dele

	ls(arq, 0);

	printf("Inodo n = %d", procuraInodo(arq, 0, "ex.txt"));

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

	printf("\n Inodo n.%d: \nStatus: %d\nPasta: %d\n Nome: %s \nDados direto: %s \n",inode_num, aux.status,aux.pasta, aux.nome, aux.dados);
}

void simpleReadInode(FILE * so, int inode_num) {
	struct inodo aux;
	int pos = 2+ (sizeof(aux)*inode_num);

	fseek(so, pos,SEEK_SET);


	fread(&aux, sizeof(aux), 1, so);

	if(aux.pasta) { // se é uma pasta, pintar de azul
		printf(ANSI_BOLDBLUE "\n%s" ANSI_COLOR_RESET, aux.nome);
	} else {
		printf("\n%s", aux.nome);
	}
}

// cria uma pasta, retorna o numero do inodo
void createFolder(FILE * so, char * name) {
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
	for(int i = 0; i < 32; i++) {
		novo.dados[i] = 0;
	}

	fwrite(&novo, sizeof(novo), 1, so);
	
	// inodenum é o endereço

}

// cria um arquivo na pasta inode_dest
void createFile(FILE * so, char * name, char * dados, int inode_dest) {
	// @todo: colocar .. como endereço do diretorio pai
	int inode_novo;
	struct inodo novo, aux;

	fseek(so, sizeof(super_bloco),SEEK_SET);

	// procura inodo vazio para colocar novo arquivo
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
	
	if(novo.tam > 32) { // aloca blocos indiretos

	} else { // aloca normalmente
		strcpy(novo.dados, dados);
	}

	// insere novo inodo no arquivo
	fwrite(&novo, sizeof(novo), 1, so);
	
	

	// coloca o endereço do inode atual na pasta

	// inode_dest é o inode da pasta que possui o arquivo
	pos = 2+ (sizeof(novo)*inode_dest);
	fseek(so, pos,SEEK_SET);

	fread(&aux, sizeof(aux), 1, so);	
	aux.tam += 1;
	// inserir ponteiro na proxima posicao livre
	int contador = 0;

	if(aux.tam < 32) {
		while(contador < 32) {
			if(aux.dados[contador] == '\0') {
				aux.dados[contador] = (char) inode_novo;
				break;
			}
			contador++;
		}
	} else { // olha nos indiretos
		// se nao tiver indireto alocado, alocar
		// olhar o próximo disponível
	}	

	// insere alteração do ponteiro adicionado
	fseek(so, pos,SEEK_SET);
	fwrite(&aux, sizeof(aux), 1, so);

	//printf("\n\n Para o arquivo %s", novo.nome);

}

// lista tudo que tem em um inodo pasta
void ls(FILE * so, int inode) {
	struct inodo aux;
	int pos = sizeof(super_bloco) + inode;
	int posAntiga;

	// vai para a posicao do inode
	fseek(so, pos,SEEK_SET);

	// carrega inodo da pasta que 
	// o cara quer ver
	fread(&aux, sizeof(aux), 1, so);

	// numero de elementos que devem ser
	// percorridos

	int numero_arquivos = aux.tam;


	for(int i = 0; i < strlen(aux.dados); i++) {
		if(aux.dados[i] != 0) {
			// se é um inode valido
			//printf("\nINODO =  %d", aux.dados[i]);
			int inodo_filho = aux.dados[i] - '0';
			simpleReadInode(so, inodo_filho);
		}
	}
}

/* procura inodo com aquele nome dentro
	da pasta do inodo pai
	se nao encontrar, retorna -1
 */
int procuraInodo(FILE * so, int inodo_pai, char * name) {
	struct inodo pai;
	int pos = sizeof(super_bloco) + inodo_pai;
	int contador = 0;
	char * nome_aux;

	fseek(so, pos,SEEK_SET);
	fread(&pai, sizeof(pai), 1, so);

	while(contador < pai.tam) {
		// se nao for um ponteiro vazio

		if(pai.dados[contador] != 0) {

			struct inodo filho;


			pos = 2 + sizeof(filho) * ((int) pai.dados[contador]);

			fseek(so, pos,SEEK_SET);
			fread(&filho, sizeof(filho), 1, so);

			// se ele possui o mesmo nome, retornar
			// a posicao

			if(strcmp(filho.nome, name) == 0) {
				return ((pos- sizeof(super_bloco))/sizeof(filho));
			}
		}
		contador++;
	}

	// caso ele nao encontre o arquivo, ele vai retornar -1
	return -1;
}
