TARGET = test
SRC    = $(TARGET).c
BIN    = $(TARGET).out

ARGS = input1.pla

OBJ    = parser.o autosymmetry.o equations.o binmat.o logic.o

ccflags = -std=c99 -Wall
debug   = -g -DDEBUG

$(BIN):  $(SRC) $(OBJ)
	gcc  $(ccflags) $(debug) $(SRC) $(OBJ) -o $@ -lcudd -lm

%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $<

run: $(BIN)
	./$(BIN) $(ARGS) && cat mvs.pla
	
valgrind: $(BIN)
	valgrind ./$(BIN) $(ARGS) && cat mvs.pla
	
gdb: $(BIN)
	gdb -q $(BIN)

clean:
	rm -f *.o *.dot mvs.pla
	
cleanall: clean
	rm -f $(BIN)
	
