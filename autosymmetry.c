#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "autosymmetry.h"
#include "debug.h"

static inline DdNode* difference(DdNode* a, DdNode* b) 
{
    // A\B = A and not(B)

    DdNode* res = Cudd_bddAnd(manager, a, Cudd_Not(b));
    Cudd_Ref(res);

    return res;
}

static DdNode* translate(DdNode* Space, DdNode* u, int inputs, int set) 
{
    int* cube = calloc(2*inputs, sizeof(int));

    if (cube == NULL)
    {
        perror("Errore chiamata calloc: ");
        return NULL;
    }

    // costruisce il cubo dal bdd del mintermine u.
    // il vettore cube ha nella posizione i il valore 1
    // se la variabile i appare affermata nel mintermine,
    // 0 se appare negata e 2 se non compare.
	if (!Cudd_BddToCubeArray(manager, u, cube))
    {
        fprintf(stderr, "Errore chiamata Cudd_BddToCudeArray\n");
        free(cube);
        return NULL;
    }

    DdNode* tmp;
    DdNode* xor;
    DdNode* alpha;
    DdNode* p;
    DdNode* g1;
    int i;

    g1 = Space;
    Cudd_Ref(Space);

    // il parametro set serve per gestire il caso in cui il mintermine
    // e lo spazio hanno due ordinamenti delle variabili differenti.
    // ad esempio nella funzione buildNewF il mintermine u dipende solo
    // dalle variabili x, mentre ls solo da quelle alfa.

    for (i=0; i<inputs; i++)
    {
        // recupera la variabile a_i
        alpha = Cudd_bddIthVar(manager, 2*i);

        if (cube[(2*i)+set] == 0) {p = Cudd_ReadLogicZero(manager); Cudd_Ref(p);}
        else if (cube[(2*i)+set] == 1) {p = Cudd_ReadOne(manager); Cudd_Ref(p);}
        else continue;

        // calcola la nuova variabile come a_i xor p
        xor = Cudd_bddXor(manager, alpha, p);
        Cudd_Ref(xor);

        // sostituisce ad a_i a_i xor p
        tmp = Cudd_bddCompose(manager, g1, xor, 2*i);
        Cudd_Ref(tmp);

        Cudd_RecursiveDeref(manager, p);
        Cudd_RecursiveDeref(manager, xor);
        Cudd_RecursiveDeref(manager, g1);
        g1 = tmp;
    }

    if (set)
    {
        int perm[2*inputs];

        for (i=0; i<inputs; i++)
        {
            perm[2*i] = (2*i)+1;
            perm[(2*i)+1] = 2*i;
        }

        tmp = Cudd_bddPermute(manager, g1, perm);
        Cudd_Ref(tmp);

        Cudd_RecursiveDeref(manager, g1);
        g1 = tmp;
    }

    free(cube);

    return g1;
}

void init() 
{
    manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
}

DdNode* buildNewF(DdNode* f, DdNode* Ls, int inputs) 
{
    DdNode* f_new = f;
    Cudd_Ref(f);

    DdNode* g = f;
    Cudd_Ref(f);

    DdNode** vars = calloc(2*inputs, sizeof(DdNode*));

    if (vars == NULL)
    {
        perror("Chiamata a calloc: ");
        return NULL;
    }

    // serve per poter estrarre un mintermine da f
    for (int i=0; i<inputs; i++)
        vars[i] = Cudd_bddIthVar(manager, (2*i)+1);

    DdNode* tmp1;
    DdNode* tmp2;
    DdNode* g2;
    DdNode* h;
    DdNode* u;

    g2 = Ls;
    Cudd_Ref(Ls);

    while (g!=Cudd_ReadLogicZero(manager))
    {
        // estrae un mintermine da g
        u = Cudd_bddPickOneMinterm(manager, g, vars, inputs);
        Cudd_Ref(u);

        dbg_bdd_printf(manager, u, inputs, "Mintermine scelto:\n")

        // calcola u xor Ls
        h = translate(g2, u, inputs, 1);

        dbg_bdd_printf(manager, h, inputs, "H:\n")

        // inserisce lo spazio affine h in f_new
        tmp1 = Cudd_bddOr(manager, f_new, h);
        Cudd_Ref(tmp1);

        dbg_bdd_printf(manager, f_new, inputs, "F new:\n")

        // toglie da g lo spazio affine h
        tmp2 = difference(g, h);
        Cudd_Ref(tmp2);

        Cudd_RecursiveDeref(manager, h);
        Cudd_RecursiveDeref(manager, u);
        Cudd_RecursiveDeref(manager, g);
        Cudd_RecursiveDeref(manager, f_new);

        f_new = tmp1;
        g     = tmp2;

        dbg_bdd_printf(manager, g, inputs, "g aggiornato:\n")
    }

    Cudd_RecursiveDeref(manager, g2);
    Cudd_RecursiveDeref(manager, g);

    free(vars);

    return f_new;
}

DdNode* restrictionFunction(DdNode* u, int *cv, int inputs) 
{
    DdNode* g = u;
    Cudd_Ref(u);

    DdNode* tmp;
    DdNode* xi;
    
    // cv nella posizione i vale 1 se i è una variabile
    // canonica della funzione trattata, 0 altrimenti.
    // assegna tutte le variabili canoniche a zero.
    for (int i=0; i<inputs; i++)
    {
        if (cv[i])
        {
            xi = Cudd_bddIthVar(manager, (2*i)+1);
            tmp = Cudd_bddRestrict(manager, g, Cudd_Complement(xi));
            Cudd_Ref(tmp);
            Cudd_RecursiveDeref(manager, g);

            g = tmp;
        }
    }

    return g;
}

int* computeCanonicalVariables(DdNode* Lf, int i, int inputs, int *cv) 
{
    // implementa l'algoritmo ricorsivo per il calcolo delle variabili canoniche

    if (i<inputs)
    {
        int j;

        if (Lf == Cudd_ReadLogicZero(manager)) return cv;

        else if (Lf == Cudd_ReadOne(manager))
        {
            for (j=i; j<inputs; j++)
                cv[j] = 1;

            return cv;
        }
        else
        {
            DdNode* g1 = Lf;
            Cudd_Ref(Lf);

            DdNode* xi = Cudd_bddIthVar(manager, 2*i);
            DdNode* h0 = Cudd_bddRestrict(manager, g1, Cudd_Complement(xi));
            DdNode* h1 = Cudd_bddRestrict(manager, g1, xi);
            int* result;

            Cudd_Ref(h0);
            Cudd_Ref(h1);

            if (h0!=Cudd_ReadLogicZero(manager) && h1 == Cudd_ReadLogicZero(manager))
            {
                result = computeCanonicalVariables(h0, i+1, inputs, cv);

                Cudd_RecursiveDeref(manager, g1);
                Cudd_RecursiveDeref(manager, h0);
                Cudd_RecursiveDeref(manager, h1);

                return result;
            }
            else if (h0 == Cudd_ReadLogicZero(manager) && h1!=Cudd_ReadLogicZero(manager))
            {
                result = computeCanonicalVariables(h1, i+1, inputs, cv);

                Cudd_RecursiveDeref(manager, g1);
                Cudd_RecursiveDeref(manager, h0);
                Cudd_RecursiveDeref(manager, h1);

                return result;
            }
            else
            {
                cv[i] = 1;

                for (j=i+1; j<inputs; j++)
                    cv[j] = 0;

                result = computeCanonicalVariables(h1, i+1, inputs, cv);

                Cudd_RecursiveDeref(manager, g1);
                Cudd_RecursiveDeref(manager, h0);
                Cudd_RecursiveDeref(manager, h1);

                return result;
            }
        }
    }

    return cv;
}

DdNode* buildS(DdNode* u, DdNode* g1, int inputs) 
{
    DdNode* g2; 
    DdNode* g3; 
    DdNode* g4;
    int i;

    // g2 BDD per f^{on e dc}(x1 xor a1,...,xn xor an)
    g2 = u;
    Cudd_Ref(u);

    // insieme di supporto di g1
    DdNode* cube = Cudd_ReadOne(manager);
    Cudd_Ref(cube);

    // serve per costruire il bdd della quantificazione
    // universale
    for (i=0; i<inputs; i++)
    {
        DdNode* tmp = Cudd_bddAnd(manager, cube, Cudd_bddIthVar(manager, (2*i)+1));
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(manager, cube);
        cube = tmp; 
    }

    DdNode* alpha;
    DdNode* xor;
    DdNode* tmp;
    DdNode* x;

    for (i=0; i<inputs; i++)
    {
        alpha = Cudd_bddIthVar(manager, 2*i);
        x = Cudd_bddIthVar(manager, (2*i)+1);

        // calcola xi xor ai
        xor = Cudd_bddXor(manager, x, alpha);
        Cudd_Ref(xor);

        // sostituisce xi xor ai alla variabile di indice i nel BDD g2
        tmp = Cudd_bddCompose(manager, g2, xor, (2*i)+1);
        Cudd_Ref(tmp);

        Cudd_RecursiveDeref(manager, xor);
        Cudd_RecursiveDeref(manager, g2);

        g2 = tmp;
    }

    // g3 BDD per g1 implica g2 = not(g1) or g2
    g3 = Cudd_bddOr(manager, Cudd_Not(g1), g2);
    Cudd_Ref(g3);

    // g4 BDD per forall x.g3
    g4 = Cudd_bddUnivAbstract(manager, g3, cube);
    Cudd_Ref(g4);

    Cudd_RecursiveDeref(manager, g2);
    Cudd_RecursiveDeref(manager, g3);
    Cudd_RecursiveDeref(manager, cube);

    return g4;
}

DdNode* buildLf(DdNode* g1, int inputs) 
{
    // analogo a buildS

    DdNode* g2; 
    DdNode* g3; 
    DdNode* g4;
    int i;

    g2 = g1;
    Cudd_Ref(g1);

    DdNode* cube = Cudd_ReadOne(manager);
    Cudd_Ref(cube);

    DdNode* alpha;
    DdNode* xor;
    DdNode* tmp;
    DdNode* x;

    for (i=0; i<inputs; i++)
    {
        tmp = Cudd_bddAnd(manager, cube, Cudd_bddIthVar(manager, (2*i)+1));
        Cudd_Ref(tmp);
        Cudd_RecursiveDeref(manager, cube);
        cube = tmp; 
    }

    for (int i=0; i<inputs; i++)
    {
        alpha = Cudd_bddIthVar(manager, 2*i);
        x = Cudd_bddIthVar(manager, (2*i)+1);

        xor = Cudd_bddXor(manager, x, alpha);
        Cudd_Ref(xor);

        tmp = Cudd_bddCompose(manager, g2, xor, (2*i)+1);
        Cudd_Ref(tmp);

        Cudd_RecursiveDeref(manager, xor);
        Cudd_RecursiveDeref(manager, g2);

        g2 = tmp;
    }

    g3 = Cudd_bddXnor(manager, g1, g2);
    Cudd_Ref(g3);

    g4 = Cudd_bddUnivAbstract(manager, g3, cube);
    Cudd_Ref(g4);

    Cudd_RecursiveDeref(manager, g2);
    Cudd_RecursiveDeref(manager, g3);
    Cudd_RecursiveDeref(manager, cube);

    return g4;
}

DdNode* extractVectorSpace(DdNode* S, DdNode* Ls, int inputs) 
{
    DdNode** vars = calloc(2*inputs, sizeof(DdNode*));

    if (vars == NULL)
    {
        perror("Chiamata calloc: ");
        return NULL;
    }

    // serve per poter estrarre un mintermine
    for (int i=0; i<inputs; i++)
        vars[i] = Cudd_bddIthVar(manager, (2*i));

    DdNode* diff;
    DdNode* tmp1;
    DdNode* S1;
    DdNode* tr;
    DdNode* g1;
    DdNode* x;

    S1 = S;
    Cudd_Ref(S);

    g1 = Ls;
    Cudd_Ref(Ls);

    // numero di punti in S e in Ls
    double set_size = Cudd_CountMinterm(manager, S1, inputs), ls_size = Cudd_CountMinterm(manager, g1, inputs);

    while (set_size > ls_size)
    {
        // calcola S1 \ Ls
        diff = difference(S1, g1);

        dbg_bdd_printf(manager, diff, inputs, "Differenza tra S1 e Lf:\n")

        // estrae un mintermine dalla differenza tra S ed Ls
        x = Cudd_bddPickOneMinterm(manager, diff, vars, inputs);
        Cudd_Ref(x);

        dbg_bdd_printf(manager, x, inputs, "Mintermine scelto dalla differenza:\n")

        // calcola x xor Ls
        tr = translate(g1, x, inputs, 0);

        dbg_bdd_printf(manager, x, inputs, "x xor Lf:\n")

        if (Cudd_bddLeq(manager, tr, S))
        {
            
            // tmp1 è il bdd per l'unione tra ls calcolato
            // fino ad ora e x xor ls
            tmp1 = Cudd_bddOr(manager, g1, tr);
            Cudd_Ref(tmp1);

            Cudd_RecursiveDeref(manager, diff);
            Cudd_RecursiveDeref(manager, x);
            Cudd_RecursiveDeref(manager, tr);
            Cudd_RecursiveDeref(manager, g1);
            
            g1 = tmp1;

            ls_size = Cudd_CountMinterm(manager, g1, inputs);

            dbg_bdd_printf(manager, g1, inputs, "Nuovo Lf:\n")
        }
        else
        {
            
            // tmp1 è il bdd per la differenza tra l'insieme S
            // e x xor ls
            tmp1 = difference(S1, tr);

            Cudd_RecursiveDeref(manager, diff);
            Cudd_RecursiveDeref(manager, x);
            Cudd_RecursiveDeref(manager, tr);
            Cudd_RecursiveDeref(manager, S1);

            S1 = tmp1;

            set_size = Cudd_CountMinterm(manager, S1, inputs);

            dbg_bdd_printf(manager, S1, inputs, "Nuovo S1:\n")
        }
    }

    Cudd_RecursiveDeref(manager, S1);

    free(vars);

    return g1;
}

Eqns_t* reductionEquations(DdNode* h, int i, int inputs, EQ_manager *eq_man) 
{
	if (h == Cudd_ReadOne(manager) || h == Cudd_ReadLogicZero(manager) || i>=inputs)
	{
		Eqns_t *empty = new_eqns(eq_man);
		return empty;
	}
	DdNode* h0 = Cudd_bddRestrict(manager, h, Cudd_Complement(Cudd_bddIthVar(manager, 2*i)));  
	DdNode* h1 = Cudd_bddRestrict(manager, h, Cudd_bddIthVar(manager, 2*i));	  			   
	if (h1 == Cudd_ReadLogicZero(manager) && h0 == Cudd_ReadOne(manager))
	{
		Eqns_t *eqns = new_eqns(eq_man);
		Eqn_t* eqn = new_eqn(inputs);
		add_var(eqn, i, 1);
		add_eqn(eqns, eqn);
		return eqns;
	}
	else if (h0 == Cudd_ReadLogicZero(manager) && h1 == Cudd_ReadOne(manager))
	{
		Eqns_t *eqns = new_eqns(eq_man);
		Eqn_t* eqn = new_eqn(inputs);
		add_var(eqn, i, 0);
		add_eqn(eqns, eqn);
		return eqns;
	}
	else if (h1 == Cudd_ReadLogicZero(manager) && (h0 != Cudd_ReadLogicZero(manager) || h0 != Cudd_ReadOne(manager)))
	{	
		Eqns_t *S0 = reductionEquations(h0, i+1, inputs, eq_man);
		Eqn_t* eqn = new_eqn(inputs);
		add_var(eqn, i, 1);
		add_eqn(S0, eqn);
		return S0;
	}
	else if (h0 == Cudd_ReadLogicZero(manager) && (h1 != Cudd_ReadLogicZero(manager) || h1 != Cudd_ReadOne(manager)))
	{
		Eqns_t *S1 =  reductionEquations(h1, i+1, inputs, eq_man);
		Eqn_t* eqn = new_eqn(inputs);
		add_var(eqn, i, 0);
		add_eqn(S1, eqn);
		return S1;
	}
	else
	{
		Eqns_t *S0 = reductionEquations(h0, i+1, inputs, eq_man);
		Eqns_t *S1 = reductionEquations(h1, i+1, inputs, eq_man);
		Eqns_t *S = new_eqns(eq_man);
		list_t l = S0->head;
		while (l!=NULL)
		{
			Eqn_t *eqn = new_eqn(inputs);
			memcpy(eqn, l, sizeof(Eqn_t)); 
			if (!isInEqns(S1, eqn))
				add_var(eqn, i, 0);
			add_eqn(S, eqn);
			l = l->next;
		}
		return S;
	}
}

DdNode* buildMinimumVectorSpace(DdNode* S, int numInputs, boolean b_alpha) {

	vProduct XorInput=NULL;
	binmat *bm=NULL; MO_SOP funzione=NULL;
    int i, uscite, numRi; Product alpha = NULL;
    DdNode* result = NULL;
    
    printPla (manager, "S.pla", S, b_alpha ? 2*numInputs : numInputs);
    LoadPLA ("S.pla", &funzione, &uscite);
    
    alpha = CreateProduct(funzione[0]->NumInputs);
    for (i = 0; i < numInputs; i++)
    	alpha[i] = '0';
    
    dbg_printf("\nalpha: %s\n\n", alpha);
    
    #ifdef DEBUG_TEST
        printf("Products: ");
        for (i = 0; i < funzione[0]->NumProducts; i++)
            printf("%s, ", funzione[0]->Products[i]);
        printf("\n\n");
    #endif
    
    funzione[0]->NumCEXProducts = 0;
    funzione[0]->CEXLetterali = CreateProduct (funzione[0]->NumInputs);

    for (i = 0; i < funzione[0]->NumInputs; i++) {
        funzione[0]->CEXLetterali[i] = '-';
    }

	if (funzione[0]->NumProducts > 1) {
	
		setAlpha (funzione[0], alpha);
        XorInput = XorConAlpha(funzione[0]);
        numRi = funzione[0]->NumProducts + funzione[0]->NumInputs;
        
        bm = bm_new (numRi, funzione[0]->NumInputs);
        RiempiMatrice (bm,XorInput);
        
        #ifdef DEBUG_TEST
            printf ("la matrice prima della gauss elimination\n");
            bm_print (bm); printf("\n");
        #endif

		bm_sort_by_rows (bm);
        bm_unique_row_echelon_form (bm);
            
        #ifdef DEBUG_TEST
            printf ("la matrice dopo la gauss elimination\n");
            bm_print (bm); printf("\n");
        #endif
		
		if (bm->rows <= funzione[0]->NumInputs) {
            
        	CreaCEX(bm, funzione[0]);
                
            #ifdef DEBUG_TEST
                printf("NumCEXProducts: %d\n\n", funzione[0]->NumCEXProducts);
                    
                printf("CEXProducts: ");
                for (i = 0; i < funzione[0]->NumCEXProducts; i++)
                    printf("%s, ", funzione[0]->CEXProducts[i]);
                printf("\n\n");
                        
                printf("OutCEX: %s\n\n", funzione[0]->OutCEX);
                    
                printf("variabiliNC: ");
                for (i = 0; i < funzione[0]->NumCEXProducts; i++)
                    printf ("%d, ", funzione[0]->variabiliNC[i]); 
                printf("\n\n");
                        
                printf("CEXLetterali: %s\n\n", funzione[0]->CEXLetterali);
            #endif
            
            result = Cudd_ReadOne(manager); Cudd_Ref(result);
            DdNode* tmp, *x, *sum; int j, letterale;
            
            /*for (i = 0; i < funzione[0]->NumInputs; i++) {
            
            	if (b_alpha) x = Cudd_bddIthVar(manager, 2*i);
                else x = Cudd_bddIthVar(manager, i);
            
				DdNode* tmp2 = Cudd_bddOr(manager, x, Cudd_Not(x));
        		Cudd_Ref(tmp2);
		        Cudd_RecursiveDeref(manager, result);
        		result = tmp; 
            
            }*/

            for (i = 0; i < funzione[0]->NumCEXProducts; i++) {

                sum = Cudd_ReadLogicZero(manager); Cudd_Ref(sum);

                for (j = 0; j < funzione[0]->NumInputs; j++) {

                    if (funzione[0]->CEXProducts[i][j] == '1') {

                        if (b_alpha) x = Cudd_bddIthVar(manager, 2*j);
                        else x = Cudd_bddIthVar(manager, j);

                        if (funzione[0]->variabiliNC[i] == j && funzione[0]->OutCEX[i] == '0')
                            x = Cudd_Not(x);

                        tmp = Cudd_bddXor(manager, sum, x);
                        Cudd_Ref(tmp);
                        Cudd_RecursiveDeref(manager, sum);
                        sum = tmp;
                    }
                }

                tmp = Cudd_bddAnd(manager, result, sum);
                Cudd_Ref(tmp);
                Cudd_RecursiveDeref(manager, result);
                result = tmp;
            }

            for (i = 0; i < funzione[0]->NumInputs; i++) {
				
				letterale = funzione[0]->CEXLetterali[i];
				boolean flag = letterale == '0' || letterale == '1';
				
				if (flag) {
					if (b_alpha) x = Cudd_bddIthVar(manager, 2*i);
                    else x = Cudd_bddIthVar(manager, i);
				}
				
				if (letterale == '0')
					x = Cudd_Not(x);
				
				if (flag) {
				
					tmp = Cudd_bddAnd(manager, result, x);
                    Cudd_Ref(tmp);
	                Cudd_RecursiveDeref(manager, result);
	                result = tmp;
				}
            }
    	}
    }

    for (i = 0; i < funzione[0]->NumCEXProducts; i++)
    	DestroyProduct(&funzione[0]->CEXProducts[i]);
    
    for (i = 0; i < funzione[0]->NumProducts; i++)
    	DestroyProduct(&XorInput[i]);
    
    DestroyProduct(&alpha);
    DestroyProduct(XorInput);
    DestroyProduct(&funzione[0]->alpha);
    DestroyProduct(&funzione[0]->CEXLetterali);
    DestroyProduct(&funzione[0]->OutCEX);
    
    bm_free(bm);
    unlink("S.pla");
    free(funzione[0]->CEXProducts);
    free(funzione[0]->variabiliNC);
    free(XorInput); DestroySOP(funzione);
    free(funzione);
    
    return result;
}

void quit()
{
    Cudd_Quit(manager);
}
