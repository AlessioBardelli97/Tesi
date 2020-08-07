#include "autosymmetry.h"
#include "parser.h"

// argv[1] == file .pla contenente l'input 

int main(int argc, char** argv) {

    init(); /*DdNode *u, *S;*/

    boolean_function_t* f = parse_pla(manager, argv[1], FALSE);
    
    /*u = Cudd_bddOr(manager, f->on_set[0], f->dc_set[0]);
    Cudd_Ref(u);*/

    /*S = buildS(u, f->on_set[0], f->inputs);
    Cudd_RecursiveDeref(manager, u);*/
    
    /*DdNode* Ls = buildMaximumVectorSpace(S, f->inputs, TRUE);
    write_bdd_pla(manager, Ls, "ls.pla", TRUE);*/
    
    DdNode* Ls = buildMaximumVectorSpace(f->on_set[0], f->inputs, FALSE);
    write_bdd_dot(manager, Ls, "ls.dot");
    write_bdd_pla(manager, Ls, "ls.pla", FALSE);

	free(f->on_set);
	free(f->dc_set);
	free(f); quit();

    return 0;
}
