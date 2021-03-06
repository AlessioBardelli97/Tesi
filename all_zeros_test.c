#include <time.h>
#include "autosymmetry.h"

int main(int argc, char** argv)
{
    init();

    boolean_function_t* f = parse_pla(manager, argv[1], 1);
    DdNode* lf; int dimTotLs=0, max=0, i;
    double elements=0, degree=0, total=0;
    clock_t start, end;

    for(i=0; i<f->outputs; i++)
    {
        start = clock();

        lf = buildLf(f->on_set[i], f->inputs);

        elements = Cudd_CountMinterm(manager, lf, f->inputs);
        degree = log(elements)/log(2);

        end = clock();

        dimTotLs += degree;
        max = degree > max ? degree : max;

        Cudd_RecursiveDeref(manager, f->on_set[i]);
        Cudd_RecursiveDeref(manager, f->dc_set[i]);
        Cudd_RecursiveDeref(manager, lf);

        printf("%s(%i): %.3lf\n", argv[1], i, degree);

        total += (double)(end-start);
    }

    printf("%s %i %i %.3f %i %f\n", argv[1], f->inputs, f->outputs, (double)dimTotLs/(double)f->outputs, max, (total/CLOCKS_PER_SEC));
    
    free(f->dc_set);
    free(f->on_set);
    free(f);

    quit(); return 0;
}
