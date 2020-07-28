#include "autosymmetry.h"
#include "parser.h"

/* 
 * Se si specificano anche gli alfa nella parse_pla 
 * (passando 1 come terzo parametro)
 * gli input diventano 2*f->input per la printPla.
 */
 
// argv[1] == file .pla contenente l'input 

int main(int argc, char** argv) {

    init(); DdNode *u, *S;

    boolean_function_t* f = parse_pla(manager, argv[1], 1);

    u = Cudd_bddOr(manager, f->on_set[0], f->dc_set[0]);
    Cudd_Ref(u);
    
    S = buildS(u, f->on_set[0], f->inputs);
    Cudd_RecursiveDeref(manager, u);
    
    DdNode* MVS = buildMinimumVectorSpace(u, f->inputs, TRUE);
    
    free(f->on_set);
	free(f->dc_set);
	free(f); quit();
	
    return 0;
}
