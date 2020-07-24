//#include <cudd.h>

static const int g_long_length = sizeof (int);

struct _binmat {
  unsigned long  rows;
  unsigned long  cols;
  int* data;
};

typedef struct _binmat binmat;

/**
 * classical swap with temp variable
 * which swaps array element at index a
 * with array element at index b.
 */
void swap (int* array, int a, int b);

/**
 * devides an array into two parts
 * depending on leq, which means, if leq == 1
 * in the first part are all the values
 * which are less equal than the element of the
 * middle in the array and vice versa.
 */
int divide (int* array, int l, int r, int leq);

/**
 * classical quicksort algorithm which sorts
 * the array from index position l to index position
 * r, depending on leq in ascending (leq == 1) or descending
 * order (leq == 0).
 */
void quicksort (int* array, int l, int r, int leq);

/**
 * Checks whether the vector value is normal.
 *
 * @see [LuPa99, Definition 1]
 */
int is_normal (unsigned long value, int length);

/**
 * Checks whether a == ~b considering only the last
 * "length" bits of their bit representations.
 *
 * For example:
 *   a = 3  (0000...00011)
 *   b = 12 (0000...01100)
 * It is: (a == ^b) is false, but
 *        is_complemented (a, b, 4) is true.
 */
int is_complemented (unsigned long a, unsigned long b, int length);

/**
 * Checks whether the vector value is k-canonical
 * for a given length.
 *
 * @return  k, if vector is k-canonical
 *         -1, if vector is not canonical
 * @see [LuPa99, Definition 3]
 */
int is_canonical (unsigned long value, int length);

/**
 * Generates a k-canonical vector of
 * a given length.
 *
 * For example:
 *  canonical_value (8, 1) = 0..011001100
 */
int canonical_value (int length, int k);

///* creation *///
/**
 * creates a new matrix of dimension
 * rows x cols. All the necessary memory
 * is allocated for the binmat struct and
 * the long* array containing the elements
 * of the matrix.
 *
 * You cannot change the dimension of the
 * matrix after creation. Probably you can
 * decrease the rows by calling some routines.
 *
 * All elements in the matrix are set to 0.
 *
 * @see bm_free
 * @see bm_new_from_zdd
 */
binmat* bm_new (int rows, int cols);

/**
 * creates a new matrix of a ZDD. All
 * possible paths are iterated and translated
 * to bitvectors. You need to specify the
 * number of variables. This is corresponding to
 * the number of cols of the matrix. The number
 * of points instead is corresponding to the number
 * of rows.
 *
 * @see bm_free
 */
//binmat* bm_new_from_zdd (DdManager *manager, DdNode *node, int variableLength);

/**
 * Frees all the memory which was used
 * for constructing the matrix.
 */
void bm_free (binmat *bm);

/* information */
/**
 * Returns the size of the internal
 * long* array which holds the elements
 * of the matrix.
 */
int bm_array_len (binmat *bm);

/**
 * Returns the number of elements
 * containing in the matrix, of course
 * it is rows x cols.
 */
long bm_elems_len (binmat *bm);

///* set/get on elements *///
/**
 * Sets a value of an element in the matrix.
 * In order to win performance you better can
 * call bm_switch on a newly created matrix
 * which elements are all set to zero.
 *
 * @see bm_switch
 */
int bm_set (binmat *bm, int row, int col, int value);

/**
 * Gets the value of an element in the matrix.
 */
int bm_get (binmat *bm, int row, int col);

/**
 * Switches an element in the matrix, so if it is
 * 0 it becomes 1 and vice versa. This call is
 * faster than bm_set using simply the XOR operator.
 */
int bm_switch (binmat *bm, int row, int col);

///* modify and info about rows and cols *///
/**
 * Returns the integer representation of a row, which
 * is for the row 1 0 1 0 for example the decimal
 * value 10.
 */
int bm_get_row_value (binmat *bm, int row);

/**
 * Sets the row value of a row by reading the
 * value as bitvector. For example the value 12
 * is the bitvector 1 1 0 0.
 */
int bm_set_row_value (binmat *bm, int row, int value);

/**
 * Returns all row values in an array, which length
 * is the number of rows, of course.
 * The index of the array corresponds to the index of the
 * row. By the way, the memory of the constructed array
 * is much larger than the memory allocated for the matrix.
 */
int* bm_get_row_values (binmat *bm);

/**
 * In regard to bm_get_row_values we can set all row
 * values by calling this routing. This could be helpful
 * when operating this way:
 *
 *  1. Get all rows
 *  2. Modifying all rows (e.g. sort them)
 *  3. Set all rows
 */
int bm_set_row_values (binmat *bm, int* values);

/**
 * The matrix is sorted by its integer representation
 * of the rows ascending.
 */
int bm_sort_by_rows (binmat *bm);

/**
 * The matrix is sorted by its integer representation
 * of the rows descending.
 */
int bm_sort_by_rows_desc (binmat *bm);

/**
 * As bm_get_row_value, but returns the integer
 * representation of a column.
 */
int bm_get_col_value (binmat *bm, int col);

/**
 * As bm_set_row_value, but sets the column
 * values by the integer representation of value.
 */
int bm_set_col_value (binmat *bm, int col, long value);

/**
 * As bm_get_row_values but returns an array
 * with all integer representations of the
 * columns of the array.
 */
int* bm_get_col_values (binmat *bm);

///* modify and info about whole matrix *///
/**
 * Checks, wether a matrix has only normal
 * columns.
 *
 * @see [LuPa99, Definition 4]
 */
int bm_has_normal_columns (binmat *bm);

/**
 * If the matrix has only normal colums
 * and does not contain equal rows,
 * the matrix is sorted by the integer
 * representation of its rows and is canonical
 * then.
 *
 * @see [LuPa99, Definition 4]
 * @return 0  if the matrix is canonical after this step
 *         -1 if the matrix cannot transform in a canonical one,
 *            because either the columns are not normal or it is
 *            containing equal rows.
 */
int bm_make_canonical (binmat *bm);

/**
 * Returns the canonical and noncanonical columns of a
 * canonical matrix. The memory for canonical and noncanonical
 * arrays is allocated in the routine and the arrays
 * are ordered by its values, which are the indexes of the columns.
 * The arrays are disjoint per definition and m is the size
 * of the canonical array, so the size of the noncanonical array
 * can be computed by bm->cols - m.
 *
 * @see [LuPa99, Definition 9]
 */
int bm_get_canonical_and_noncanonical_columns (binmat *bm, int** canonical, int** noncanonical, int* m);

/**
 * Transforms all rows by applying
 * vector XOR row to each row.
 */
int bm_row_xor (binmat *bm, int vector);

/**
 * Removes all rows of the matrix
 * which contain only zeros.
 */
int bm_remove_zero_rows (binmat *bm);

/**
 * Removes all equal rows of the matrix
 * and the rows which contain only zeros.
 */
int bm_remove_equal_rows_and_zeros (binmat *bm);

/**
 * Transforms the matrix in the canonical
 * reduced row echelon form.
 *
 * It could be, that the size of the matrix
 * changed after calling this routing, because
 * it computes the base of the vector space
 * of the matrix.
 */
int bm_unique_row_echelon_form (binmat *bm);

/**
 * Given a matrix in reduced echelon form, which
 * is probably a base of the vector space of the
 * original matrix and a vector a pseudoproduct
 * can be calculated.
 *
 * It is returnd as the 2-pseudoproduct represention
 * which is used in the SppProject tool.
 *
 * @see [Ci03, Definition 10]
 */
int* bm_get_2spp_path_from_base_and_vector (binmat *bm, int vector);

///* debugging *///
/**
 * Prints the matrix to STDOUT.
 *
 * Example of output of the 3 x 3 unit matrix.
 *
 * 1 0 0
 * 0 1 0
 * 0 0 1
 */
int bm_print (binmat *bm);

/**
 * Prints the CEX of the matrix to STDOUT which is describing
 * a pseudocube. So the matrix has to be canonical.
 *
 * @see [LuPa99, Definition 9]
 */
int bm_print_cex (binmat *bm);

/**
 * Prints the CEX to STDOUT given the base of a vectorspace
 * and a corresponding vector.
 *
 * @see [Ci03, Definition 10]
 * @see bm_get_2spp_path_from_base_and_vector
 */
int bm_print_cex_from_base_and_vector (binmat *bm, int vector);

/**
 * REFERENCES
 *
 * [LuPa99] F. Liccio, L. Pagli: "On a New Boolean Function with Applications", IEEE Transactions
 *          On Computers" 48(3), March 1999.
 *
 * [Ci03]   V. Ciriani: "Synthesis of SPP Three-Level Logic Networks Using Affine Spaces", IEEE Transactions
 *          On Computer-Aided Design of Integrated Circuits and Systems" 22(10), October 2003.
 */
