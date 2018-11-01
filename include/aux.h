#include "t2fs.h"
#include "apidisk.h"
#include <stdlib.h>
#include <stdio.h>




DWORD SetorLogico_ClusterDados(struct t2fs_superbloco superbloco, DWORD cluster);

DWORD NextCluster(struct t2fs_superbloco superbloco, DWORD cluster_atual);

struct t2fs_superbloco ReadSuperbloco();

//struct t2fs_record ReadEntrada(DWORD sector_dir, int n_entrada);
