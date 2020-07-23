#ifndef _DEBUG_
#define _DEBUG_

#ifdef DEBUG
#define DEBUG_TEST 1
#endif

#ifndef DEBUG
#define DEBUG_TEST 0
#endif

#define dbg_printf(ftm, ...) \
        do { if(DEBUG_TEST) printf(ftm, __VA_ARGS__); } while(0)

#define dbg_bdd_printf(manager, bdd, n, bdd_name) if(DEBUG_TEST){ printf("%s\n", bdd_name); Cudd_PrintSummary(manager, bdd, n, 0); Cudd_bddPrintCover(manager, bdd, bdd);}

#endif