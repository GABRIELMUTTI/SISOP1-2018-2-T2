#include "t2fs.h"
#include "apidisk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//escreve uma entrada no diretório
int WriteEntrada(DWORD cluster_dir, struct t2fs_record entrada);

//acha um cluster vazio na FAT
int FindFreeCluster();

//recebe um pathname e divide para o path e o name
void DividePathAndFile(char *pathname,char *path, char *name);

//TRaduz de cluster para setor
DWORD SetorLogico_ClusterDados(DWORD cluster);

//devolve o proximo cluster apontado na FAT
DWORD NextCluster(DWORD cluster_atual);

//Devolve o superbloco
struct t2fs_superbloco ReadSuperbloco();

//recebe sector inicial de um dir, le a entrada n, guarda em entrada.
int ReadEntrada(DWORD sector_first, int n_entrada, struct t2fs_record *entrada );

//recebe cluster inicial de um dir e um nome, devolve o cluster do arquivo nome
int SearchEntradas(DWORD cluster_dir,char name[51]);

//devolve o cluster do arquivo
int FindFile(char *pathname);
