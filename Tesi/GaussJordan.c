#include "GaussJordan.h"

static const int g_long_length = sizeof (int);

binmat* bm_new (int rows, int cols) {

    int n, i;

    binmat *bm = (binmat*) malloc (sizeof (binmat));
    if (bm == 0L) return 0L;

    n = ((rows * cols) / 8);
    if ((n % g_long_length) != 0) {
        n /= g_long_length;
        n++;
    } else {
        n /= g_long_length;
    }

    bm->data = (int*) malloc (n * g_long_length);

    if (bm->data == 0L) {
        free (bm);
        return 0L;
    }

    bm->rows = rows;
    bm->cols = cols;

    for (i = 0; i < n; i++)
        bm->data[i] = 0;

    return bm;
}

void bm_free (binmat *bm) {
    if (bm && bm->data)
        free (bm->data);

    if (bm)
        free (bm);

    bm = NULL;
}

int bm_get (binmat *bm, int row, int col) {

    int pos, idx, eidx, val;
    if (!bm) return -1;

    // determine the position of the element
    pos = row * bm->cols + col;

    // determine the index of the array
    idx = pos / (g_long_length * 8);

    // determine the bit position of the array
    eidx = g_long_length * 8 - 1 - (pos % (g_long_length * 8));

    // get the current val
    val = (bm->data[idx] & (1 << eidx)) == 0 ? 0 : 1;

    return val;
}

int bm_get_row_value (binmat *bm, int row) {

    int rv = 0, c;

    if (!bm || row >= bm->rows) return -1;

    for (c = 0; c < bm->cols; c++)
        rv += (bm_get (bm, row, c) << bm->cols - 1 - c);

    return rv;
}

int* bm_get_row_values (binmat *bm) {

    int *values;
    int r;

    if (!bm) return 0L;
    values = (int*) malloc (sizeof (int) * (bm->rows));
    for (r = 0; r < bm->rows; r++)
        values[r] = bm_get_row_value (bm, r);
    return values;
}

int bm_set (binmat *bm, int row, int col, int value) {

    long pos, idx, eidx, val;
    if (!bm) return -1;

    // determine the position of the element
    pos = row * bm->cols + col;

    // determine the index of the array
    idx = pos / (g_long_length * 8);

    // determine the bit position of the array
    eidx = g_long_length * 8 - 1 - (pos % (g_long_length * 8));

    // get the current val
    val = (bm->data[idx] & (1 << eidx)) == 0 ? 0 : 1;

    if (value != val) { // we have to switch the bits
        bm->data[idx] ^= (1 << eidx);
    }

    return 0;
}

int bm_set_row_value (binmat *bm, int row, int value) {
    int c;
    if (!bm || row >= bm->rows) return -1;

    for (c = 0; c < bm->cols; c++) {
        bm_set (bm, row, c, ((value & (1 << bm->cols - 1 - c)) == 0 ? 0 : 1));
    }

    return 0;
}

int bm_set_row_values (binmat *bm, int *values) {
    int r;

    if (!bm) return -1;

    for (r = 0; r < bm->rows; r++) {
        bm_set_row_value (bm, r, values[r]);
    }

    return 0;
}

void swap (int* array, int a, int b) {
    int tmp = array[a];
    array[a] = array[b];
    array[b] = tmp;
}

int divide (int* array, int l, int r, int leq) {

    int idx = l, i;

    for (i = l; i < r; i++) {
        if (leq) {
            if (array[i] <= array[r]) {
                swap (array, idx, i);
                idx++;
            }
        } else {
            if (array[i] >= array[r]) {
                swap (array, idx, i);
                idx++;
            }
        }
    }
    swap (array, idx, r);
    return idx;
}

void quicksort (int* array, int l, int r, int leq) {
    int d;
    if (r > l) {
        d = divide (array, l, r, leq);
        quicksort (array, l, d - 1, leq);
        quicksort (array, d + 1, r, leq);
    }
}

int bm_sort_by_rows (binmat *bm) {  

	int *v, i;

	if (!bm) return -1;

	v = bm_get_row_values (bm);
	quicksort (v, 0, bm->rows - 1, 1);
	bm_set_row_values (bm, v);  

	free(v); return 0;
}

int bm_sort_by_rows_desc (binmat *bm) {

    if (!bm) return -1;
    int* v = bm_get_row_values (bm);

    quicksort (v, 0, bm->rows - 1, 0);
    bm_set_row_values (bm, v);

    free(v); return 0;
}

int bm_print (binmat *bm) {

    int eidx = g_long_length * 8 - 1, idx = 0;
    long l;

    if (!bm) return -1;

    for (l = 0; l < (bm->rows * bm->cols); l++, eidx--) {
        if (eidx == -1) {
            idx++;
            eidx = g_long_length * 8 - 1;
        }

        printf ("%d ", (bm->data[idx] & (1 << eidx)) == 0 ? 0 : 1);

        if (l % bm->cols == (bm->cols - 1)) printf ("\n");
    }

    return 0;
}

int bm_remove_zero_rows (binmat *bm) {

    if (!bm) return -1;

    bm_sort_by_rows_desc (bm);

    while (bm->rows > 0 && bm_get_row_value (bm, bm->rows - 1) == 0)
        bm->rows--;
        
    return 0;
}

int bm_remove_equal_rows_and_zeros (binmat *bm) {
    int current, n;
    int r;

    if (!bm) return -1;

    bm_sort_by_rows_desc (bm);

    current = bm_get_row_value (bm, 0);
    for (r = 1; r < bm->rows; r++) {
        n = bm_get_row_value (bm, r);
        if (n == current)
            bm_set_row_value (bm, r, 0);
        else
            current = n;
    }

    bm_remove_zero_rows (bm);

    return 0;
}

int bm_unique_row_echelon_form (binmat *bm) {

    int r, c, i, val, row;
    if (!bm) return -1;

    bm_remove_equal_rows_and_zeros (bm); r = 0;

    for (c = 0; c < bm->cols; c++) {

        if (bm_get (bm, r, c) == 0) continue;

        row = bm_get_row_value (bm, r);

        for (i = (r + 1); i < bm->rows; i++) {

            if (bm_get (bm, i, c) == 1) {
                val = bm_get_row_value (bm, i);
                val ^= row;
                bm_set_row_value (bm, i, val);
            }
        }

        bm_remove_equal_rows_and_zeros (bm);
        r++;
    }

    r = 0;

    for (c = 0; c < bm->cols; c++) {

        if (bm_get (bm, r, c) == 0) continue;

        row = bm_get_row_value (bm, r);

        for (i = 0; i < r; i++) {
            if (bm_get (bm, i, c) == 1) {
                val = bm_get_row_value (bm, i);
                val ^= row;
                bm_set_row_value (bm, i, val);
            }
        }

        r++;
    }

    return 0;
}

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
