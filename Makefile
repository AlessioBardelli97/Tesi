#!/bin/bash

TARGET1  = generalized_test_Filippo
SRC1     = $(TARGET1).c
BIN1     = $(TARGET1).out
OUTFILE1 = output.$(TARGET1).txt

TARGET2  = generalized_test_Alessio
SRC2     = $(TARGET2).c
BIN2     = $(TARGET2).out
OUTFILE2 = output.$(TARGET2).txt

TARGET3  = generalized_test_Ottimo
SRC3     = $(TARGET3).c
BIN3     = $(TARGET3).out
OUTFILE3 = output.$(TARGET3).txt

ARGS = pla/dk17.pla

OBJ = parser.o autosymmetry.o equations.o binmat.o logic.o

ccflags = -std=c99 #-Wall
debug   = -g #-DDEBUG

all: $(BIN1) $(BIN2) $(BIN3)

$(BIN1): $(SRC1) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC1) $(OBJ) -o $@ -lcudd -lm
	
$(BIN2): $(SRC2) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC2) $(OBJ) -o $@ -lcudd -lm
	
$(BIN3): $(SRC3) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC3) $(OBJ) -o $@ -lcudd -lm

%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $<
	
filippo: $(BIN1)
	timeout 10m ./$(BIN1) $(ARGS) > $(OUTFILE1)
	
alessio: $(BIN2)
	timeout 20m ./$(BIN2) $(ARGS) > $(OUTFILE2)
	
ottimo: $(BIN3)
	timeout 30m ./$(BIN3) $(ARGS) > $(OUTFILE3)

run: alessio filippo ottimo
	python3 extract.py
	
valgrind: all
	valgrind ./$(BIN2) $(ARGS) > $(OUTFILE2) 2>&1
	
clean:
	rm -f S.pla $(OUTFILE1) $(OUTFILE2) $(OUTFILE3)
	
cleanall: clean
	rm -f *.o $(BIN1) $(BIN2) $(BIN3)

