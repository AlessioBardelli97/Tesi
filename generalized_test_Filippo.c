#include <time.h>

#include "autosymmetry.h"
#include "parser.h"

int main(int argc, char ** argv) 
{
    init(); double elements=0, degree=0, total=0;
    int i; DdNode* lf=NULL, *s=NULL, *u=NULL, *ls=NULL;
    clock_t start, end; int dimTotLs=0, max=0;

    boolean_function_t* f = parse_pla(manager, argv[1], 1);
    
    //per ciascun output della funzione trattata
    //effettua il test di autosimmetria ed eventualmente
    //il calcolo della forma ridotta e delle equazioni di riduzione
    for(i=0; i<f->outputs; i++)
    {
        start = clock();

        //spazio vettoriale lf
        lf = buildLf(f->on_set[i], f->inputs);

        //unione tra on-set e dc-set
        u = Cudd_bddOr(manager, f->on_set[i], f->dc_set[i]);
        Cudd_Ref(u);

        //insieme S
        s = buildS(u, f->on_set[i], f->inputs);

        //calcola uno spazio vettoriale contenuto in S
        ls = extractVectorSpace(s, lf, f->inputs);

        dbg_bdd_printf(manager, ls, f->inputs, "\nSpazio vettoriale Ls");

        //calcola la dimensione di ls
        elements = Cudd_CountMinterm(manager, ls, f->inputs);
        degree = log(elements)/log(2);
        
        dimTotLs += degree;
        
        max = degree > max ? degree : max;

        end = clock();

        Cudd_RecursiveDeref(manager, f->on_set[i]);
        Cudd_RecursiveDeref(manager, f->dc_set[i]);
        Cudd_RecursiveDeref(manager, lf);
        Cudd_RecursiveDeref(manager, u);
        Cudd_RecursiveDeref(manager, s);
        Cudd_RecursiveDeref(manager, ls);
        
        printf("\n%s(%i): \n  Inputs: %i\n  Dimensione di Ls: %i\n", argv[1], i, f->inputs, (int)degree);

        total += (double)(end-start);
        
        printf("\n**********************************************************\n");
    }

    printf("\n%s:\n  Inputs: %i\n  Outputs: %i\n  Tempo totale: %f\n  Autosimmetria media: %lf\n  Autosimmetria massima: %d\n", argv[1], f->inputs, f->outputs, (total/CLOCKS_PER_SEC), (double)dimTotLs/(double)f->outputs, max);

    free(f->dc_set);
    free(f->on_set);
    free(f);

    quit(); return 0;
}

