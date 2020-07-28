#include <stdlib.h>
#include <stdio.h>
#include "autosymmetry.h"

void write_dd(DdManager* m, DdNode* dd, char* filename) {

	FILE* outfile = fopen(filename,"w");
	DdNode** ddarray = malloc(sizeof(DdNode*));
	ddarray[0] = dd;
	Cudd_DumpDot(m, 1, ddarray, NULL, NULL, outfile);
	free(ddarray);
	fclose(outfile);
}

int main(int argc, char *argv[]) {

	init ();

    boolean_function_t* f = parse_pla (manager, argv[1], 0);

    DdNode* MVS = buildMinimumVectorSpace(f->on_set[0], f->inputs, FALSE);
    
    MVS = Cudd_BddToAdd(manager, MVS);
    write_dd(manager, MVS, "MVS.dot");
    
    free(f->on_set);
	free(f->dc_set);
	free(f); quit();
	
	return 0;
}
