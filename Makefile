SRC=src
INC=include
OBJ=objects
BIN=bin

CC=gcc
CFLAGS=-I$(INC) -ggdb -O0 -Wall -pedantic -ansi -Wextra -fno-omit-frame-pointer

$(BIN)/int: $(OBJ)/help.o $(OBJ)/eval.o $(OBJ)/var.o $(SRC)/int.c
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/eval.o: $(SRC)/eval.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJ)/var.o: $(SRC)/var.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJ)/help.o: $(SRC)/help.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJ)/oper.o: $(SRC)/oper.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: all clean

all:
	make $(BIN)/int

clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*
