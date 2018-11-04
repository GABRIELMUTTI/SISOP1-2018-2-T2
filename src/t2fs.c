#include "../include/t2fs.h"
#include "../include/aux.h"


struct DirsOpen {
    
    DIR2 handle;
    DWORD first_cluster;
    int CE;

};

struct FilesOpen {

    DIR2 handle;
    struct t2fs_record *file_data;
    int CP;
};

struct DirsOpen DirsHandle[10];
struct FilesOpen FilesHandle[10];

int handDirCont = 0;

int main(int argc, char *argv[]){
    struct t2fs_superbloco superbloco = ReadSuperbloco();

    char *buffer = malloc(sizeof(char) * 55);

    struct t2fs_record *fileRecord = malloc(sizeof(struct t2fs_record));
    ReadEntrada(137, 2, fileRecord);

    struct FilesOpen *filesOpen = malloc(sizeof(struct FilesOpen));
    filesOpen->handle = 0;
    filesOpen->file_data = fileRecord;
    filesOpen->CP = 0;
    
    FilesHandle[0] = *filesOpen;

  
    int status = read2(0, buffer, 21);

    
    printf("Status: %d\nValor: %s\n", status, buffer);
    status = read2(0, buffer, 34);

    printf("WithCP:\nStatus: %d\nValor: %s\n", status, buffer);
    

    
    
    return 0;

}


int identify2 (char *name, int size);


FILE2 create2 (char *filename);


int delete2 (char *filename);
    
FILE2 open2 (char *filename);


int close2 (FILE2 handle);


int read2 (FILE2 handle, char *buffer, int size) {
    
    struct FilesOpen filesOpen = FilesHandle[handle];
    struct t2fs_superbloco superbloco = ReadSuperbloco();
    struct t2fs_record *fileRecord = filesOpen.file_data;

    unsigned int currentPointSectorOffset = filesOpen.CP % SECTOR_SIZE;
    unsigned int bufferBeginning = 0;
    unsigned int bufferEnding = SECTOR_SIZE;

    // Calcula o ceiling de size / SECTOR_SIZE.
    unsigned int numSectorsToRead = (size / SECTOR_SIZE) + ((size % SECTOR_SIZE) != 0);
    
    if (currentPointSectorOffset != 0) {	
	bufferBeginning = currentPointSectorOffset;
    }

    unsigned int sizeWithoutCurrentPoint = currentPointSectorOffset + size;
    if (sizeWithoutCurrentPoint % SECTOR_SIZE != 0) {
	bufferEnding = sizeWithoutCurrentPoint;
    }


    char *tmpBuffer = malloc(sizeof(char) * numSectorsToRead * SECTOR_SIZE);

    DWORD firstSector = SetorLogico_ClusterDados(fileRecord->firstCluster);
    DWORD currentSector = (filesOpen.CP + (firstSector * SECTOR_SIZE)) / SECTOR_SIZE;
    DWORD currentCluster = (currentSector / superbloco.SectorsPerCluster);
    
    unsigned int sectorCounter = 0;
    unsigned int bytesRead = 0;
    unsigned int i;
    
    for (i = 0; i < numSectorsToRead; i++)
    {
	int status = read_sector(currentSector, tmpBuffer);

	if (status != 0) {
	    return -1;
	}

	bytesRead = bytesRead + SECTOR_SIZE;


	sectorCounter = sectorCounter + 1;
	
	// Vai pro próximo cluster.
	if (sectorCounter >= superbloco.SectorsPerCluster) {
	    currentCluster = NextCluster(currentCluster);
	    currentSector = SetorLogico_ClusterDados(currentCluster);
	    sectorCounter = 0;
	}

	// Chegou no final do arquivo.
	if (currentCluster == 0xFFFFFFFF) {
	    break;
	}

    }

    memcpy(buffer, (tmpBuffer + bufferBeginning), bufferEnding);
    free(tmpBuffer);

    bytesRead = bytesRead - currentPointSectorOffset - (SECTOR_SIZE - (sizeWithoutCurrentPoint % SECTOR_SIZE));

    FilesHandle[handle].CP = bytesRead +  FilesHandle[handle].CP;
    
    return bytesRead;
}


int write2 (FILE2 handle, char *buffer, int size);
    
int truncate2 (FILE2 handle);
   
int seek2 (FILE2 handle, DWORD offset);

int mkdir2 (char *pathname){
   
   char* path = malloc(51);
   char* name = malloc(51);
   DividePathAndFile(pathname,path,name);
   
   DWORD dir_cluster = FindFile(path);
   if(dir_cluster == -1) return -1;
   
   struct t2fs_record entrada = (struct t2fs_record){.TypeVal = TYPEVAL_DIRETORIO,.bytesFileSize = 1024,.clustersFileSize = 1};
   int i;
   for(i=0;i<51;i++)entrada.name[i] = name[i];
   
   
   free(path);
   free(name);
   
   //ocupar entrada de diretorio
   //alocar um cluster
   //Além disso, deve‐se acrescentar no diretório recém criado 
   //entradas para os diretórios “.”  ponto) e “..” (ponto‐ponto)
   return 0;
   }
   
int rmdir2 (char *pathname);
  

int chdir2 (char *pathname);


int getcwd2 (char *pathname, int size);


DIR2 opendir2 (char *pathname)
{
    DWORD file_cluster = FindFile(pathname);
    if(file_cluster == -1) return -1; //ERROR
    
    DirsHandle[handDirCont] = (struct DirsOpen){.handle = handDirCont,.first_cluster = file_cluster, .CE = 0}; 
    
    
    return handDirCont++;
    
}


int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     
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


int closedir2 (DIR2 handle);


int ln2(char *linkname, char *filename);
