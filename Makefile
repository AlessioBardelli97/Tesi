TARGET = test
SRC    = $(TARGET).c
BIN    = $(TARGET).out

ARGS = input.pla

OBJ    = parser.o autosymmetry.o equations.o binmat.o logic.o

ccflags = -std=c99 -Wall
debug   = -g -DDEBUG

$(BIN):  $(SRC) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC) $(OBJ) -o $@ -lcudd -lm

%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $<

run: $(BIN)
	./$(BIN) $(ARGS)
	
valgrind: $(BIN)
	valgrind ./$(BIN) $(ARGS)
	
gdb: $(BIN)
	gdb -q $(BIN)

clean:
	rm -f *.o *.dot
	
cleanall: clean
	rm -f $(BIN)
	
