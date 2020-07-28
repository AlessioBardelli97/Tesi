#include <stdlib.h>
#include "autosymmetry.h"

int main(int argc, char *argv[]) {

	init ();

    boolean_function_t* f = parse_pla (manager, argv[1], 0);

    buildMinimumVectorSpace(f->on_set[0], f->inputs);
    
    free(f->on_set);
	free(f->dc_set);
	free(f); quit();
	
	return 0;
}
