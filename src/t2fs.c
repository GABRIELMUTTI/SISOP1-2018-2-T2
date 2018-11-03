#include "../include/t2fs.h"
#include "../include/aux.h"


struct DirsOpen {
    
    DIR2 handle;
    DWORD first_cluster;
    int CE;

};

struct FilesOpen {

    struct t2fs_record *file_data;
    int CP;
};

struct DirsOpen DirsHandle[10];
int handDirCont = 0;

int main(int argc, char *argv[]){

    //char* nome = malloc(51);
    //nome = "/dir1/file1.txt\0";
    
    //printf("%X\n", mkdir2(nome));
    return 0;
}


int identify2 (char *name, int size);


FILE2 create2 (char *filename);


int delete2 (char *filename);
    
FILE2 open2 (char *filename);


int close2 (FILE2 handle);


int read2 (FILE2 handle, char *buffer, int size);


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
