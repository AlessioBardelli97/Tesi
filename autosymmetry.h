#ifndef _AUTOSYMMETRY_
#define _AUTOSYMMETRY_

#include <stdlib.h>
#include <cudd.h>

#include "equations.h"

DdManager* manager;

void init();

DdNode* buildNewF(DdNode* f, DdNode* lf, int inputs);

DdNode* restrictionFunction(DdNode* u, int *cv, int inputs);

int* computeCanonicalVariables(DdNode* lf, int i, int inputs, int *cv);

Eqns_t* reductionEquations(DdNode* h, int i, int inputs, EQ_manager *eq_man);

DdNode* buildLf(DdNode* g1, int inputs);

DdNode* buildS(DdNode* u, DdNode* g1, int inputs);

DdNode* extractVectorSpace(DdNode* S, DdNode* lf, int inputs);

void quit();

#endif