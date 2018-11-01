#include "../include/t2fs.h"
#include "../include/aux.h"



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


int readdir2 (DIR2 handle, DIRENT2 *dentry);


int closedir2 (DIR2 handle);


int ln2(char *linkname, char *filename);
