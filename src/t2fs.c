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
int handDirCont = 0;

int main(int argc, char *argv[]){
    
    char name[51] = "/dir24\0";
    char* namep = malloc(255);
    int i;
    for(i = 0; i < 51; i++ )namep[i] = name[i];
    printf("%X\n",mkdir2(namep));


    printf("%X\n",rmdir2(namep));
    
    
    free(namep);
    
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
   
   if(FindFile(pathname) != -1)//nome de dir/arquivo ja existente
        return -1;
   char* path = malloc(255);
   char* name = malloc(51);
   DividePathAndFile(pathname,path,name);
   
   DWORD dir_cluster = FindFile(path);
   free(path);
   if(dir_cluster == -1) {free(name);return -1;}
   
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
    DWORD cluster = FindFile(pathname);
    if(cluster == -1) return -1;//ERROR LOOKING FOR

    if(!CheckIfDirAndEmpty(cluster)) return -1;// not empty or not a dir
    
    //APAGA ENTRADA
    char* path = malloc(255);
    char* name = malloc(51);
    DividePathAndFile(pathname, path, name);  
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
