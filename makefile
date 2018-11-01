#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src


all: compilacao ligacao


compilacao: $(SRC_DIR)/t2fs.c
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o -Wall

ligacao:
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(LIB_DIR)/apidisk.o 

test1:$(SRC_DIR)/aux.c $(SRC_DIR)/t2fs.c
	$(CC) -c $(SRC_DIR)/aux.c -o $(BIN_DIR)/aux.o -Wall
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o -Wall

test2:$(BIN_DIR)/aux.o $(LIB_DIR)/apidisk.o $(BIN_DIR)/t2fs.o
	$(CC) -o test $(BIN_DIR)/aux.o $(LIB_DIR)/apidisk.o $(BIN_DIR)/t2fs.o

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~


