ccflags = -O3 -std=c99 -Wall
debug   = -g -DDEBUG

main.out: parser.o GaussJordan.o autosymmetry.o equations.o main.c
	gcc  $(ccflags) $(debug) main.c parser.o GaussJordan.o autosymmetry.o equations.o -o $@ -lcudd -lm

%.o : %.c %.h
	gcc $(ccflags) $(debug) -c -o $@ $< 

run: main.out
	./main.out
	
debug: main.out
	valgrind ./main.out

clean:
	rm -f *.o
	
cleanall: clean
	rm -f main.out
	
