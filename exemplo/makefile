CC=gcc
LIB_DIR=../lib


all: compilacao


compilacao: main.c $(LIB_DIR)/libt2fs.a
	$(CC) -o main main.c -L$(LIB_DIR) -lt2fs -Wall 

again:
	cd .. &&	make   
	make 
	
clean:
	rm -rf *~ main
