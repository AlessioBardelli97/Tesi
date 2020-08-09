#include <stdio.h>
#include <stdlib.h>
#include <cudd.h>
#include <math.h>
#include <time.h>

#include "autosymmetry.h"
#include "parser.h"
#include "debug.h"

int main(int argc, char ** argv) 
{
    init(); int i; DdNode* lf=NULL, *s=NULL, *u=NULL, *ls=NULL;
    clock_t start0, end0, start1, end1;
    double elements=0, degree=0, total=0;

    boolean_function_t* f = parse_pla(manager, argv[1], 1);
    
    //per ciascun output della funzione trattata
    //effettua il test di autosimmetria ed eventualmente
    //il calcolo della forma ridotta e delle equazioni di riduzione
    for(i=0; i<f->outputs; i++)
    {
        start0 = clock();

        //spazio vettoriale lf
        lf = buildLf(f->on_set[i], f->inputs);

        //unione tra on-set e dc-set
        u = Cudd_bddOr(manager, f->on_set[i], f->dc_set[i]);
        Cudd_Ref(u);

        //insieme S
        s = buildS(u, f->on_set[i], f->inputs);

        //calcola uno spazio vettoriale contenuto in S
        ls = extractVectorSpace(s, lf, f->inputs);

        dbg_bdd_printf(manager, ls, f->inputs, "Spazio vettoriale Ls:\n");

        //calcola la dimensione di ls
        elements = Cudd_CountMinterm(manager, ls, f->inputs);
        degree = log(elements)/log(2);

        end0 = clock();

        Cudd_RecursiveDeref(manager, f->on_set[i]);
        Cudd_RecursiveDeref(manager, f->dc_set[i]);
        Cudd_RecursiveDeref(manager, lf);
        Cudd_RecursiveDeref(manager, u);
        Cudd_RecursiveDeref(manager, s);
        Cudd_RecursiveDeref(manager, ls);
        
        printf("%s(%i) %i %i %f\n", argv[1], i, f->inputs, (int)degree, (double)(f->dcs/f->outputs));

        total += (double)(end0-start0);
    }

    printf("%s %i %i %f %f\n", argv[1], f->inputs, f->outputs, (double)(f->dcs/f->outputs), (total/CLOCKS_PER_SEC));

    free(f->dc_set);
    free(f->on_set);
    free(f);

    quit(); return 0;
}

