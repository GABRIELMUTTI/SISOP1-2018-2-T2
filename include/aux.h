#include "t2fs.h"
#include "apidisk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//Escreve as entradas '.' e '..' em um dir novo
int StartNewDir(DWORD cluster, BYTE* new_dir_entry, DWORD cluster_father);



//acha uma entrada vazia no cluster e escreve a entrada passada
int WriteInEmptyEntry(DWORD cluster,BYTE* entrada);


//acha um cluster vazio na FAT e o ocupa
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
int ReadEntrada(DWORD sector_first, int n_entrada, struct t2fs_record *entrada );

//recebe cluster inicial de um dir e um nome, devolve o cluster do arquivo nome
int SearchEntradas(DWORD cluster_dir,char name[51]);

//devolve o cluster do arquivo
int FindFile(char *pathname);
