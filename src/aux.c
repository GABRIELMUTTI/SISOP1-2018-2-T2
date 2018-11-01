#include "../include/aux.h"

int main(int argc, char *argv[]){



    return 0;
}


struct t2fs_superbloco ReadSuperbloco()
{
    BYTE* buffer = malloc(256);
    read_sector(0,buffer);
    struct t2fs_superbloco superbloco;
    
    superbloco.id[0] = buffer[0];
    superbloco.id[1] = buffer[1];
    superbloco.id[2] = buffer[2];
    superbloco.id[3] = buffer[3];
    
    superbloco.version = buffer[4] + buffer[5] * 16*16;
    superbloco.superblockSize = buffer[6] + buffer[7]*16*16;
    superbloco.DiskSize = buffer[8] + buffer[9]*16*16 + buffer[10]*16*16*16*16 + buffer[11]*16*16*16*16*16*16; 
    superbloco.NofSectors = buffer[12] + buffer[13]*16*16 + buffer[14]*16*16*16*16 + buffer[15]*16*16*16*16*16*16;
    superbloco.SectorsPerCluster = buffer[16]+ buffer[17]*16*16 + buffer[18]*16*16*16*16 + buffer[19]*16*16*16*16*16*16;
    superbloco.pFATSectorStart = buffer[20]+ buffer[21]*16*16 + buffer[22]*16*16*16*16 + buffer[23]*16*16*16*16*16*16;
    superbloco.RootDirCluster = buffer[24]+ buffer[25]*16*16 + buffer[26]*16*16*16*16 + buffer[27]*16*16*16*16*16*16;
    superbloco.DataSectorStart = buffer[28]+ buffer[29]*16*16 + buffer[30]*16*16*16*16 + buffer[31]*16*16*16*16*16*16;
    
    free(buffer);
    return superbloco;
     
}

DWORD SetorLogico_ClusterDados(struct t2fs_superbloco superbloco, DWORD cluster)
{
    DWORD end = (superbloco.DataSectorStart*SECTOR_SIZE) + (SECTOR_SIZE * cluster * superbloco.SectorsPerCluster);
    return end;
}



DWORD NextCluster(struct t2fs_superbloco superbloco, DWORD cluster_atual)
{
    DWORD sector_cluster = cluster_atual/64 + 1;
    BYTE* buffer = malloc(SECTOR_SIZE);
    read_sector(sector_cluster,buffer);
    DWORD pos_atual = (cluster_atual - 64*(sector_cluster-1))*4;
    DWORD next_cluster = buffer[pos_atual] + buffer[pos_atual+1]*16*16 + buffer[pos_atual+2]*16*16*16*16 + buffer[pos_atual+3]*16*16*16*16*16*16;
    
    free(buffer);
    return next_cluster;
}




