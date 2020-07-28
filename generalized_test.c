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
    init();

    boolean_function_t* f = parse_pla(manager, argv[1], 1);
    int i;

    DdNode* lf;
    DdNode* s;
    DdNode* u;
    DdNode* ls;
    DdNode* x;
    DdNode* tr;
    DdNode* new_f;
    DdNode* fk;

    DdNode** vars = calloc(2*f->inputs, sizeof(DdNode*));

    double elements=0, degree=0, avg_degree=0;

    if(vars==NULL)
    {
        perror("Errore chiamata calloc: ");
        return -1;
    }

    for(int i=0; i<f->inputs; i++)
    {
        DdNode* tmp = Cudd_bddIthVar(manager, i);
        vars[i] = Cudd_bddIthVar(manager, (2*f->inputs)-1-i);
        vars[(2*f->inputs)-1-i] = tmp;
    }

    int** cv = calloc(f->outputs, sizeof(int*));

    if(cv==NULL)
    {
        perror("Errore chiamata calloc: ");
        free(vars);
        return -1;
    }

    //array per i nomi dei file contenenti le equazioni di riduzione
    //e le pla delle forme ridotte.
    char** eq_file_names = calloc(f->outputs, sizeof(char*));
    char** red_file_names = calloc(f->outputs, sizeof(char*));

    for(i=0; i<f->outputs; i++)
    {
        eq_file_names[i] = calloc(10, sizeof(char));
        red_file_names[i] = calloc(10, sizeof(char));
        sprintf(eq_file_names[i], "eq%i.re", i);
        sprintf(red_file_names[i], "fk%i.pla", i);
    }

    //array per le variabili canoniche
    for(i=0; i<f->outputs; i++)
        cv[i] = calloc(f->inputs, sizeof(int));

    clock_t start0, end0, start1, end1;
    double total=0, max_degree=0;
    int max_ns=0, min_ns = INT32_MAX, ns=0, out_num=0;

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

        //se la funzione è autosimmetrica di grado k>=1
        //calcola la forma ridotta e le equazioni di riduzione
        if(degree>=1 && degree<=f->inputs)
        {
            dbg_printf("Numero di elementi in Ls: %.2f\n", elements);
            dbg_printf("Grado di autosimmetria di f_out_%i: %.2f\n", i, degree);

            start1 = clock();

            new_f = buildNewF(f->on_set[i], ls, f->inputs);

            cv[i] = computeCanonicalVariables(ls, 0, f->inputs, cv[i]);

            fk = restrictionFunction(new_f, cv[i], f->inputs);

            if(Cudd_SupportSize(manager, fk)!=(f->inputs-(int)degree))
            {
                fprintf(stderr, "Costruzione di fk fallita\n");
                return -1;
            }

            EQ_manager* eq_manager = EQ_manager_init();
            Eqns_t* eq = new_eqns(eq_manager);

            eq = reductionEquations(ls, 0, f->inputs, eq_manager);
            complements_all(eq);

            end1 = clock();

            FILE *eq_file = fopen(eq_file_names[i], "w");
            print_equations(eq_file, eq);
            fclose(eq_file);
            EQ_manager_quit(eq_manager);

            printPla(manager, red_file_names[i], fk, 2*f->inputs);

            Cudd_RecursiveDeref(manager, f->on_set[i]);
            Cudd_RecursiveDeref(manager, f->dc_set[i]);
            Cudd_RecursiveDeref(manager, lf);
            Cudd_RecursiveDeref(manager, u);
            Cudd_RecursiveDeref(manager, s);
            Cudd_RecursiveDeref(manager, ls);
            Cudd_RecursiveDeref(manager, new_f);
            Cudd_RecursiveDeref(manager, fk);

            if(degree>max_degree) max_degree = degree;

            avg_degree += degree;
            total += (double)(end1-start1);
        }
        else
        {
            Cudd_RecursiveDeref(manager, f->on_set[i]);
            Cudd_RecursiveDeref(manager, f->dc_set[i]);
            Cudd_RecursiveDeref(manager, lf);
            Cudd_RecursiveDeref(manager, u);
            Cudd_RecursiveDeref(manager, s);
            Cudd_RecursiveDeref(manager, ls);
        }

        ns = Cudd_SupportSize(manager, f->on_set[i]);

        //printf("%s(%i) %i %i %i %f\n", argv[1], i, f->inputs, ns, (int)degree, (double)(f->dcs/f->outputs));

        total += (double)(end0-start0);
    }

    printf("%s %i %i %f %.2f %.2f %f\n", argv[1], f->inputs, f->outputs, (double)(f->dcs/f->outputs), (double)(avg_degree/f->outputs), max_degree, (total/CLOCKS_PER_SEC));

    for(i=0; i<f->outputs; i++)
    {
        free(eq_file_names[i]);
        free(red_file_names[i]);
        free(cv[i]);
    }

    free(eq_file_names);
    free(red_file_names);
    free(f->dc_set);
    free(f->on_set);
    free(vars);
    free(cv);
    free(f);

    quit();

    return 0;
}
