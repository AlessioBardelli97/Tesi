#ifndef TESI_GAUSSJORDAN_H
#define TESI_GAUSSJORDAN_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "autosymmetry.h"
#include "binmat.h"

/**
 * Implementazione del metodo Gauss-Jordan elimination.
 * E' utilizzato per ottenere i vettori linearmente
 * indipendenti dell'insieme S.
 * @param S Insieme di vettori di partenza.
 * @param inputs Numero di componenti dei vettori in S.
 * @return Un BDD che rappresenta i vettori linearmente indipendenti di S.
 */
DdNode* get_linearly_independent_vectors (DdNode* S, int inputs);

#endif //TESI_GAUSSJORDAN_H
