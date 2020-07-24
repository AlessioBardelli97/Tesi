#include "GaussJordan.h"

void foo (binmat* bm, int col, int* i, int j, char* buf) {

	for (; j < col; j++) {
	
		if (buf[j] == '0')
			bm_set (bm, *i, j, 0);
		else if (buf[j] == '1')
			bm_set (bm, *i, j, 1);
		else if (buf[j] == '-') {
			
			char copy[256]; 
			memset(copy, '\0', 256);
			strcpy(copy, buf);
			
			copy[j] = '0';
			foo (bm, col, i, j, copy);
			
			(*i)++;
			
			for (int k = 0; k < j; k++) {
				if (buf[k] == '0')
					bm_set (bm, *i, k, 0);
				else if (buf[k] == '1')
					bm_set (bm, *i, k, 1); 
			}
			
			copy[j] = '1';
			foo (bm, col, i, j, copy);
			
			return;
		}
	}
}

DdNode* get_linearly_independent_vectors (DdNode* S, int inputs) {

	int col = 0, i = 0, j = 0; 
	char S_pla_name[] = "S.pla", buf[256];
	FILE* S_pla_file = NULL;
	
	printPla (manager, S_pla_name, S, inputs);
	S_pla_file = fopen (S_pla_name, "r");
	
	if (S_pla_file == NULL) {
	
		perror ("errore durante l'apertura di S.pla");
		return NULL;
	}
	
	// Leggo il numero di colonne della matrice.
	fscanf (S_pla_file, ".i %d\n", &col);
	
	// Leggo la linea ".o %d\n", che non ci serve.
	fgets (buf, 256, S_pla_file); memset (buf, '\0', 256);

	// Inizializzo la matrice. Il numero di 
	// righe possibili è dato da 2^col.
	binmat* bm = bm_new (pow (2, col), col);

	// Riempo la matrice.
	while (fgets (buf, 256, S_pla_file) != NULL) {
		
		if (memchr (buf, '-', sizeof (buf)) != NULL) {
		
			j = 0; foo (bm, col, &i, j, buf);
			
		} else {
		
			for (j = 0; j < col; j++) {	
				if (buf[j] == '0')
					bm_set(bm, i, j, 0);
				else if (buf[j] == '1')
					bm_set(bm, i, j, 1);
			}
		}
		
		i++; memset(buf, '\0', 256);
	}
	
	fclose(S_pla_file); unlink(S_pla_name);
	
	// bm_remove_zero_rows(bm);
	// bm_sort_by_rows(bm);
	
    #ifdef DEBUG
        printf("Matrice prima della Gauss-Jordan elimination:\n");
        bm_print (bm);
    #endif
    
    bm_unique_row_echelon_form (bm);

    #ifdef DEBUG
        printf ("Matrice dopo la Gauss-Jordan elimination:\n");
        bm_print (bm);
    #endif

    int* linearly_independent_vectors = bm_get_row_values (bm);
    bm_free (bm); free (linearly_independent_vectors);

    return NULL;
}
