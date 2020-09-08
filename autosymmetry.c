#include "autosymmetry.h"

// ****************************************************************************************************************************

static DdNode* buildMinimumVectorSpace(binmat* bm, SOP* fun, boolean b_alpha) {

	DdNode* result=NULL; int i;

	fun->NumCEXProducts = 0;
    fun->CEXLetterali = CreateProduct (bm->cols);

    for (i=0;i<bm->cols;i++) {
	    fun->CEXLetterali[i] = '-';
    }

	if (bm->rows <= fun->NumInputs) {
            
    	CreaCEX(bm, fun);
            
        #if DEBUG_TEST
            printf("\nNumCEXProducts: %d\n", fun->NumCEXProducts);
                
            printf("\nCEXProducts: ");
            for (i = 0; i < fun->NumCEXProducts; i++)
                printf("%s, ", fun->CEXProducts[i]);
            printf("\n");
                    
            printf("\nOutCEX: %s\n", fun->OutCEX);
                
            printf("\nvariabiliNC: ");
            for (i = 0; i < fun->NumCEXProducts; i++)
                printf ("%d, ", fun->variabiliNC[i]); 
            printf("\n");
                    
            printf("\nCEXLetterali: %s\n", fun->CEXLetterali);
        #endif
        
        result = Cudd_ReadOne(manager); Cudd_Ref(result);
        DdNode* tmp, *x, *sum; int j, letterale;
        
        for (i = 0; i < fun->NumCEXProducts; i++) {

            sum = Cudd_ReadLogicZero(manager); Cudd_Ref(sum);

            for (j = 0; j < fun->NumInputs; j++) {

                if (fun->CEXProducts[i][j] == '1') {

                    x = Cudd_bddIthVar(manager, b_alpha ? 2*j : j);

                    if (fun->variabiliNC[i] == j && fun->OutCEX[i] == '0')
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

        for (i = 0; i < fun->NumInputs; i++) {
			
			letterale = fun->CEXLetterali[i];
			boolean flag = letterale == '0' || letterale == '1';
			
			if (flag)
				x = Cudd_bddIthVar(manager, b_alpha ? 2*i : i);
			
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
    
    for (i = 0; i < fun->NumCEXProducts; i++) {
    	DestroyProduct(&fun->CEXProducts[i]);
    }
    
    DestroyProduct(&fun->CEXLetterali);
	DestroyProduct(&fun->OutCEX);
	
	free(fun->CEXProducts);
	free(fun->variabiliNC);
    
    return result;
}

/* 
 * arr[]  ---> Input Array 
 * n      ---> Size of input array 
 * r      ---> Size of a combination to be printed 
 * index  ---> Current index in data[] 
 * data[] ---> Temporary array to store current combination 
 * i      ---> index of current element in arr[]
 */
static DdNode* combination_1(int* arr, int n, int r, int index, int* data, int i, SOP* fun, DdNode* S, boolean b_alpha) {

    // Current cobination is ready, print it 
    if (index == r) {
    
    	binmat* bm = bm_new(r, fun->NumInputs);
    	bm_set_row_values(bm, data);

		#if DEBUG_TEST
			printf("\n****************************************************************\n");
		#endif
		
		dbg_printf("Possibile scelta di %d vettori linearmente indipendenti da S:\n", r);
            
    	#if DEBUG_TEST
	    	bm_print(bm);
	    #endif
    	
    	DdNode* VS = buildMinimumVectorSpace(bm, fun, b_alpha);
    	bm_free(bm);
    	
		if (Cudd_bddLeq(manager, VS, S)) {
			#if DEBUG_TEST
				printf("\nE' contenuto in S, abbiamo finito.\n");
			#endif
			return VS;
		} else {
			#if DEBUG_TEST
				printf("\nNon e' contenuto.\n");
			#endif
			return NULL;
		}
    }
  
    // When no more elements are there to put in data[]
    if (i >= n)
        return NULL;
  
    // current is included, put next at next location
    data[index] = arr[i];
    DdNode* result = combination_1(arr, n, r, index+1, data, i+1, fun, S, b_alpha);
    if (result)
    	return result;
  
    // current is excluded, replace it with next (Note that 
    // i+1 is passed, but index is not changed) 
    return combination_1(arr, n, r, index, data, i+1, fun, S, b_alpha);
}

// The main function that prints all combinations of size r 
// in arr[] of size n. This function mainly uses combinationUtil() 
static DdNode* Combination_1(int arr[], int n, int r, SOP* fun, DdNode* S, boolean b_alpha) {
    
    // A temporary array to store all combination one by one 
    int* data = calloc(r, sizeof(int));
  
    // Print all combination using temprary array 'data[]' 
    DdNode* result = combination_1(arr, n, r, 0, data, 0, fun, S, b_alpha);
    
    free(data); 
    return result;
}

static DdNode* combination_2(int* arr, int n, int r, int index, int* data, int i, SOP* fun, DdNode* S, boolean b_alpha) {

    // Current cobination is ready, print it 
    if (index == r) {
    
        // Inserisco i vettori selezionati
        // in una matrice binaria.
    	binmat* bm = bm_new(r, fun->NumInputs);
    	bm_set_row_values(bm, data);
        bm_sort_by_rows (bm);

        // Calcolo il minimo tra il numero di 
        // righe e colonne della matrice.
        int min = bm->rows >= bm->cols ? bm->cols : bm->rows;

		#if DEBUG_TEST
			printf("\n****************************************************************\n");
		#endif
		
		dbg_printf("Possibile scelta di %d vettori di S:\n", r);
            
    	#if DEBUG_TEST
	    	bm_print(bm);
	    #endif

        // Riduco la matrice a scalini.
        bm_unique_row_echelon_form (bm);

        // Se la matrice è di rango massimo.
        if (bm->rows == min) {

            // Costriusco lo spazio vettoriale generato con i vettori selezionati di S.
            DdNode* VS = buildMinimumVectorSpace(bm, fun, b_alpha); bm_free(bm);

            // Se lo spazio vettoriale generato 
            // è contenuto o uguale a S.
            if (Cudd_bddLeq(manager, VS, S)) {
                #if DEBUG_TEST
                    printf("\nE' contenuto in S, abbiamo finito.\n");
                #endif
                return VS;
		    } else {
                #if DEBUG_TEST
                    printf("\nNon e' contenuto.\n");
                #endif
                return NULL;
            }
        }

        bm_free(bm);
        return NULL;
    }
  
    // When no more elements are there to put in data[]
    if (i >= n)
        return NULL;
  
    // current is included, put next at next location
    data[index] = arr[i];
    DdNode* result = combination_2(arr, n, r, index+1, data, i+1, fun, S, b_alpha);
    if (result)
    	return result;
  
    // current is excluded, replace it with next (Note that 
    // i+1 is passed, but index is not changed) 
    return combination_2(arr, n, r, index, data, i+1, fun, S, b_alpha);
}

static DdNode* Combination_2 (int* points, int numPoints, int r, SOP* fun, DdNode* S, boolean b_alpha) {

	// A temporary array to store all combination one by one 
    int* data = calloc(r, sizeof(int));

    // Print all combination using temprary array 'data[]'
    DdNode* result = combination_2(points, numPoints, r, 0, data, 0, fun, S, b_alpha);
    
    free(data);
    return result;
}

// ****************************************************************************************************************************

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

    DdNode* diff, *tmp1, *S1, *tr, *g1, *x;

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

DdNode* build_Ls_1(DdNode* S, int inputs, boolean b_alpha, int* dimResult) {

	char** XorInput=NULL;
	binmat *bm=NULL; MO_SOP fun=NULL;
    int i, uscite, numRi; Product alpha = NULL;
    DdNode* result = NULL;
    
    // Calcolo il numero di punti dell'insieme S.
	double num_points = Cudd_CountMinterm(manager, S, inputs);
	dbg_printf("\nNumero di punti di S: %d\n", (int)num_points);

	// Calcolo la dimensione dell'insieme S.
    int dimS = (int)log2(num_points);
    dbg_printf("\ndimS: %d\n", dimS);

    // Inizialmente la dimensione del più
    // grande spazio vettoriale che contiene
    // S è pari alla dimensione di S.
    int dimLs = dimS;
    
    // Scrivo S in formato pla per poi caricarlo in una SOP.
    write_bdd2pla (manager, S, "S.pla", inputs, b_alpha, TRUE);
    LoadPLA ("S.pla", &fun, &uscite);
    
    alpha = CreateProduct(inputs);
    for (i = 0; i<inputs; i++)
    	alpha[i] = '0';
    
    #if DEBUG_TEST
        printf("\nPunti dell'insieme S: ");
        for (i = 0; i < fun[0]->NumProducts; i++)
            printf("%s ", fun[0]->Products[i]);
        printf("\n");
    #endif
    
    dbg_printf("\nalpha: %s\n", alpha);
    
    if (fun[0]->NumProducts > 1) {
    
    	setAlpha (fun[0], alpha);
        XorInput = XorConAlpha(fun[0]);
        numRi = fun[0]->NumProducts + inputs;
        
        // Riempo la matrice, inserendo per righe i vettori
        // dell'insieme S. Se ho un don't care in posizione i
        // sul vettore j, inserisco alla riga j della matrice
        // l'i-esimo vettore canonico.
        bm = bm_new (numRi, inputs);
        RiempiMatrice (bm, XorInput);
        bm_sort_by_rows (bm);
        
        #if DEBUG_TEST
            printf ("\nLa matrice prima della gauss elimination\n");
            bm_print (bm);
        #endif

		// Riduca la matrice utilizzando
		// il metodo della Gauss Elimination.
        bm_unique_row_echelon_form (bm);
		
		#if DEBUG_TEST
            printf ("\nLa matrice dopo la gauss elimination\n");
            bm_print (bm);
        #endif
        
        dbg_printf("\nNumero di vettori linearmente indipendenti di S: %ld\n", bm->rows);

        // Ottengo le righe della matrice, 
        // che corrispondono ai vettori 
        // linearmente indipendenti dell'insieme S.
        int* rows = bm_get_row_values(bm);
        
        if (dimLs > 1)
	        result = Combination_1(rows, bm->rows, dimLs, fun[0], S, b_alpha);
        
        while (!result && dimLs > 1) {
        
	        dimLs--;

			if (dimLs > 1)
				result = Combination_1(rows, bm->rows, dimLs, fun[0], S, b_alpha);
        }
        
        if (result)
			dbg_bdd_printf(manager, result, inputs, "\nSpazio vettoriale Ls");
		
        
        for (i = 0; i < fun[0]->NumProducts; i++) {
    		DestroyProduct(&XorInput[i]);
    	}
    
		DestroyProduct(XorInput);
		DestroyProduct(&fun[0]->alpha);
		
		bm_free(bm);
		free(XorInput);
		free(rows);
    }
    
    unlink("S.pla");
    DestroyProduct(&alpha);
    DestroySOP(fun);
    free(fun);
    
    *dimResult = dimLs;
    return result;
}

DdNode* build_Ls_2(DdNode* S, int inputs, boolean b_alpha, int* dimResult) {

    char** XorInput=NULL;
	binmat *bm=NULL; MO_SOP _fun=NULL;
    int i, uscite, numRi; Product alpha = NULL;
    DdNode* result = NULL; SOP* fun = NULL;
    
    double num_points = Cudd_CountMinterm(manager, S, inputs);
	dbg_printf("\nNumero di punti di S: %d\n", (int)num_points);

	int dimS = (int)log2(num_points);
    dbg_printf("\ndimS: %d\n", dimS);

    int dimLs = dimS;

    // Scrivo S in formato pla per poi caricarlo in una SOP.
    write_bdd2pla (manager, S, "S.pla", inputs, b_alpha, TRUE);
    LoadPLA("S.pla", &_fun, &uscite); fun = _fun[0];

    alpha = CreateProduct(inputs);
    for (i = 0; i<inputs; i++)
    	alpha[i] = '0';
    
    #if DEBUG_TEST
        printf("\nPunti dell'insieme S: ");
        for (i = 0; i < fun->NumProducts; i++)
            printf("%s ", fun->Products[i]);
        printf("\n");
    #endif
    
    dbg_printf("\nalpha: %s\n", alpha);
    
    if (fun->NumProducts > 1) {
    
    	setAlpha (fun, alpha);
        XorInput = XorConAlpha(fun);
        numRi = fun->NumProducts + inputs;
        
        bm = bm_new (numRi, inputs);
        RiempiMatrice (bm, XorInput);

        int* points = bm_get_row_values(bm);
        int numPoints = bm->rows; bm_free(bm);

        if (dimLs > 1)
	        result = Combination_2(points, numPoints, dimLs, fun, S, b_alpha);

        while (!result && dimLs > 1) {
        
	        dimLs--;

			if (dimLs > 1)
				result = Combination_2(points, numPoints, dimLs, fun, S, b_alpha);
        }

        if (result)
			dbg_bdd_printf(manager, result, inputs, "\nSpazio vettoriale Ls");
        
        for (i = 0; i < fun->NumProducts; i++) {
    		DestroyProduct(&XorInput[i]);
    	}
    
		DestroyProduct(XorInput);
		DestroyProduct(&fun->alpha);
		
		free(XorInput);
		free(points);
    }

    unlink("S.pla");
    DestroyProduct(&alpha);
    DestroySOP(_fun);
    free(_fun);
    
    *dimResult = dimLs;
    return result;
}

/*DdNode* build_Ls_3(DdNode* S, int inputs, DdNode* on_set, DdNode* dc_set) {

	DdNode* u = Cudd_bddOr(manager, on_set, dc_set);
    Cudd_Ref(u); DdNode* result = NULL;

	DdNode* Lf_0 = buildLf(on_set, inputs);
    DdNode* Lf_1 = buildLf(u, inputs);
    
    int elem0 = Cudd_CountMinterm(manager, Lf_0, inputs); 
    int elem1 = Cudd_CountMinterm(manager, Lf_1, inputs);
    
    if (elem0 > elem1)
		result = extractVectorSpace(S, Lf_0, inputs);
	else
	    result = extractVectorSpace(S, Lf_1, inputs);
    
    Cudd_RecursiveDeref(manager, Lf_0);
    Cudd_RecursiveDeref(manager, Lf_1);
    
    return result;
}*/

DdNode* build_Ls_3(DdNode* S, int inputs, DdNode* on_set, DdNode* dc_set) {

	DdNode* u = Cudd_bddOr(manager, on_set, dc_set);
    Cudd_Ref(u);

	DdNode* Lf_0 = buildLf(on_set, inputs);
    DdNode* Lf_1 = buildLf(u, inputs);
    
    DdNode* result0 = extractVectorSpace(S, Lf_0, inputs);
    DdNode* result1 = extractVectorSpace(S, Lf_1, inputs);
    
    int elem0 = Cudd_CountMinterm(manager, result0, inputs); 
    int elem1 = Cudd_CountMinterm(manager, result1, inputs);
    
    Cudd_RecursiveDeref(manager, Lf_0);
    Cudd_RecursiveDeref(manager, Lf_1);
    
    return elem0 >= elem1 ? result0 : result1;
}

void quit()
{
    Cudd_Quit(manager);
}
