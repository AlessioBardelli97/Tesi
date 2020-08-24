#!/bin/bash

TARGET1  = generalized_test_Filippo
SRC1     = $(TARGET1).c
BIN1     = $(TARGET1).out
OUTFILE1 = output.$(TARGET1).txt

TARGET2  = generalized_test_Alessio
SRC2     = $(TARGET2).c
BIN2     = $(TARGET2).out
OUTFILE2 = output.$(TARGET2).txt

ARGS = pla/wim.pla

OBJ    = parser.o autosymmetry.o equations.o binmat.o logic.o

ccflags = -std=c99 #-Wall
debug   = -g #-DDEBUG

all: $(BIN1) $(BIN2)

$(BIN1): $(SRC1) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC1) $(OBJ) -o $@ -lcudd -lm
	
$(BIN2): $(SRC2) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC2) $(OBJ) -o $@ -lcudd -lm

%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $<
	
filippo: $(BIN1)
	./$(BIN1) $(ARGS) > $(OUTFILE1)
	
alessio: $(BIN2)
	./$(BIN2) $(ARGS) > $(OUTFILE2)

run: alessio filippo
	
valgrind: $(BIN1) $(BIN2)
	valgrind ./$(BIN2) $(ARGS) > $(OUTFILE2) 2>&1
	
clean:
	rm -f S.pla $(OUTFILE1) $(OUTFILE2)
	
cleanall: clean
	rm -f *.o $(BIN1) $(BIN2)

