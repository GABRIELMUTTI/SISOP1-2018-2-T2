#include "t2fs.h"
#include "apidisk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_PATH_SIZE 255


//verifica se pathnameOriginal eh um link, se for copia o path para o pathnameNew
//retorna 0 se nao for, 1 se for e -1 se der algum erro
int checkiflink(char* pathnameOriginal, char* pathnameNew);

//apaga a entrada de name no dir path
//retorna 0 se conseguiu e -1 se falhou
int EraseEntry(char* path,char* name);


//devolve 1 se vazio e dir, 0 se nao
int CheckIfDirAndEmpty(DWORD cluster);


//Escreve as entradas '.' e '..' em um dir novo
//retorna 0 se sucesso e -1 se falha
int StartNewDir(DWORD cluster, BYTE* new_dir_entry, DWORD cluster_father);



//acha uma entrada vazia no cluster e escreve a entrada 
//devolve o numero da entrada se sucesso e -1 se falha
int WriteInEmptyEntry(DWORD cluster,BYTE* entrada);


//acha um cluster vazio na FAT e o ocupa com FFF...FF
//retorna o cluster
int OccupyFreeCluster();

//recebe um pathname e divide para o path e o name
void DividePathAndFile(char *pathname,char *path, char *name);

//TRaduz de cluster para setor
DWORD SetorLogico_ClusterDados(DWORD cluster);

//devolve o proximo cluster apontado na FAT
DWORD NextCluster(DWORD cluster_atual);

//Devolve o superbloco
struct t2fs_superbloco ReadSuperbloco();

//recebe sector inicial de um dir, le a entrada n, guarda em entrada.
//retorna 0 se sucesso e -1 se falha
int ReadEntrada(DWORD sector_first, int n_entrada, struct t2fs_record *entrada );

//recebe cluster inicial de um dir e um nome, 
//devolve a entrada do arquivo nome
struct t2fs_record* SearchEntradas(DWORD cluster_dir,char name[51]);

//devolve o cluster do arquivo
int FindFile(char *pathname);

//devolve o setor que o offset se encontra
DWORD FindFileOffsetSector(struct t2fs_record *fileRecord, DWORD offset);

//atualiza a fat com value
int UpdateFatEntry(unsigned int entry, DWORD value);

int UpdateDirEntry(DWORD directory_cluster, struct t2fs_record* record);

int GetFileEntry(DWORD cluster,char name[51]);

//acha o ultimo cluster de um arquivo
DWORD FindLastCluster(DWORD firstCluster);
