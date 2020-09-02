#ifndef _AUTOSYMMETRY_
#define _AUTOSYMMETRY_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "equations.h"
#include "logic.h"
#include "parser.h"

DdManager* manager;

void init();

DdNode* buildNewF(DdNode* f, DdNode* lf, int inputs);

DdNode* restrictionFunction(DdNode* u, int *cv, int inputs);

int* computeCanonicalVariables(DdNode* lf, int i, int inputs, int *cv);

Eqns_t* reductionEquations(DdNode* h, int i, int inputs, EQ_manager *eq_man);

DdNode* buildLf(DdNode* g1, int inputs);

DdNode* buildS(DdNode* u, DdNode* g1, int inputs);

DdNode* extractVectorSpace(DdNode* S, DdNode* lf, int inputs);

/******************************************************************************/

/*
 * Euristica per il calcolo di Ls.
 * Partendo dai vettori linearmente indipendenri di S
 * si considerano un sotto insieme di questi e si vede se lo spazio generato è 
 * contenuto o uguale a S, in caso affermativo ci si ferma altrimenti 
 * si considera un altro sottoinsieme. 
 */
DdNode* build_Ls_1(DdNode* S, int inputs, boolean b_alpha, int* dimResult);

/*
 * Procedura ottima per il calcolo di Ls. Partendo dai punti di S
 * si considerano un sotto insieme linearmente indipendenti di questi 
 * e si vede se lo spazio generato è contenuto o uguale a S, 
 * in caso affermativo ci si ferma altrimenti 
 * si considera un altro sottoinsieme. 
 */
DdNode* build_Ls_2(DdNode* S, int inputs, boolean b_alpha, int* dimResult);

/*
 * Euristica per il calcolo di Ls. 
 * Simile alla funzione extractVectorSpace,
 */
DdNode* build_Ls_3(DdNode* S, int inputs, DdNode* on_set, DdNode* dc_set, int* dimResult);

/******************************************************************************/

void quit();

#endif
