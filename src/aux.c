#include "../include/aux.h"


char workingDir[MAX_PATH_SIZE] = "/dir1\0";

int EraseEntry(char* path,char* name)
{
     DWORD cluster = FindFile(path);
     if(cluster == -1) return -1;

     char nameEntry[51];
     int x;
     for(x = 0; x < 51; x++) nameEntry[x] = name[x];
     //procura a entrada
     struct t2fs_superbloco superbloco  = ReadSuperbloco();
     int file_size = superbloco.SectorsPerCluster*256;
     DWORD sector = SetorLogico_ClusterDados(cluster);
       
    struct t2fs_record* entrada = malloc(sizeof(struct t2fs_record));


    int i = 2;
    int j = 2;
    if(ReadEntrada(sector, i, entrada) == -1){free(entrada);return -1;}
    while(strcmp(nameEntry,entrada->name)!=0 && file_size/64 > i)
    {
        j++;
        if(j == 4) j = 0;
        i++;
        if(ReadEntrada(sector, i, entrada)){free(entrada);return -1;}
        
        
    }
    free(entrada);
    if(file_size/64 <= i) return -1; //END OF FILE
    
    //apaga a entrada
    BYTE* buffer = malloc(SECTOR_SIZE);
    if(read_sector(sector + i/4,buffer)) {free(buffer);return -1;}
    int k;
    for(k = 0;k < 64;k++) buffer[j*64 + k] = '\0';
    if(write_sector(sector + i/4,buffer)) {free(buffer);return -1;}
    free(buffer);
    
    return 0;


}


int CheckIfDirAndEmpty(DWORD cluster)
{     
     DWORD sector = SetorLogico_ClusterDados(cluster);
     
     
     //Get dir size
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     if(read_sector(sector ,buffer2)) {free(buffer2);return 0;} //ERROR
     if(buffer2[0] != TYPEVAL_DIRETORIO || buffer2[1] != '.')
        {free(buffer2);return 0;}//NOT A DIR
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     
    int j = 2;
    int i = 2;
    //procura entrada nao vazia
    while(file_size/64 > i)
    {
        
        if(buffer2[j*64] != TYPEVAL_INVALIDO)
        {
            free(buffer2);
            return 0;//diretorio nao vazio
        }
        
        if(j == 3) // acabou setor
         {
            sector++;
            if(read_sector(sector ,buffer2)) {free(buffer2);return 0;} //ERROR
            j=-1;
         }        
         j++; 
         i++; 
    }
    
    free(buffer2);
    return 1; //END OF FILE, diretorio vazio




}








int StartNewDir(DWORD cluster, BYTE* new_dir_entry, DWORD cluster_father)
{
    DWORD newDirSector = SetorLogico_ClusterDados(cluster);
    BYTE* buffer = malloc(256);
    if(read_sector(newDirSector, buffer)) {free(buffer);return -1;}
    ///// .
    buffer[0] =  new_dir_entry[0];
    buffer[1] = '.';
    int i;
    for(i=2;i<52;i++) buffer[i] = '\0';
    for(i=52;i<64;i++) buffer[i] = new_dir_entry[i];
    ////  ..   
    int fatherBegin = 64;
    DWORD fatherDirSector = SetorLogico_ClusterDados(cluster_father);
    BYTE* buffer2 = malloc(256);
    if(read_sector(fatherDirSector,buffer2)){free(buffer);free(buffer2);return -1;}
    buffer[fatherBegin+0] = buffer2[0];
    buffer[fatherBegin+1] = '.';
    buffer[fatherBegin+2] = '.';
    for(i=3;i<52;i++) buffer[fatherBegin+i] = '\0';
    for(i=52;i<64;i++) buffer[fatherBegin+i] = buffer2[i];
    free(buffer2);
    for(i=fatherBegin+64;i<256;i++) buffer[i] = '\0';
    if(write_sector(newDirSector,buffer)) {free(buffer);return -1;}
    
    //erase any garbage in the dir
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    int k;
    for(k = 0; k < 256;k++) buffer[k] = '\0';
    for(k=1;k < superbloco.SectorsPerCluster;k++)
        if(write_sector(newDirSector+k,buffer)) {free(buffer);return -1;}
    
    free(buffer);
    
    return 0;
}


int OccupyFreeCluster()
{
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    DWORD sector = superbloco.pFATSectorStart;
    BYTE* buffer = malloc(SECTOR_SIZE);
    read_sector(sector, buffer);
    DWORD pos_atual = 0;
    DWORD clusterIndex = 0;
    while(1)
    {
        if(pos_atual >= 256) //fim do setor
        {
            read_sector(++sector, buffer);
            pos_atual = 0;
        }
        DWORD cluster = buffer[pos_atual] + buffer[pos_atual+1]*16*16 + buffer[pos_atual+2]*16*16*16*16 + buffer[pos_atual+3]*16*16*16*16*16*16;
        if(cluster == 0) break; //achou 
              
        pos_atual += 4;
        clusterIndex++;
    }
    buffer[pos_atual] = 0XFF;//ocupa o cluster
    buffer[pos_atual+1] = 0XFF;
    buffer[pos_atual+2] = 0XFF;
    buffer[pos_atual+3] = 0XFF;
    write_sector(sector,buffer);

    free(buffer);
    return clusterIndex;
    
    

}
void DividePathAndFile(char *pathname,char *path, char *name)
{
   int i = 0;
   int j = 0;
   int k;
   int n;
   int fim = 0;
   while(1)
   {    
       while(pathname[j+i] != '/' )
       {
            name[i] = pathname[j+i];
            if(pathname[(j+i)] == '\0') {fim=1;break;}
            i++;
            
       }
       if(fim)break; //End path
       name[i] = '/';
       n=0;
       for(k = j;k <= j+i;k++) path[k]=name[n++];

       j++;
       j += i;
       i = 0;
   }  
   if(path[1]!='\0')//not root
        path[j-1]='\0';


}

int FindFile(char *pathname)
{
    if(pathname[0]=='\0') return -1;
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    int cluster;
    char name[51];
    int i = 1;
    
    if(pathname[0] == '/')//absoluto
        cluster = superbloco.RootDirCluster;
    else 
    {
          if(pathname[1] == '.')//relativo pai 
          {
                char* path1 = malloc(MAX_PATH_SIZE);
                strcpy(path1,workingDir);
                char* path2 = malloc(MAX_PATH_SIZE);
                char* name2 = malloc(51);
                DividePathAndFile(path1,path2,name2);
                free(path1);
                free(name2);
                cluster = FindFile(path2);
                free(path2);
                i+=2;           
          }
          else //relativo current
         {  
                if(pathname[0] == '.') // relativo ex: ./dir
                    i++;
                else                   // relativo ex:  dir
                    i = 0;
                char* path = malloc(MAX_PATH_SIZE);
                strcpy(path,workingDir);
                cluster = FindFile(path);
                free(path);
                
         }
    }
   
            
    
    struct t2fs_record* entrada;
    int j = 0;
    while(1)
    {
       
        if(pathname[i] == '\0')
            return cluster;
            
        while(pathname[i] != '\0' && pathname[i] != '/')
        { 
            name[j] = pathname[i];
            i++;
            j++;
        }
        
        name[j] = '\0';
        if(name[0] == '\0')  //caso o path terminar em /   ex:/dir/
             {return cluster;}
        entrada = SearchEntradas(cluster, name);
        if(entrada == NULL) return -1;
        cluster = entrada->firstCluster; 
        free(entrada);
        if(cluster < 0) return -1;

      
      j=0;
    }
    

}

int WriteInEmptyEntry(DWORD cluster,BYTE* entrada )
{
     struct t2fs_superbloco superbloco  = ReadSuperbloco();
     int bloco = 256.0*superbloco.SectorsPerCluster/64.0;
     
     DWORD sector = SetorLogico_ClusterDados(cluster);
     
     
     //Get dir size
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     if(read_sector(sector ,buffer2)) {free(buffer2);return -1;} //ERROR
     
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     
    int j = 2;
    int i = 2;
    while(buffer2[j*64] != TYPEVAL_INVALIDO && file_size/64 > i)//procura entrada vazia
    {
        if(i%bloco == 0.0)  //acabou o bloco
            {
                cluster = NextCluster(cluster);
                if (cluster == -1)return -1;;// END OF FILE;
                sector = SetorLogico_ClusterDados(cluster);
                if(read_sector(sector ,buffer2)) {free(buffer2);return -1;} //ERROR
                j = -1;
            }
         else if(j == 3) // acabou setor
         {
            sector++;
            if(read_sector(sector ,buffer2)) {free(buffer2);return -1;} //ERROR
            j=-1;
         }        
         j++; 
         i++; 
    }
    if(file_size/64 <= i) return -1; //END OF FILE
    if(j == 4) // achou e acabou setor
    {
        sector++;
        if(read_sector(sector ,buffer2)) {free(buffer2);return -1;} //ERROR
        j=0;
    }
   
    int k;
    for(k = 0;k<64;k++)
        buffer2[k+j*64] = entrada[k];

    if(write_sector(sector, buffer2)) return -1;
    free(buffer2);
    return i;


}



struct t2fs_record* SearchEntradas(DWORD cluster,char name[51])
{
     struct t2fs_superbloco superbloco  = ReadSuperbloco();
     float bloco = 256.0*superbloco.SectorsPerCluster/64.0;
     
     
     DWORD sector_first = SetorLogico_ClusterDados(cluster);
     
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     //Get dir size
     if(read_sector(sector_first ,buffer2)) {free(buffer2);return NULL;} //ERROR
     
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     free(buffer2);
     
    struct t2fs_record* entrada = malloc(sizeof(struct t2fs_record));
    if(ReadEntrada(sector_first, 2, entrada)){free(entrada);return NULL;}

    int j = 3;
    int i = 3;
    
    while(strcmp(name,entrada->name)!=0 && file_size/64 > i)
    {
        if(j>=bloco)  //acabou o bloco
            {
                cluster = NextCluster(cluster);
                if (cluster == -1){free(entrada);return NULL;}// END OF FILE;
                sector_first = SetorLogico_ClusterDados(cluster);
                j = 0;
            }
        if(ReadEntrada(sector_first, j, entrada)){free(entrada);return NULL;}        
         j++; 
         i++; 
    }
    if(file_size/64 <= i) {free(entrada);return NULL;} //END OF FILE
    
    
    
    return entrada;


}




int ReadEntrada(DWORD sector_dir, int n_entrada, struct t2fs_record *entrada )
{    
       
        DWORD sector_entrada = sector_dir + n_entrada/4;
        BYTE* buffer2 = malloc(SECTOR_SIZE);
        if(read_sector(sector_entrada,buffer2)){free(buffer2);return -1;}
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
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    DWORD sector_cluster = cluster_atual/64 + superbloco.pFATSectorStart;
    BYTE* buffer = malloc(SECTOR_SIZE);
    read_sector(sector_cluster,buffer);
    DWORD pos_atual = (cluster_atual - 64*(sector_cluster-1))*4;
    DWORD next_cluster = buffer[pos_atual] + buffer[pos_atual+1]*16*16 + buffer[pos_atual+2]*16*16*16*16 + buffer[pos_atual+3]*16*16*16*16*16*16;
    
    free(buffer);
    return next_cluster;
}

DWORD FindFileOffsetSector(struct t2fs_record *fileRecord, DWORD offset) {

    struct t2fs_superbloco superblock = ReadSuperbloco();
    unsigned int numOffsetSectors = offset / SECTOR_SIZE + (offset % SECTOR_SIZE == 0);
    
    DWORD currentCluster = fileRecord->firstCluster;
    unsigned int sectorCounter = 0;
    unsigned int offsetSectorCounter = 1;

    // Acha o setor do offset nos outros clusters.
    while (offsetSectorCounter < numOffsetSectors && currentCluster != 0xFFFFFFFF) {
			
	offsetSectorCounter = offsetSectorCounter + 1;
	sectorCounter = sectorCounter + 1;
	
	if (sectorCounter >= superblock.SectorsPerCluster) {
	    sectorCounter = 0;
	}
	
	currentCluster = NextCluster(currentCluster);
    }

    return SetorLogico_ClusterDados(currentCluster) + sectorCounter - 1;
}

int UpdateFatEntry(unsigned int entry, DWORD value) {
    struct t2fs_superbloco superblock = ReadSuperbloco();

    unsigned int entrySector = superblock.pFATSectorStart + (entry / (SECTOR_SIZE / 4));

    DWORD *buffer = malloc(sizeof(DWORD) * SECTOR_SIZE / 4);

    if (read_sector(entrySector, buffer)) {
	return -1;
    }

    unsigned int entryIndex = entry % (SECTOR_SIZE / 4);
    buffer[entryIndex] = value;

    if (write_sector(entrySector, buffer)) {
	return -1;
    }

    return 0;
    
}




