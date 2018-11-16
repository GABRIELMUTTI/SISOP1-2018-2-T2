#include "../include/aux.h"




char workingDir[MAX_PATH_SIZE] = "/dir1\0";


//verifica se pathnameOriginal eh um link, se for copia o path para o pathnameNew
//retorna 0 se nao for, 1 se for e -1 se der algum erro
int checkiflink(char* pathnameOriginal, char* pathnameNew)
{
    DWORD file_cluster = FindFile(pathnameOriginal);

    if(file_cluster == -1) return -1; //ERROR
    if(NextCluster(file_cluster) == 0xFFFFFFFE) return -1; //corrompido
    
    //pega dir
    char* path = malloc(MAX_PATH_SIZE);
    char* name = malloc(51);
    DividePathAndFile(pathnameOriginal, path, name);
    DWORD dir_cluster = FindFile(path);
    
    
    
    if(dir_cluster == -1) {free(name);return -1;}
    if(NextCluster(dir_cluster) == 0xFFFFFFFE) {free(name);return -1;} //corrompido
    
    //pega entrada
    struct t2fs_record* entrada = SearchEntradas(dir_cluster,name);
    free(name);
    if(entrada == NULL) return -1;
    //testa se e link 
    if(entrada->TypeVal != TYPEVAL_LINK) {free(entrada);return 0;}
    free(entrada);
    //pega o path de verdade
    DWORD sector = SetorLogico_ClusterDados(file_cluster);
    BYTE* buffer = malloc(256);
    if(read_sector(sector,buffer)) {free(buffer); return -1;}
    
    int i = 0;
    while(buffer[i] != '\0')
    {
        pathnameNew[i] = buffer[i];
        i++;
    }
    pathnameNew[i] = buffer[i];
    free(buffer);
    
    
    return 1;
     



}

//apaga a entrada de name no dir path
//retorna 0 se conseguiu e -1 se falhou
int EraseEntry(char* path,char* name)
{
     DWORD cluster = FindFile(path);
     if(cluster == -1) return -1;
     
     if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido

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

//devolve 1 se vazio e dir, 0 se nao
int CheckIfDirAndEmpty(DWORD cluster)
{     

    if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
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







//Escreve as entradas '.' e '..' em um dir novo
//retorna 0 se sucesso e -1 se falha
int StartNewDir(DWORD cluster, BYTE* new_dir_entry, DWORD cluster_father)
{
    if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
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

//acha um cluster vazio na FAT e o ocupa com FFF...FF
//retorna o cluster
int OccupyFreeCluster()
{
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    DWORD sector = superbloco.pFATSectorStart;
    BYTE* buffer = malloc(SECTOR_SIZE);
    if(read_sector(sector, buffer)) {free(buffer); return -1;}
    DWORD pos_atual = 0;
    DWORD clusterIndex = 0;

    while(1)
    {
        if(pos_atual >= 256) //fim do setor
        {
            if(read_sector(++sector, buffer)){free(buffer); return -1;}
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
    if(write_sector(sector,buffer)) {free(buffer); return -1;}

    free(buffer);

    return clusterIndex;
    
    

}

//recebe um pathname e divide para o path e o name
void DividePathAndFile(char *pathname,char *path, char *name)
{
   int i = 0;
   int j = 0;
   int k;
   int n;
   int fim = 0;
   path[0] = '\0';
   while(1)
   {    
       while(pathname[j+i] != '/' )
       {
            name[i] = pathname[j+i];
            if(pathname[(j+i)] == '\0') {fim=1;break;}
            i++;
            
       }
       if(pathname[j+i+1]=='\0' && fim==0) break;
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

//devolve o cluster do arquivo
int FindFile(char *pathname)
{
    if(pathname[0]=='\0') return FindFile(workingDir);
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    int cluster;
    char name[51];
    int i = 1;
    
    if(pathname[0] == '/')//absoluto
        cluster = superbloco.RootDirCluster;
    else 
    {
          if(pathname[0] == '.' && pathname[1] == '.')//relativo pai 
          {
                char* path1 = malloc(MAX_PATH_SIZE);
                strcpy(path1,workingDir);
                char* path2 = malloc(MAX_PATH_SIZE);
                char* name2 = malloc(51);
                DividePathAndFile(path1,path2,name2);
                free(path1);
                free(name2);
                cluster = FindFile(path2);
                if(pathname[2] == '/')
                    i+=2;
                else
                    i++;
                free(path2);
                           
          }
          else //relativo current
         {  
                if(pathname[0] == '.') // relativo ex: ./dir
                    {if(pathname[1] == '/') i++;}
                else                   // relativo ex:  dir
                    i = 0;
                char* path = malloc(MAX_PATH_SIZE);
                strcpy(path,workingDir);
                cluster = FindFile(path);
                free(path);
                
         }
    }
   if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
            
    
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
        if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
        if(cluster < 0) return -1;

      
      j=0;
    }
    

}

//acha uma entrada vazia no cluster e escreve a entrada 
//devolve o numero da entrada se sucesso e -1 se falha
int WriteInEmptyEntry(DWORD cluster,BYTE* entrada )
{
    if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
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
                if (cluster == -1){free(buffer2);return -1;}// END OF FILE;
                if(NextCluster(cluster) == 0xFFFFFFFE) {free(buffer2);return -1;} //corrompido
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


//recebe cluster inicial de um dir e um nome, 
//devolve a entrada do arquivo nome
struct t2fs_record* SearchEntradas(DWORD cluster,char name[51])
{
    if(NextCluster(cluster) == 0xFFFFFFFE) return NULL; //corrompido
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
    
    while(entrada->TypeVal != TYPEVAL_INVALIDO && strcmp(name,entrada->name)!=0 && file_size/64 > i)
    {
        if(j>=bloco)  //acabou o bloco
            {
                cluster = NextCluster(cluster);
                if (cluster == -1){free(entrada);return NULL;}// END OF FILE;
                if(NextCluster(cluster) == 0xFFFFFFFE) {free(entrada);return NULL;} //corrompido
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



//recebe sector inicial de um dir, le a entrada n, guarda em entrada.
//retorna 0 se sucesso e -1 se falha
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


//Devolve o superbloco
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


//TRaduz de cluster para setor
DWORD SetorLogico_ClusterDados(DWORD cluster)
{
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    DWORD end = (superbloco.DataSectorStart) + (cluster * superbloco.SectorsPerCluster);
    return end;
}


//devolve o proximo cluster apontado na FAT
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

//devolve o setor que o offset se encontra
DWORD FindFileOffsetSector(struct t2fs_record *fileRecord, DWORD offset) {

    struct t2fs_superbloco superblock = ReadSuperbloco();
    unsigned int numOffsetSectors = offset / SECTOR_SIZE;
    
    DWORD currentCluster = fileRecord->firstCluster;
    if(NextCluster(currentCluster) == 0xFFFFFFFE) return -1; //corrompido
    unsigned int sectorCounter = 0;
    unsigned int offsetSectorCounter = 0;

    // Acha o setor do offset nos outros clusters.
    while (offsetSectorCounter < numOffsetSectors && currentCluster != 0xFFFFFFFF) {
			
	offsetSectorCounter++;
	sectorCounter++;
	
	if (sectorCounter >= superblock.SectorsPerCluster) {
	    currentCluster = NextCluster(currentCluster);
	    if(NextCluster(currentCluster) == 0xFFFFFFFE) return -2; //corrompido
	    
	    sectorCounter = 0;
	}
    }

    return SetorLogico_ClusterDados(currentCluster) + sectorCounter;
}

//atualiza a fat com value
int UpdateFatEntry(unsigned int entry, DWORD value) {
    struct t2fs_superbloco superblock = ReadSuperbloco();

    unsigned int entrySector = superblock.pFATSectorStart + (entry / (SECTOR_SIZE / superblock.SectorsPerCluster));

    DWORD *buffer = malloc(sizeof(DWORD) * SECTOR_SIZE / superblock.SectorsPerCluster);
    if (buffer == 0) { return -1; }

    if (read_sector(entrySector, (BYTE*)(buffer))) {
	free(buffer);
	return -2;
    }

    unsigned int entryIndex = entry % (SECTOR_SIZE / superblock.SectorsPerCluster);
    buffer[entryIndex] = value;

    if (write_sector(entrySector, (BYTE*)(buffer))) {
	free(buffer);
	return -3;
    }

    free(buffer);
    return 0;
    
}

//acha o ultimo cluster de um arquivo
DWORD FindLastCluster(DWORD firstCluster)
{
    struct t2fs_superbloco superblock = ReadSuperbloco();
    unsigned int numEntriesPerSector = SECTOR_SIZE / 4;
    unsigned int entrySector = superblock.pFATSectorStart + (firstCluster / numEntriesPerSector);

    DWORD *buffer = malloc(sizeof(DWORD) * SECTOR_SIZE);
    
    // Não conseguiu alocar.
    if (buffer == 0) { return -1; }
    
    DWORD currentCluster = firstCluster;
    DWORD lastCluster;
    unsigned int currentEntry = firstCluster;

    do {
	lastCluster = currentCluster;
	
	if (read_sector(entrySector, (BYTE*)(buffer)) != 0) {
	    free(buffer);
	    return -2;
	}

	currentCluster = buffer[currentEntry];
	currentEntry = currentCluster;
	entrySector = superblock.pFATSectorStart + (currentCluster / numEntriesPerSector);
    } while(currentCluster != 0xFFFFFFFF);

    free(buffer);
    return lastCluster;
}

int UpdateDirEntry(DWORD directory_cluster, struct t2fs_record *record)
{
    struct t2fs_superbloco superblock = ReadSuperbloco();
    
    int entry = GetFileEntry(directory_cluster, record->name);
    if (entry < 0) { return -1; }

    DWORD entrySector = SetorLogico_ClusterDados(directory_cluster) + ((entry * 64) / SECTOR_SIZE);
    unsigned int entriesPerSector = SECTOR_SIZE / 64;
    
    struct t2fs_record *buffer = malloc(sizeof(struct t2fs_record) * entriesPerSector);
    if (buffer == 0) { return -2; }

    if (read_sector(entrySector, (BYTE*)(buffer)) != 0) {
	free(buffer);
	return -3;
    }

    buffer[entry % entriesPerSector] = *record;
    
    if (write_sector(entrySector, (BYTE*)(buffer)) != 0) {
	free(buffer);
	return -4;
    }

    free(buffer);

    return 0;
}

//devolve a entrada do arquivo nome
int GetFileEntry(DWORD cluster,char name[51])
{
    if(NextCluster(cluster) == 0xFFFFFFFE) return NULL; //corrompido
     struct t2fs_superbloco superbloco  = ReadSuperbloco();
     float bloco = 256.0*superbloco.SectorsPerCluster/64.0;
     
     
     DWORD sector_first = SetorLogico_ClusterDados(cluster);
     
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     //Get dir size
     if(read_sector(sector_first ,buffer2)) {free(buffer2);return -1;} //ERROR
     
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     free(buffer2);
     
    struct t2fs_record* entrada = malloc(sizeof(struct t2fs_record));
    if(ReadEntrada(sector_first, 2, entrada)){free(entrada);return -2;}

    int j = 3;
    int i = 3;
    
    while(entrada->TypeVal != TYPEVAL_INVALIDO && strcmp(name,entrada->name)!=0 && file_size/64 > i)
    {
        if(j>=bloco)  //acabou o bloco
            {
                cluster = NextCluster(cluster);
                if (cluster == -1){free(entrada);return NULL;}// END OF FILE;
                if(NextCluster(cluster) == 0xFFFFFFFE) {free(entrada);return -3;} //corrompido
                sector_first = SetorLogico_ClusterDados(cluster);
                j = 0;
            }
        if(ReadEntrada(sector_first, j, entrada)){free(entrada);return -4;}        
         j++; 
         i++; 
    }
    if(file_size/64 <= i) {free(entrada);return -5;} //END OF FILE
    
    return j - 1;
}
