#include "../include/aux.h"



/*

DWORD LocateDir(char* pathname)
{
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    DWORD cluster;
    char name[51]
    if(pathname[0] == '/')
        cluster = superbloco.RootDirCluster;
    int i=1;
    int j = 0;
    
    while(pathname[i] != '\0' && pathname[i] != '/')
    {
     
        name[j] = pathname[i];
        i++;
        j++
    }
    name[j+1] = '\0';
    
    

}

*//*
struct t2fs_record ReadEntrada(DWORD sector_dir, int n_entrada)
{
 
    DWORD sector_entrada = sector_dir + n_entrada/4;
  
    BYTE* buffer2 = malloc(SECTOR_SIZE);
    read_sector(sector_entrada,buffer2);
    
    DWORD pos_atual = (n_entrada - 4*(sector_entrada-sector_dir))*64; 
    
   
    struct t2fs_record entrada;
    if(entrada == NULL)printf("aloooooo\n");
    entrada.TypeVal = buffer2[pos_atual];
    int i;
    for(i = 0;i<51;i++)
        entrada.name[i] = buffer2[pos_atual+i+1];
    
    entrada.bytesFileSize = buffer2[pos_atual+52] + buffer2[pos_atual+53]*16*16 + buffer2[pos_atual+54]*16*16*16*16 + buffer2[pos_atual+55]*16*16*16*16*16*16;
    entrada.clustersFileSize = buffer2[pos_atual+56] + buffer2[pos_atual+57]*16*16 + buffer2[pos_atual+58]*16*16*16*16 + buffer2[pos_atual+59]*16*16*16*16*16*16;
    entrada.firstCluster = buffer2[pos_atual+60] + buffer2[pos_atual+61]*16*16 + buffer2[pos_atual+62]*16*16*16*16 + buffer2[pos_atual+62]*16*16*16*16*16*16;
    
    free(buffer2);
    return entrada;
}
*/
struct t2fs_superbloco ReadSuperbloco()
{
    BYTE* buffer = malloc(SECTOR_SIZE);
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
    DWORD end = (superbloco.DataSectorStart) + (cluster * superbloco.SectorsPerCluster);
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




