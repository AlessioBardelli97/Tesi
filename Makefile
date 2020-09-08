#!/bin/bash

target = all_zeros_test.out 					 \
		 all_ones_test.out  					 \
		 generalized_test_Filippo.out 			 \
		 generalized_test_Alessio.out            \
		 generalized_test_Filippo_migliorato.out \
		 generalized_test_Ottimo.out			 

OBJ  = parser.o autosymmetry.o equations.o binmat.o logic.o

ccflags = -std=c99 #-Wall
debug   = -g #-DDEBUG

.PHONY: all clean cleanall runall

pla/%.pla: all
	./run.sh $@

%.out: %.c $(OBJ)
	gcc  $(ccflags) $(debug) $< $(OBJ) -o $@ -lcudd -lm
	
%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $<

all: $(target)

runall: all
	./runall.sh

clean:
	rm -f S.pla output.*.txt eq*.re fk*
	
cleanall: clean
	rm -f *.o *.out

