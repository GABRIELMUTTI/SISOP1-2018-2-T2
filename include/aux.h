#include "t2fs.h"
#include "apidisk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



//TRaduz de cluster para seto
DWORD SetorLogico_ClusterDados(struct t2fs_superbloco superbloco, DWORD cluster);

//devolve o proximo cluster apontado na FAT
DWORD NextCluster(struct t2fs_superbloco superbloco, DWORD cluster_atual);

//Devolve o superbloco
struct t2fs_superbloco ReadSuperbloco();

//recebe sector inicial de um dir, le a entrada n, guarda em entrada.
int ReadEntrada(DWORD sector_dir, int n_entrada, struct t2fs_record *entrada );

//recebe cluster inicial de um dir e um nome, devolve o cluster do arquivo nome
int SearchEntradas(DWORD cluster_dir,char name[51]);

//devolve o cluster do arquivo
int FindFile(char *pathname);
