#include "autosymmetry.h"
#include "parser.h"

// argv[1] == file .pla contenente l'input 

int main(int argc, char** argv) {

    init(); DdNode *u, *S;

    boolean_function_t* f = parse_pla(manager, argv[1], TRUE);
    
    u = Cudd_bddOr(manager, f->on_set[0], f->dc_set[0]);
    Cudd_Ref(u);
    
    S = buildS(u, f->on_set[0], f->inputs);
    Cudd_RecursiveDeref(manager, u);*/

    /*write_bdd(manager, S, "s.dot");
    printPla(manager, "s.pla", S, 2*f->inputs);*/
    
    DdNode* MVS = buildMinimumVectorSpace(S, f->inputs, TRUE);
    
    if (MVS) {
    	write_bdd(manager, MVS, "mvs.dot");
	    printPla(manager, "mvs.pla", MVS, 2*f->inputs);
	}
    
    free(f->on_set);
	free(f->dc_set);
	free(f); quit();
	
    return 0;
}
