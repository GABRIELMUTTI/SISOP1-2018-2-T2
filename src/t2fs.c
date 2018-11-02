#include "../include/t2fs.h"
#include "../include/aux.h"


struct openDir {
    
    DIR2 handle;
    DWORD firstSector;
    DWORD cluster;
    int CE;

};

struct openDir DirsHandle[10];

int main(int argc, char *argv[]){

    char* nome = malloc(51);
    nome = "/file1.txt\0";
    printf("%X\n", FindFile(nome));
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

int mkdir2 (char *pathname);
   //ler superbloco
   //read root
   //for dirs in pathname
   //   search current dir for dir location
   //   read dir
   //   current dir = dir
   //
   //ocupar entrada de diretorio
   //alocar um cluster
   //Além disso, deve‐se acrescentar no diretório recém criado 
   //entradas para os diretórios “.”  ponto) e “..” (ponto‐ponto)
   
int rmdir2 (char *pathname);
  

int chdir2 (char *pathname);


int getcwd2 (char *pathname, int size);


DIR2 opendir2 (char *pathname);



int readdir2 (DIR2 handle, DIRENT2 *dentry)
{
     BYTE* buffer2 = malloc(SECTOR_SIZE);
     
     //Get dir size
     if(read_sector(DirsHandle[handle].firstSector ,buffer2)) {free(buffer2);return -1;} //ERROR
     
     DWORD file_size = buffer2[52] + buffer2[53]*16*16 + buffer2[54]*16*16*16*16 + buffer2[55]*16*16*16*16*16*16;
     
     if(file_size/64 <= DirsHandle[handle].CE) {free(buffer2);return -2;} //END OF FILE
     
    DWORD sector_entrada = DirsHandle[handle].firstSector + DirsHandle[handle].CE/4;    
    DWORD pos_atual = (DirsHandle[handle].CE - 4*(sector_entrada - DirsHandle[handle].firstSector))*64; 
    
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
