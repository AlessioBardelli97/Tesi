TARGET = test2
SRC    = $(TARGET).c
BIN    = $(TARGET).out

OBJ    = parser.o GaussJordan.o autosymmetry.o equations.o binmat.o logic.o 

ccflags = -O3 -std=c99 -Wall
debug   = -g -DDEBUG

$(BIN):  $(SRC) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC) $(OBJ) -o $@ -lcudd -lm

%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $< 

run: $(BIN)
	./$(BIN) input.pla
	
debug: $(BIN)
	valgrind ./$(BIN) input.pla

clean:
	rm -f *.o
	
cleanall: clean
	rm -f $(BIN)
	
