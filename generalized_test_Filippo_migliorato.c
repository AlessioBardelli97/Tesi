#include <time.h>
#include "autosymmetry.h"

int main(int argc, char ** argv) {

    init(); double total=0;
    DdNode *s=NULL, *u=NULL, *ls=NULL;
    clock_t start, end; int i, dimLs=0, dimTotLs=0, max=0;
    
    boolean_function_t* f = parse_pla(manager, argv[1], TRUE);
    
    // Per ciascun output della funzione trattata
    // effettua il test di autosimmetria.
    for(i=0; i<f->outputs; i++) {
    
    	start = clock();

        // Unione tra on-set e dc-set.
        u = Cudd_bddOr(manager, f->on_set[i], f->dc_set[i]);
        Cudd_Ref(u);

        // Calcola l'insieme S.
        s = buildS(u, f->on_set[i], f->inputs);

        // Calcola l'insieme Ls.
        ls = build_Ls_3(s, f->inputs, f->on_set[i], f->dc_set[i], &dimLs);

        end = clock();
        
        dimTotLs += dimLs;
        max = dimLs > max ? dimLs : max;

        Cudd_RecursiveDeref(manager, f->on_set[i]);
        Cudd_RecursiveDeref(manager, f->dc_set[i]);
        Cudd_RecursiveDeref(manager, u);
        Cudd_RecursiveDeref(manager, s);
        if (ls != NULL) Cudd_RecursiveDeref(manager, ls);
        
        printf("\n%s(%i): \n  Inputs: %i\n  Dimensione di Ls: %i\n", argv[1], i, f->inputs, dimLs);
        printf("\n**********************************************************\n");

        total += (double)(end-start);
        
        #if DEBUG_TEST
	        printf("\n**********************************************************\n");
		#endif
    }

    printf("\n%s:\n  Inputs: %i\n  Outputs: %i\n  Tempo totale: %f\n  Autosimmetria media: %lf\n  Autosimmetria massima: %d\n", argv[1], f->inputs, f->outputs, (total/CLOCKS_PER_SEC), (double)dimTotLs/(double)f->outputs, max);

    free(f->dc_set);
    free(f->on_set);
    free(f);

    quit(); return 0;
}

