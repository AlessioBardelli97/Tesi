#include "GaussJordan.h"
#include "autosymmetry.h"
#include "parser.h"

/* 
 * Se si specificano anche gli alfa nella parse_pla 
 * gli input diventano 2*f->input per la printPla.
 */

int main() {

    init (); DdNode *u, *S;
    char input_name[] = "input.pla";

    boolean_function_t* f = parse_pla (manager, input_name, 1);

    u = Cudd_bddOr (manager, f->on_set[0], f->dc_set[0]);
    Cudd_Ref (u);

    S = buildS (u, f->on_set[0], f->inputs);
    Cudd_RecursiveDeref(manager, u);
    
    void* dummy = get_linearly_independent_vectors (S, 2*f->inputs);
    
    free(dummy);
    free(f->on_set);
	free(f->dc_set);
	free(f); quit();
	
    return 0;
}
