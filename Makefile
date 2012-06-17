SRC=src
INC=include
OBJ=objects
BIN=bin

CC=gcc
CFLAGS=-I$(INC) -ggdb -O0 -Wall

$(BIN)/int: $(OBJ)/help.o $(OBJ)/eval.o $(OBJ)/var.o $(SRC)/int.c
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/eval.o: $(SRC)/eval.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJ)/var.o: $(SRC)/var.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJ)/help.o: $(SRC)/help.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(OBJ)/*
	rm -f $(BIN)/*
