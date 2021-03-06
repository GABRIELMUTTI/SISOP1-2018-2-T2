#include "../include/t2fs.h"
#include "../include/aux.h"



extern char workingDir[MAX_PATH_SIZE];


#define MAX_HANDLE  10
struct DirsOpen {
    
    DIR2 handle;
    DWORD first_cluster;
    int CE;

};

struct FilesOpen {

    DIR2 handle;
    DWORD directory_cluster;
    struct t2fs_record *file_data;
    int CP;
};

struct DirsOpen DirsHandle[MAX_HANDLE];
struct FilesOpen FilesHandle[MAX_HANDLE] = {{0}};





int identify2 (char *name, int size){
	
	if(strncpy(name, "Felipe Flores - 274712.\nGabriel Mutti - 261583.\nTatiane Sequerra Stivelman - 243681\n", size) != NULL)
		return 0;
	else
		return -1;
}

FILE2 create2 (char *filename)
{

	
	
    char* path = malloc(MAX_PATH_SIZE);
    char* name = malloc(51);
    DividePathAndFile(filename, path, name);
    DWORD dir_cluster = FindFile(path);
    free(path);
    if(dir_cluster == -1) {free(name);return -1;}
    if(NextCluster(dir_cluster) == 0xFFFFFFFE) {free(name);return -1;} //corrompido
    if(FindFile(filename) != -1) //nome de dir/arquivo ja existente
    {
        struct t2fs_record* entrada = SearchEntradas(dir_cluster, name); 
        if(entrada->TypeVal != TYPEVAL_REGULAR ) {free(entrada);free(name); return -1;}
        free(entrada);
	    if(delete2(filename)) {free(name);return -1;}    
    }
    
    //define a entrada do novo arquivo
    BYTE* entrada = malloc(sizeof(struct t2fs_record));
    entrada[0] = TYPEVAL_REGULAR;
    int i = 0;
    while(name[i] != '\0')
        {entrada[i+1] = name[i]; i++;}
    free(name);
    int j;
    for(j = i;j<51;j++)entrada[j+1] = '\0';
       
	//bytesFileSize
    entrada[52] = 0;
    entrada[53] = 0;
    entrada[54] = 0;
    entrada[55] = 0;
        //ClusterFileSize
    entrada[56] = 0X01;
    entrada[57] = 0X00;
    entrada[58] = 0X00;
    entrada[59] = 0X00;    
        //FirstCluster
    DWORD clusterfree = OccupyFreeCluster();//entrada FAT
    entrada[60] = clusterfree;
    entrada[61] =(clusterfree/16)/16;
    entrada[62] = ((((clusterfree/16)/16)/16)/16);
    entrada[63] =((((((clusterfree/16)/16)/16)/16)/16)/16);        
    
    //escreve a entrada no dir pai
    if(WriteInEmptyEntry(dir_cluster,entrada) == -1){free(entrada);return -1;}
    
    free(entrada);
    int handle = open2(filename);
    
    
    return handle;

}


int delete2 (char *filename)
{
    char* link = malloc(MAX_PATH_SIZE);
    char* filename2 = malloc(MAX_PATH_SIZE);
    int iflink = checkiflink(filename,link);

    if(iflink == -1) {free(link);return -1;}
    else
        if(iflink)
            strcpy(filename2,link);
        else
            strcpy(filename2,filename);
    free(link);
    
    //VERIFICA SE EXISTE
    char* path = malloc(MAX_PATH_SIZE);
    char* name = malloc(51);
    DividePathAndFile(filename2, path, name);
   
    DWORD dir_cluster = FindFile(path);
  
    if(dir_cluster == -1) {free(path);free(name);return -1;}

    if(NextCluster(dir_cluster) == 0xFFFFFFFE) {free(path);free(name);return -1;} //corrompido

    if(FindFile(filename2) == -1){free(path);free(name);return -1;}   //nome de arquivo nao existente
    free(filename2);
    //Verifica se arquivo regular
    struct t2fs_record* entrada = SearchEntradas(dir_cluster, name); 

    if(entrada->TypeVal != TYPEVAL_REGULAR ) {free(path);free(entrada);free(name); return -1;}
    int firstCluster = entrada->firstCluster;
    free(entrada);

	if(NextCluster(dir_cluster) == 0xFFFFFFFE){free(path);free(name);return -1;}
	
	//APAGA ENTRADA
	if(EraseEntry(path,name)) {free(path);free(name);return -1;}
    free(path);
    free(name);
  
	int cluster = firstCluster;
	
	//APAGA ARQUIVO
	struct t2fs_superbloco superbloco = ReadSuperbloco();
    int k;
    BYTE* buffer = malloc(SECTOR_SIZE);
	while(cluster != -1)
	{
	
        for(k = 0; k < 256;k++) buffer[k] = '\0';
        for(k=0;k < superbloco.SectorsPerCluster;k++)
            if(write_sector(SetorLogico_ClusterDados(cluster)+k,buffer)) {free(buffer);return -1;}
	    cluster = NextCluster(cluster);
	    if(cluster == 0xFFFFFFFE){free(buffer);return -1;}
	}
	
		
	cluster = firstCluster;
	//APAGA NA FAT
	DWORD sector_cluster;
	DWORD pos_atual;
	
	while(cluster != -1)
	{
	    sector_cluster = cluster/64 + superbloco.pFATSectorStart;
        if(read_sector(sector_cluster,buffer)){free(buffer);return -1;}
        pos_atual = (cluster - 64*(sector_cluster-1))*4;
        cluster = NextCluster(cluster);
        buffer[pos_atual] = 0;
        buffer[pos_atual+1] = 0;
        buffer[pos_atual+2] = 0;
        buffer[pos_atual+3] = 0;
        if(write_sector(sector_cluster,buffer)){free(buffer);return -1;}
    }
    
    free(buffer);
    return 0;


}
    
FILE2 open2 (char *filename) {
    
    
    char* link = malloc(MAX_PATH_SIZE);
    char* filename2 = malloc(MAX_PATH_SIZE);
    int iflink = checkiflink(filename,link);
    if(iflink == -1) {free(link);return -1;}
    else
        if(iflink)
            strcpy(filename2,link);
        else
            strcpy(filename2,filename);
    free(link);
  

    char* path = malloc(MAX_PATH_SIZE);
    char* name = malloc(51);
    DividePathAndFile(filename2, path, name);
    free(filename2);
    DWORD cluster = FindFile(path);
    free(path);
    if(cluster == -1) {free(name);return -1;} //arquivo não encontrado
   
    if(NextCluster(cluster) == 0xFFFFFFFE) {free(name);return -1;} //corrompido


    struct t2fs_record* entrada = SearchEntradas(cluster, name);
    free(name);
    if(entrada == NULL) return -1;

    int i = 0;

    while(i < MAX_HANDLE && FilesHandle[i].file_data != NULL){
        i++;
    }
    if(i == MAX_HANDLE) return -1;

    FilesHandle[i] = (struct FilesOpen){.handle = i, .directory_cluster = cluster, .file_data = entrada, .CP = 0};

    return i;

}


int close2 (FILE2 handle){
    free(FilesHandle[handle].file_data);
    FilesHandle[handle] = (struct FilesOpen){.handle = -1, .file_data = NULL, .CP = 0};

    return 0;
}


int read2 (FILE2 handle, char *buffer, int size) {
    
    struct FilesOpen filesOpen = FilesHandle[handle];
    struct t2fs_superbloco superbloco = ReadSuperbloco();
    struct t2fs_record *fileRecord = filesOpen.file_data;

    if (NextCluster(fileRecord->firstCluster) == 0xFFFFFFFE) { return -1; } //corrompido
    
    unsigned int currentPointSectorOffset = filesOpen.CP % SECTOR_SIZE;
    unsigned int bufferBeginning = currentPointSectorOffset;
   
    // Calcula o ceiling de size / SECTOR_SIZE.
    unsigned int numSectorsToRead = (size / SECTOR_SIZE) + ((size % SECTOR_SIZE) != 0);
    
    if (currentPointSectorOffset != 0) {	
	bufferBeginning = currentPointSectorOffset;
    }

    unsigned int sizeWithoutCurrentPoint = currentPointSectorOffset + size;

  
    BYTE *tmpBuffer = malloc(sizeof(BYTE) * numSectorsToRead * SECTOR_SIZE);
    if (tmpBuffer == 0) { return -2; }
    
    DWORD currentSector = FindFileOffsetSector(fileRecord, filesOpen.CP);
    DWORD currentCluster = ((currentSector - superbloco.DataSectorStart) / superbloco.SectorsPerCluster);
    
    if(NextCluster(currentCluster) == 0xFFFFFFFE) { return -3; } //corrompido

    unsigned int sectorCounter = 0;
    unsigned int bytesRead = 0;
    unsigned int i;
    
    for (i = 0; i < numSectorsToRead; i++)
    {
	if (read_sector(currentSector, tmpBuffer + (i * SECTOR_SIZE))) {
	    free(tmpBuffer);
	    return -4;
	}

	bytesRead = bytesRead + SECTOR_SIZE;
	sectorCounter++;
	
	// Vai pro próximo cluster.
	if (sectorCounter >= superbloco.SectorsPerCluster) {
	    if(NextCluster(currentCluster) == 0xFFFFFFFE) { return -5; } //corrompido
	    currentCluster = NextCluster(currentCluster);
	    
	    currentSector = SetorLogico_ClusterDados(currentCluster);
	    sectorCounter = 0;
	}

	// Chegou no final do arquivo.
	if (currentCluster == 0xFFFFFFFF) {
	    break;
	}

    }

    unsigned int sizeCopy;
    if (sizeWithoutCurrentPoint + filesOpen.CP <= fileRecord->bytesFileSize) {
	sizeCopy = sizeWithoutCurrentPoint - filesOpen.CP;
    } else {
	sizeCopy = fileRecord->bytesFileSize - filesOpen.CP;
    }

    if (sizeCopy > 0) {
	memcpy(buffer, (tmpBuffer + bufferBeginning), sizeCopy);	
    }
    
    free(tmpBuffer);
    bytesRead = sizeCopy;
    
    FilesHandle[handle].CP = bytesRead +  FilesHandle[handle].CP;
    
    return bytesRead;
}




int write2 (FILE2 handle, char *buffer, int size) {

    struct FilesOpen filesOpen = FilesHandle[handle];

    if (filesOpen.file_data == NULL) {
	return -11;
    }
    
    struct t2fs_record *fileRecord = filesOpen.file_data;
    struct t2fs_superbloco superblock = ReadSuperbloco();

    unsigned int sizeWithoutCurrentPointer = (size + filesOpen.CP) % SECTOR_SIZE;
    unsigned int currentPointerSectorDistance = filesOpen.CP % SECTOR_SIZE;
    
    unsigned int numSectorsToWrite = ((size + sizeWithoutCurrentPointer) / SECTOR_SIZE) + ((size + sizeWithoutCurrentPointer) % SECTOR_SIZE != 0);

    BYTE *firstSectorBuffer = malloc(sizeof(BYTE) * SECTOR_SIZE);
    BYTE *lastSectorBuffer = malloc(sizeof(BYTE) * SECTOR_SIZE);
    

    DWORD currentPointerSector = FindFileOffsetSector(fileRecord, filesOpen.CP);
    unsigned int bytesWritten = 0;
    
    DWORD currentSector = currentPointerSector;
    DWORD currentCluster = (currentSector - superblock.DataSectorStart) / superblock.SectorsPerCluster;

    if (NextCluster(currentCluster) == 0xFFFFFFFE) {
	free(firstSectorBuffer);
	free(lastSectorBuffer);
	return -1;
    }
    unsigned int sectorCounter = currentPointerSector % superblock.SectorsPerCluster;
    
    // Aloca clusters se faltar espaço.
    unsigned int finalFilesize;
    if (filesOpen.CP == fileRecord->bytesFileSize) {
	finalFilesize = fileRecord->bytesFileSize + size;
    } else {
	if (filesOpen.CP + size > fileRecord->bytesFileSize) {
	    finalFilesize = fileRecord->bytesFileSize + ((filesOpen.CP + size) - fileRecord->bytesFileSize);
	} else {
	    finalFilesize = fileRecord->bytesFileSize;
	}
    }
    
     if (finalFilesize > fileRecord->bytesFileSize) {
	unsigned int clusterSize = SECTOR_SIZE * superblock.SectorsPerCluster;
	unsigned int numAllocatedClusters = fileRecord->clustersFileSize;

	unsigned int numClustersWithWrite;

	if (finalFilesize != 0) {
	    numClustersWithWrite = (finalFilesize / clusterSize) + (finalFilesize % clusterSize != 0);
	} else {
	    numClustersWithWrite = 1;
	}

	fileRecord->clustersFileSize = numClustersWithWrite;

	int numClustersToAllocate = numClustersWithWrite - numAllocatedClusters;
	DWORD lastCluster = FindLastCluster(fileRecord->firstCluster);

	int i;
	for (i = 0; i < numClustersToAllocate; i++) {
	    int newCluster = OccupyFreeCluster();
	    UpdateFatEntry(lastCluster, newCluster);
	    lastCluster = newCluster;
	}

	// Marca último cluster como EOF.
	UpdateFatEntry(lastCluster, 0xFFFFFFFF);
		
    } else {
	finalFilesize = fileRecord->bytesFileSize;
    }

    // Escreve o primeiro setor, lendo ele antes se o current pointer não estiver no começo do setor.
    if (currentPointerSectorDistance != 0 || size < SECTOR_SIZE) {
	if (read_sector(currentPointerSector, firstSectorBuffer) != 0) {
	    free(firstSectorBuffer);
	    free(lastSectorBuffer);
	    return -2;
	}

	unsigned int bufferCopySize;
	if (size + currentPointerSectorDistance > SECTOR_SIZE) {
	    bufferCopySize = SECTOR_SIZE - currentPointerSectorDistance;
	} else {
	    bufferCopySize = size;
	}


	memcpy(firstSectorBuffer + currentPointerSectorDistance, buffer, bufferCopySize);

	if (write_sector(currentPointerSector, firstSectorBuffer) != 0) {
	    free(firstSectorBuffer);
	    free(lastSectorBuffer);
	    return -3;
	}

	bytesWritten = bytesWritten + bufferCopySize;
    } else {
	if (write_sector(currentPointerSector, (BYTE*)(buffer)) != 0) {
	    free(firstSectorBuffer);
	    free(lastSectorBuffer);
	    return -4;
	}
	
	bytesWritten = bytesWritten + SECTOR_SIZE;
    }
    
    sectorCounter++;
    currentSector++;
	
    if (sectorCounter >= superblock.SectorsPerCluster) {
	currentCluster = NextCluster(currentCluster);
	if(NextCluster(currentCluster) == 0xFFFFFFFE) {
	    free(firstSectorBuffer);
	    free(lastSectorBuffer);
	    return -5;
	}
	sectorCounter = 0;
    }

    unsigned int i;
    // Escreve os setores do "meio".
    for (i = 1; i < numSectorsToWrite - 1; i++) {

	if (write_sector(currentSector, (BYTE*)(buffer) + (i * SECTOR_SIZE)) != 0) {
	    free(firstSectorBuffer);
	    free(lastSectorBuffer);
	    return -6;
	}
	
	sectorCounter++;
	currentSector++;
	bytesWritten = bytesWritten + SECTOR_SIZE;
	
	if (sectorCounter >= superblock.SectorsPerCluster) {
	    currentCluster = NextCluster(currentCluster);

	    if(NextCluster(currentCluster) == 0xFFFFFFFE) {
		free(firstSectorBuffer);
		free(lastSectorBuffer);
		return -7;
	    }
	    
	    sectorCounter = 0;
	}

	if (currentCluster == 0xFFFFFFFF) {
	    break;
	}
    }
  
    // Lê e depois escreve se o final da escrita não completar um setor.
     if (numSectorsToWrite > 1) {
	 int bufferOffset = ((numSectorsToWrite - 1) * SECTOR_SIZE);

	 if (sizeWithoutCurrentPointer % SECTOR_SIZE != 0) {
	     if (read_sector(currentSector, lastSectorBuffer) != 0) {
		 free(firstSectorBuffer);
		 free(lastSectorBuffer);
		 return -8;
	     }

	     memcpy(lastSectorBuffer, buffer + bufferOffset, sizeWithoutCurrentPointer % SECTOR_SIZE);
	 
	     if (write_sector(currentSector, lastSectorBuffer) != 0) {
		 free(firstSectorBuffer);
		 free(lastSectorBuffer);
		 return -9;
	     }

	     bytesWritten = bytesWritten + (sizeWithoutCurrentPointer % SECTOR_SIZE);
	 } else {
	     if (write_sector(currentSector, (BYTE*)(buffer + bufferOffset)) != 0) {
		 free(firstSectorBuffer);
		 free(lastSectorBuffer);
		 return -10;
	     }

	     bytesWritten = bytesWritten + SECTOR_SIZE;
	 }
	 
	 
     }
     

     
     free(firstSectorBuffer);
     free(lastSectorBuffer);
     
     FilesHandle[handle].CP = filesOpen.CP + bytesWritten;
     fileRecord->bytesFileSize = finalFilesize;
     
     
     UpdateDirEntry(FilesHandle[handle].directory_cluster, FilesHandle[handle].file_data);
     
     return bytesWritten;
}
    
int truncate2 (FILE2 handle) {

    struct t2fs_superbloco superblock = ReadSuperbloco();
    struct FilesOpen filesOpen = FilesHandle[handle];
    struct t2fs_record *fileRecord = filesOpen.file_data;

    
    DWORD currentPointerSector = FindFileOffsetSector(fileRecord, filesOpen.CP) + 1;
    DWORD currentCluster = ((currentPointerSector - superblock.DataSectorStart) / superblock.SectorsPerCluster);
    
    if(NextCluster(currentCluster) == 0xFFFFFFFE) { return -1; } //corrompido
    DWORD sectorCounter = (currentPointerSector % superblock.SectorsPerCluster) + 1;
    DWORD lastCluster;


    if (sectorCounter >= superblock.SectorsPerCluster) {
	sectorCounter = 0;
	   
	if(NextCluster(currentCluster) == 0xFFFFFFFE) { return -2; }  //corrompido
	currentCluster = NextCluster(currentCluster);
    }
    while (currentCluster != 0xFFFFFFFF) {

	sectorCounter++;

	if (sectorCounter >= superblock.SectorsPerCluster) {
	    sectorCounter = 0;

	    lastCluster = currentCluster;
	    currentCluster = NextCluster(currentCluster);
	    
	    if(currentCluster == 0xFFFFFFFE) { return -2; } //corrompido

	}

	UpdateFatEntry(lastCluster, 0);
    }

    // Atualiza último cluster.
    UpdateFatEntry(currentCluster, 0);
    
    fileRecord->bytesFileSize = filesOpen.CP;
    filesOpen.CP = filesOpen.CP - 1;

    if (UpdateDirEntry(FilesHandle[handle].directory_cluster, FilesHandle[handle].file_data) < 0) {
	return -3;
    }
    
    return 0;
}
   
int seek2 (FILE2 handle, DWORD offset) {

    struct FilesOpen filesOpen = FilesHandle[handle];
    struct t2fs_record *fileRecord = filesOpen.file_data;
    
    if (offset == -1) {
	FilesHandle[handle].CP = 0;
    } else if (offset >= 0){
	if (offset >= fileRecord->bytesFileSize) {
	    FilesHandle[handle].CP = fileRecord->bytesFileSize - 1;
	} else {
	    FilesHandle[handle].CP = offset;	    
	}

	
    // Deslocamento negativo gera um erro.
    } else {
	return -1;
    }

    return 0;
}

int mkdir2 (char *pathname){
   
   if(FindFile(pathname) != -1)//nome de dir/arquivo ja existente
        return -1;
   char* path = malloc(MAX_PATH_SIZE);
   char* name = malloc(51);
   DividePathAndFile(pathname,path,name);
   
   DWORD dir_cluster = FindFile(path);
   free(path);
   if(dir_cluster == -1) {free(name);return -1;}
   if(NextCluster(dir_cluster) == 0xFFFFFFFE) {free(name);return -1;} //corrompido
   //define a entrada do novo dir
   BYTE* entrada = malloc(64);
   entrada[0] = TYPEVAL_DIRETORIO;
   int i;
   for(i=0;i<51;i++)entrada[i+1] = name[i];
      free(name);
   
   struct t2fs_superbloco superbloco  = ReadSuperbloco();
        //bytesFileSize
   DWORD bytesFileSize = 256*superbloco.SectorsPerCluster;
   entrada[52] = bytesFileSize;
   entrada[53] =(bytesFileSize/16)/16;
   entrada[54] = ((((bytesFileSize/16)/16)/16)/16);
   entrada[55] =((((((bytesFileSize/16)/16)/16)/16)/16)/16); 
        //ClusterFileSize
   entrada[56] = 0X01;
   entrada[57] = 0X00;
   entrada[58] = 0X00;
   entrada[59] = 0X00;
   DWORD clusterfree = OccupyFreeCluster();//entrada FAT
   entrada[60] = clusterfree;
   entrada[61] =(clusterfree/16)/16;
   entrada[62] = ((((clusterfree/16)/16)/16)/16);
   entrada[63] =((((((clusterfree/16)/16)/16)/16)/16)/16);
    
   //escreve a entrada no dir pai
   if(WriteInEmptyEntry(dir_cluster,entrada) == -1){free(name);free(path);free(entrada);return -1;}
   //inicia o dir com '.' e '..'
   if(StartNewDir(clusterfree, entrada, dir_cluster) == -1) {free(entrada);return -1;}
   

    free(entrada);

   return 0;
}
   
int rmdir2 (char *pathname)
{

    char* link = malloc(MAX_PATH_SIZE);
    char* pathname2 = malloc(MAX_PATH_SIZE);
    int iflink = checkiflink(pathname,link);
    if(iflink == -1) {free(link);return -1;}
    else
        if(iflink)
            strcpy(pathname2,link);
        else
            strcpy(pathname2,pathname);
    free(link);

    DWORD cluster = FindFile(pathname2);
    if(cluster == -1) return -1;//ERROR LOOKING FOR
    if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
    if(!CheckIfDirAndEmpty(cluster)) return -1;// not empty or not a dir
    
    //APAGA ENTRADA
    char* path = malloc(MAX_PATH_SIZE);
    char* name = malloc(51);
    DividePathAndFile(pathname2, path, name);
    free(pathname2);  
    EraseEntry(path,name);
    free(path);
    free(name);
    BYTE* buffer = malloc(SECTOR_SIZE);
    
    //APAGA DIRETORIO
    struct t2fs_superbloco superbloco  = ReadSuperbloco();
    int k;
    for(k = 0; k < 256;k++) buffer[k] = '\0';
    for(k=0;k < superbloco.SectorsPerCluster;k++)
        if(write_sector(SetorLogico_ClusterDados(cluster)+k,buffer)) {free(buffer);return -1;}
        
    //APAGA NA FAT
    DWORD sector_cluster = cluster/64 + superbloco.pFATSectorStart;
    
    if(read_sector(sector_cluster,buffer)){free(buffer);return -1;}
    DWORD pos_atual = (cluster - 64*(sector_cluster-1))*4;
    buffer[pos_atual] = 0;
    buffer[pos_atual+1] = 0;
    buffer[pos_atual+2] = 0;
    buffer[pos_atual+3] = 0;
    if(write_sector(sector_cluster,buffer)){free(buffer);return -1;}
    
    free(buffer);
    return 0;


}
  

int chdir2 (char *pathname)
{
    char* link = malloc(MAX_PATH_SIZE);
    char* pathname2 = malloc(MAX_PATH_SIZE);
    int iflink = checkiflink(pathname,link);
    if(iflink == -1) {free(link);return -1;}
    else
        if(iflink)
            strcpy(pathname2,link);
        else
            strcpy(pathname2,pathname);
    free(link);
    
    DWORD cluster = FindFile(pathname2); 
    if(cluster == -1) return -1; //DIR DOES NOT EXIST
    if(NextCluster(cluster) == 0xFFFFFFFE) return -1; //corrompido
    BYTE* buffer = malloc(256);
    if(read_sector(SetorLogico_ClusterDados(cluster), buffer)){free(buffer);return -1;}
    if(buffer[0] != TYPEVAL_DIRETORIO || buffer[1] != '.')
        {free(buffer);return -1;} //NOT A DIR
    free(buffer);
    
    
    if(pathname2[0] == '/') //absoluto
        strcpy(workingDir,pathname2);
    else
    {
        if(pathname2[0] == '.' && pathname2[1] == '.')  // relativo pai
        {
            while(strlen(workingDir) > 1 && workingDir[strlen(workingDir)-1] == '/') //tira qualquer '/' do final
                workingDir[strlen(workingDir)-1] = '\0';
            while(strlen(workingDir) > 1 && workingDir[strlen(workingDir)-1] != '/') //pega path do pai
                workingDir[strlen(workingDir)-1] = '\0';
            
            if(strlen(workingDir) == 1 && strlen(pathname2) != 2)
                strcat(workingDir,pathname2+3);
            else
                strcat(workingDir,pathname2+2);
        }
        else  //relativo CWD
        {
            while(strlen(workingDir) > 1 && workingDir[strlen(workingDir)-1] == '/') //tira qualquer '/' do final
                workingDir[strlen(workingDir)-1] = '\0';
            if(pathname2[0] == '.')
		strcat(workingDir,pathname2+1);//copia tirando o '.'
            else
		strcat(workingDir,pathname2);
        }
    }
    free(pathname2);
    
    return 0;

}

int getcwd2 (char *pathname, int size)
{
    int len = strlen(workingDir);
    if(size < len) return -1;
    
    strcpy(pathname,workingDir);
    return 0;


}

DIR2 opendir2 (char *pathname)
{
    char* link = malloc(MAX_PATH_SIZE);
    char* pathname2 = malloc(MAX_PATH_SIZE);
    int iflink = checkiflink(pathname,link);
    if(iflink == -1) {free(link);return -1;}
    else
        if(iflink)
            strcpy(pathname2,link);
        else
            strcpy(pathname2,pathname);
    free(link);
    
    DWORD file_cluster = FindFile(pathname2);
    free(pathname2);
    if(file_cluster == -1) return -1; //ERROR
    if(NextCluster(file_cluster) == 0xFFFFFFFE) return -1; //corrompido
    int i=0;
    
    while(i < MAX_HANDLE && DirsHandle[i].first_cluster != 0)
        i++;
    
    if(i == MAX_HANDLE) return -1;
    DirsHandle[i] = (struct DirsOpen){.handle = i,.first_cluster = file_cluster, .CE = 0}; 
    
    
    return i;
    
}


int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     if(NextCluster(DirsHandle[handle].first_cluster) == 0xFFFFFFFE) return -1; //corrompido
     DWORD firstSector = SetorLogico_ClusterDados(DirsHandle[handle].first_cluster); 
     //Get dir size
     if(read_sector(firstSector ,buffer2)) {free(buffer2);return -1;} //ERROR
     
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     
     if(file_size/64 <= DirsHandle[handle].CE) {free(buffer2);return -2;} //END OF FILE
     
    DWORD sector_entrada = firstSector + DirsHandle[handle].CE/4;    
    DWORD pos_atual = (DirsHandle[handle].CE - 4*(sector_entrada - firstSector))*64; 
    
    if(read_sector(sector_entrada, buffer2)) {free(buffer2);return -1;} //ERROR
    int i;
     for(i = 0;i<256;i++)
        dentry->name[i]=buffer2[pos_atual+i+1];
        
     dentry->fileType = buffer2[pos_atual];
     
     dentry->fileSize = buffer2[pos_atual+52] + buffer2[pos_atual+53]*16*16 + buffer2[pos_atual+54]*16*16*16*16 + buffer2[pos_atual+55]*16*16*16*16*16*16;
    
    DirsHandle[handle].CE++; 
    free(buffer2);
    return 0;
}


int closedir2 (DIR2 handle)
{
    DirsHandle[handle] = (struct DirsOpen){.handle = -1,.first_cluster = 0, .CE = 0};

    return 0;
}

int ln2(char *linkname, char *filename){

    //checa se o original existe
     DWORD file_cluster = FindFile(filename); 
    if(file_cluster == -1) return -1; //arquivo não encontrado

    //checa se o path onde o link será criado é válido
    char* link_path = malloc(MAX_PATH_SIZE);
    char* link_name = malloc(51);
    DividePathAndFile(linkname, link_path, link_name);
    DWORD link_cluster = FindFile(link_path);
    free(link_path);
    if(link_cluster == -1) {free(link_name);return -1;} //path não encontrado
    //checa se já não existe outro link com o mesmo nome
    if(FindFile(linkname) != -1) return -1; //nome de dir/link ja existente

    //define a entrada do novo link
    BYTE* entrada = malloc(sizeof(struct t2fs_record));
    entrada[0] = TYPEVAL_LINK;
    int i = 0;
    while(link_name[i] != '\0')
        {entrada[i+1] = link_name[i]; i++;}
    free(link_name);
    int j;
    for(j = i;j<51;j++)entrada[j+1] = '\0';

        //bytesFileSize
   DWORD bytesFileSize = strlen(linkname);
   entrada[52] = bytesFileSize;
   entrada[53] =(bytesFileSize/16)/16;
   entrada[54] = ((((bytesFileSize/16)/16)/16)/16);
   entrada[55] =((((((bytesFileSize/16)/16)/16)/16)/16)/16); 
        //ClusterFileSize
   entrada[56] = 0X01;
   entrada[57] = 0X00;
   entrada[58] = 0X00;
   entrada[59] = 0X00;
   DWORD clusterfree = OccupyFreeCluster();//entrada FAT
   entrada[60] = clusterfree;
   entrada[61] =(clusterfree/16)/16;
   entrada[62] = ((((clusterfree/16)/16)/16)/16);
   entrada[63] =((((((clusterfree/16)/16)/16)/16)/16)/16);

   free(entrada);


   char* workingDirAux = malloc(MAX_PATH_SIZE);
   unsigned char* buffer = malloc(MAX_PATH_SIZE);

   strcpy(workingDirAux, workingDir);

   if(linkname[0] == '/'){ //absoluto
   i = 0;
        while(link_path[i] != '\0')
        {buffer[i] = link_path[i]; i++;}
        buffer[i] = '\0';
   }
    else
    {
        if(linkname[0] == '.' && linkname[1] == '.')  // relativo pai
        {
            while(strlen(workingDirAux) > 1 && workingDirAux[strlen(workingDirAux)-1] == '/') //tira qualquer '/' do final
                workingDirAux[strlen(workingDirAux)-1] = '\0';
            while(strlen(workingDirAux) > 1 && workingDirAux[strlen(workingDirAux)-1] != '/') //pega path do pai
                workingDirAux[strlen(workingDirAux)-1] = '\0';
            
            if(strlen(workingDirAux) == 1 && strlen(linkname) != 2)
                strcat(workingDirAux,linkname+3);
            else
                strcat(workingDirAux,linkname+2);
        }
        else  //relativo CWD
        {
            while(strlen(workingDirAux) > 1 && workingDirAux[strlen(workingDirAux)-1] == '/') //tira qualquer '/' do final
                workingDirAux[strlen(workingDirAux)-1] = '\0';
            if(linkname[0] == '.')
		strcat(workingDirAux,linkname+1);//copia tirando o '.'
            else
		strcat(workingDirAux,linkname);
        }
         i = 0;
        while(link_path[i] != '\0')
        {buffer[i] = link_path[i]; i++;}
        buffer[i] = '\0';
    }
    free(linkname);

    DWORD sector_cluster = SetorLogico_ClusterDados(clusterfree);
    
    if(write_sector(sector_cluster,buffer)){free(buffer);return -1;}
    
    free(buffer);
    return 0;


}
