#include <time.h>
#include "autosymmetry.h"

int main(int argc, char** argv)
{
    init();

    boolean_function_t* f = parse_pla(manager, argv[1], 1);
    DdNode* lf, *u; int dimTotLs=0, max=0, i;
    double elements=0, degree=0, total=0;
    clock_t start, end;

    for(i=0; i<f->outputs; i++)
    {
        start = clock();

        u = Cudd_bddOr(manager, f->on_set[i], f->dc_set[i]);
        Cudd_Ref(u);

        lf = buildLf(u, f->inputs);

        elements = Cudd_CountMinterm(manager, lf, f->inputs);
        degree = log(elements)/log(2);

        end = clock();

        dimTotLs += degree;
        max = degree > max ? degree : max;

        Cudd_RecursiveDeref(manager, f->on_set[i]);
        Cudd_RecursiveDeref(manager, f->dc_set[i]);
        Cudd_RecursiveDeref(manager, lf);

        printf("\n%s(%i): \n  Inputs: %i\n  Dimensione di Ls: %i\n", argv[1], i, f->inputs, (int)degree);
        printf("\n**********************************************************\n");

        total += (double)(end-start);
    }

    printf("\n%s:\n  Inputs: %i\n  Outputs: %i\n  Tempo totale: %f\n  Autosimmetria media: %lf\n  Autosimmetria massima: %d\n", argv[1], f->inputs, f->outputs, (total/CLOCKS_PER_SEC), (double)dimTotLs/(double)f->outputs, max);
    
    free(f->dc_set);
    free(f->on_set);
    free(f);

    quit(); return 0;
}