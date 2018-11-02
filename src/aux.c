#include "../include/aux.h"



int FindFile(char *pathname)
{
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    int cluster;
    char name[51];
    if(pathname[0] == '/')
        cluster = superbloco.RootDirCluster;
    int i = 1;
    int j = 0;
    while(1)
    {
        while(pathname[i] != '\0' && pathname[i] != '/')
        { 
            name[j] = pathname[i];
            i++;
            j++;
        }
        
        name[j] = '\0';
        cluster = SearchEntradas(cluster, name);   
        if(cluster < 0) return -1;
        if(pathname[i] == '\0')
            return cluster;
            
      i++;
      j=0;
    }
    

}



int SearchEntradas(DWORD cluster,char name[51])
{
     
     
     DWORD sector_first = SetorLogico_ClusterDados(cluster);
     
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     //Get dir size
     if(read_sector(sector_first ,buffer2)) {free(buffer2);return -1;} //ERROR
     
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     free(buffer2);
     
    struct t2fs_record* entrada = malloc(sizeof(struct t2fs_record));
    if(ReadEntrada(sector_first, 2, entrada))return -1;

    int j = 3;
    int i = 3;
    
    while(strcmp(name,entrada->name)!=0 && file_size/64 > i)
    {
        if(j>=16)  //acabou o bloco
            {
                cluster = NextCluster(cluster);
                if (cluster == -1)return -1;// END OF FILE;
                sector_first = SetorLogico_ClusterDados(cluster);
                j = 0;
            }
        if(ReadEntrada(sector_first, j, entrada))return -1;        
         j++; 
         i++; 
    }
    if(file_size/64 <= i) return -1; //END OF FILE
    
    
    
    return entrada->firstCluster;


}




int ReadEntrada(DWORD sector_dir, int n_entrada, struct t2fs_record *entrada )
{    
       
        DWORD sector_entrada = sector_dir + n_entrada/4;
        BYTE* buffer2 = malloc(SECTOR_SIZE);
        if(read_sector(sector_entrada,buffer2))return 0;
        DWORD pos_atual = (n_entrada - 4*(sector_entrada-sector_dir))*64;
        
        
        entrada->TypeVal = buffer2[pos_atual];
        int i;
        for(i = 0;i<51;i++)
            entrada->name[i] = buffer2[pos_atual+i+1];
        
        entrada->bytesFileSize = buffer2[pos_atual+52] + buffer2[pos_atual+53]*16*16 + buffer2[pos_atual+54]*16*16*16*16 + buffer2[pos_atual+55]*16*16*16*16*16*16;
        entrada->clustersFileSize = buffer2[pos_atual+56] + buffer2[pos_atual+57]*16*16 + buffer2[pos_atual+58]*16*16*16*16 + buffer2[pos_atual+59]*16*16*16*16*16*16;
        entrada->firstCluster = buffer2[pos_atual+60] + buffer2[pos_atual+61]*16*16 + buffer2[pos_atual+62]*16*16*16*16 + buffer2[pos_atual+62]*16*16*16*16*16*16;
        free(buffer2);
        
        return 0;
}
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

DWORD SetorLogico_ClusterDados(DWORD cluster)
{
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    DWORD end = (superbloco.DataSectorStart) + (cluster * superbloco.SectorsPerCluster);
    return end;
}



DWORD NextCluster(DWORD cluster_atual)
{
    DWORD sector_cluster = cluster_atual/64 + 1;
    BYTE* buffer = malloc(SECTOR_SIZE);
    read_sector(sector_cluster,buffer);
    DWORD pos_atual = (cluster_atual - 64*(sector_cluster-1))*4;
    DWORD next_cluster = buffer[pos_atual] + buffer[pos_atual+1]*16*16 + buffer[pos_atual+2]*16*16*16*16 + buffer[pos_atual+3]*16*16*16*16*16*16;
    
    free(buffer);
    return next_cluster;
}




