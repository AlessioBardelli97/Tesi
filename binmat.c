#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "binmat.h"


inline void get_indices (int cols, int row, int col, int* idx, int* eidx) {
  int pos;

  pos = row * cols + col;

  // determine the index of the array
  *idx = pos / (g_long_length * 8);

  // determine the bit position of the array
  *eidx = g_long_length * 8 - 1 - (pos % (g_long_length * 8));
}

void swap (int* array, int a, int b) {
  int tmp = array[a];
  array[a] = array[b];
  array[b] = tmp;
}

int divide (int* array, int l, int r, int leq) {
  int idx = l;
  int i;

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

int is_normal (unsigned long value, int length) {
  long l, r;

  if (length == 2) return 1;

  // value should have leading zeros
  value &= (1 << length) - 1;

  // split value
  r = value & ((1 << length / 2) - 1);
  l = (value >> (length / 2));

  if (r == l || is_complemented (r, l, length / 2)) {
    return is_normal (r, length / 2) && is_normal (l, length / 2);
  } else return 0;
}

int is_complemented (unsigned long a, unsigned long b, int length) {
  // so complicated because of possible overflow
  b ^= ((1 << (g_long_length * 8) - 1) - 1) + (1 << (g_long_length * 8) - 1) - ((1 << length) - 1);

  return (a == ~b);
}

int is_canonical (unsigned long value, int length) {
  int k;

  // value should have leading zeros
  value &= (1 << length) - 1;

  if (length <= 1) return -1;

  // determine k
  k = -1;

  while (1) {
    if ((1 << (k + 1)) > length) return -1;

    if ((value >> (length - (1 << (k + 1)))) == 0) {
      k++;
    } else break;
  }

  if (value == canonical_value (length, k))
    return k;
  else
    return -1;
}

int canonical_value (int length, int k) {
  int r = 0;
  int t;
  int i;
  int v;
  int s;

  t = length / (1 << (k + 1));
  v = (1 << (1 << k)) - 1;

  for (i = 0; i < t; i++) {
    s = (1 << (k + 1));

    r |= (v << (i * s));
  }

  return r;
}

binmat* bm_new (int rows, int cols) {
  int n;
  int i;

  binmat *bm = malloc (sizeof (binmat));
  if (bm == 0L) return 0L;

  n = ((rows * cols) / 8);
  if ((n % g_long_length) != 0) {
    n /= g_long_length;
    n++;
  } else {
    n /= g_long_length;

  }

  bm->data = malloc (n * g_long_length);

  bm->rows = rows;
  bm->cols = cols;

  for (i = 0; i < n; i++) {
    bm->data[i] = 0;
  }

  if (bm->data == 0L) {
    free (bm);
    return 0L;
  }

  return bm;
}

void bm_free (binmat *bm) {
  if (bm && bm->data)
    free (bm->data);

  if (bm)
    free (bm);
}

int bm_array_len (binmat *bm) {
  int n;

  if (!bm) return -1;

  n = ((bm->rows * bm->cols) / 8);
  if ((n % g_long_length) != 0) {
    n /= g_long_length;
    n++;
  } else {
    n /= g_long_length;
  }

  return n;
}

long bm_elems_len (binmat *bm) {
  if (!bm) return -1;

  return bm->rows * bm->cols;
}

int bm_set (binmat *bm, int row, int col, int value) {
  long pos;
  int  idx;
  int  eidx;
  int  val;

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

int bm_get (binmat *bm, int row, int col) {
  int pos;
  int  idx;
  int  eidx;
  int  val;

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

int bm_switch (binmat *bm, int row, int col) {
  long pos;

  if (!bm) return -1;

  // determine the position of the element
  pos = row * bm->cols + col;

  bm->data[pos / (g_long_length * 8)] ^= (1 << (g_long_length * 8 - 1 - (pos % (g_long_length * 8))));

  return 0;
}

int bm_print (binmat *bm) {
  int idx = 0;
  int eidx = g_long_length * 8 - 1;
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

int bm_get_row_value (binmat *bm, int row) {
  int rv = 0;
  int c;

  if (!bm || row >= bm->rows) return -1;


  for (c = 0; c < bm->cols; c++)
  {
    rv += (bm_get (bm, row, c) << bm->cols - 1 - c);
   }
  return rv;
}

int bm_set_row_value (binmat *bm, int row, int value) {
  int c;
  if (!bm || row >= bm->rows) return -1;

  for (c = 0; c < bm->cols; c++) {
    bm_set (bm, row, c, ((value & (1 << bm->cols - 1 - c)) == 0 ? 0 : 1));
  }

  return 0;
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

int bm_set_row_values (binmat *bm, int *values) {
  int r;

  if (!bm) return -1;

  for (r = 0; r < bm->rows; r++) {
    bm_set_row_value (bm, r, values[r]);
  }

  return 0;
}

int bm_sort_by_rows (binmat *bm) {

  int* v;

  if (!bm) return -1;

   //bm_print (bm);
  v = bm_get_row_values (bm);
  quicksort (v, 0, bm->rows - 1, 1);
  bm_set_row_values (bm, v);
  free(v);

  return 0;
}

int bm_sort_by_rows_desc (binmat *bm) {
  int* v;

  if (!bm) return -1;
  v = bm_get_row_values (bm);

  quicksort (v, 0, bm->rows - 1, 0);
  bm_set_row_values (bm, v);
  free(v);

  return 0;
}

int bm_get_col_value (binmat *bm, int col) {
  int cv = 0;
  int r;

  if (!bm || col >= bm->cols) return -1;

  for (r = 0; r < bm->rows; r++)
    cv += (bm_get (bm, r, col) << ((bm->rows - 1) - r));

  return cv;
}

int bm_set_col_value (binmat *bm, int col, long value) {
  int r;

  if (!bm || col >= bm->cols) return -1;

  for (r = 0; r < bm->rows; r++) {
    bm_set (bm, r, col, ((value & (1 << bm->rows - 1 - r)) == 0 ? 0 : 1));
  }

  return 0;
}

int* bm_get_col_values (binmat *bm) {
  int *values;
  int c;

  if (!bm) return 0L;

  values = malloc (sizeof (int) * bm->cols);

  for (c = 0; c < bm->cols; c++)
    values[c] = bm_get_col_value (bm, c);

  return values;
}

int bm_has_normal_columns (binmat *bm) {
  int* cols;
  int c;
  int r = 1;

  if (!bm) return -1;

  cols = bm_get_col_values (bm);
  for (c = 0; c < bm->cols; c++) {
    if (!is_normal (cols[c], bm->rows)) {
      r = 0;
      break;
    }
  }

  free (cols);

  return r;
}

int bm_make_canonical (binmat *bm) {
  int *rows;
  int r;
  int current;
  int same = 0;

  if (!bm) return -1;

  // normal columns?
  if (bm_has_normal_columns (bm) != 1) return -1;

  // no same rows check
  rows = bm_get_row_values (bm);
  quicksort (rows, 0, bm->rows - 1, 1);

  current = rows[0];
  for (r = 1; r < bm->rows; r++) {
    if (rows[r] == current) {
      same = 1;
      break;
    }

    current = rows[r];
  }

  if (same) {
    free (rows);
    return -1;
  }

  bm_set_row_values (bm, rows);
  free (rows);

  return 0;
}

int bm_get_canonical_and_noncanonical_columns (binmat *bm, int** canonical, int** noncanonical, int* m) {
  int i, k, a;

  if (!bm) return -1;

  *m = (int)log2 (bm->rows);
  *canonical = malloc (sizeof (int) * (*m));
  if (!*canonical) return -1;

  *noncanonical = malloc (sizeof (int) * (bm->cols - (*m)));
  if (!(*noncanonical)) {
    free (*canonical);
    return -1;
  }

  for (i = 0; i < (*m); i++) (*canonical)[i] = -1;
  for (i = 0; i < (bm->cols - (*m)); i++) (*noncanonical)[i] = -1;
  
  a = 0;
  for (i = 0; i < bm->cols; i++) {
    k = is_canonical (bm_get_col_value (bm, i), bm->rows);

    if (k != -1 && (*canonical)[k] == -1) {
      (*canonical)[k] = i;
    } else {
	  (*noncanonical)[a++] = i;
    }
  }

  quicksort (*canonical, 0, *m - 1, 1);
  quicksort (*noncanonical, 0, (bm->cols - *m) - 1, 1);

  return 0;
}

int bm_print_cex (binmat *bm) {
  int *c, *nc;
  int i, j, m;

  if (!bm) return -1;

  if (bm_get_canonical_and_noncanonical_columns (bm, &c, &nc, &m) != 0) return -1;

  for (i = 0; i < (bm->cols - m); i++) {
    printf ("(");
    for (j = 0; j < m; j++) {
      if (bm_get (bm, 0, nc[i]) != bm_get (bm, (1 << (m - j - 1)), nc[i])) {
        printf ("x_%d XOR ", c[j]);
      }
    }

    if (bm_get (bm, 0, nc[i]) == 1)
      printf ("x_%d)", nc[i]);
    else
      printf ("~x_%d)", nc[i]);

    if (i != (bm->cols - m - 1))
      printf (" * ");
    else
      printf ("\n");
  }

  return 0;
}

int bm_row_xor (binmat *bm, int vector) {
  int r;
  int val;

  if (!bm) return -1;

  // cut the vector
  vector &= (1 << bm->cols) - 1;

  for (r = 0; r < bm->rows; r++) {
    val = bm_get_row_value (bm, r);
    val ^= vector;
    bm_set_row_value (bm, r, val);
  }

  return 0;
}

/**
 * INFO: this function sorts the matrix
 */
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
  int r, c, i;
  int val, row;

  if (!bm) return -1;
  bm_remove_equal_rows_and_zeros (bm);
  r = 0;
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

int bm_print_cex_from_base_and_vector (binmat *bm, int vector) {
  int *canonical;
  int *noncanonical;
  int r, c;
  int ci, ni;
  int i;
  int val;

  if (!bm) return -1;

  canonical = (int*)malloc (sizeof (int) * bm->rows);
  noncanonical = (int*)malloc (sizeof (int) * (bm->cols - bm->rows));

  ci = ni = 0;
  r = 0;

  for (c = 0; c < bm->cols; c++)
  {

    if (bm_get (bm, r, c) == 1) {
      canonical[ci++] = c;
      r++;
    } else {
      noncanonical[ni++] = c;
    }

    if (r > bm->rows) {
      for (i = c; i < bm->cols; i++)
        noncanonical[ni++] = i;
      break;
    }
  }

  printf ("ci: %d, ni: %d\n", ci, ni);
  printf ("v: %d\n", vector);

  // construct cex by paper of V. Ciriani
  for (i = 0; i < ni; i++) {
    printf ("(");
    for (r = 0; r < bm->rows; r++) {
      val = bm_get_row_value (bm, r);
      if (val & (1 << (bm->cols - 1 - noncanonical[i])) != 0) {
        printf ("x_%d XOR ", canonical[r]);
      }
    }

    if (vector & (1 << (bm->cols - 1 - noncanonical[i])) != 0) {
      printf ("x_%d)", noncanonical[i]);
    } else {
      printf ("~x_%d)", noncanonical[i]);
    }

    if (i == (ni - 1)) {
      printf ("\n");
    } else {
      printf (" * ");
    }
  }

  return 0;
}

int* bm_get_2spp_path_from_base_and_vector (binmat *bm, int vector) {
  int *path = 0L;
  int *canonical;
  int *noncanonical;
  int r, c;
  int ci, ni;
  int i;
  int a, b, neg; // a var, b var, b is complemented?
  int dim;

  if (!bm) return 0L;

  dim = (bm->cols * (bm->cols + 1)) / 2;

  path = (int*) malloc (sizeof (int) * dim);
  if (!path) return 0L;
  for (i = 0; i < dim; i++) path[i] = 0;

  canonical = (int*)malloc (sizeof (int) * bm->rows);
  noncanonical = (int*)malloc (sizeof (int) * (bm->cols - bm->rows));

  ci = ni = 0;
  r = 0;

  for (c = 0; c < bm->cols; c++)
  {

    if (bm_get (bm, r, c) == 1) {
      canonical[ci++] = c;
      r++;
    } else {
      noncanonical[ni++] = c;
    }

    if (r > bm->rows) {
      for (i = c; i < bm->cols; i++)
        noncanonical[ni++] = i;
      break;
    }
  }



  printf("\n CEX\n");
  for (i = 0; i < ni; i++) {
    a = -1; // means, that no canonical var is found yet
    b = noncanonical[i]; // b can already be determined

    for (r = 0; r < bm->rows; r++) {
      //val = bm_get_row_value (bm, r);
      //if (val & (1 << (bm->cols - 1 - noncanonical[i])) != 0)
      if (bm_get (bm, r, b) == 1)
      {
        // if we do not have found any canonical, take it
        if (a == -1) a = canonical[r];

        // if we have already a canonical var, we do not need that factor
        else a = -2;
      }
   }

    // do not care about this factor, it has two many variables
    if (a == -2) continue;

   printf("\n VECTOR : %d", vector);
    if ((vector & (1 << (bm->cols - 1 - noncanonical[i]))) != 0)
      neg = 1;
    else
      neg = -1;

    if (a == -1) // the form is just b or ~b
    {
      path[b] = neg;
      printf("(%d %d)", neg, b);
   }
    else         // the form is a$b or a$~b
    {
//      path[Spp_GetExorFromVar (a, b)] = neg;
      printf("(%d %d XOR %d)", neg, a, b);
   }
  }

  return path;
}
