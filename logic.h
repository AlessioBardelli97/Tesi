#ifndef __logic_h
#define __logic_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*****************/
#include "binmat.h"
/*******/
//        col0    col1    col2...
//riga0   elem00
//riga1
//...
/*******/
#ifndef boolean
typedef enum _boolean boolean;
enum _boolean {FALSE = 0, TRUE = 1};
#endif

#define NAME_LENGTH 255
#define ROW_LENGTH 255

#define EXIT_SUCCESS           0
#define EXIT_WRONGCOMMANDLINE  1
#define EXIT_FILEACCESS        2
#define EXIT_MEMORY            3
#define EXIT_INCONSISTENCY     4
#define EXIT_TRIVIAL_INSTANCES 5

#define MAX(x,y,z) ( (x) < (y) ? ( (z) < (y) ? (y) : (z) ) : ( (z) < (x) ? (x) : (z) ) )

#define XOR(x,y) ( ( (x) && !(y) ) || ( !(x) && (y) ) )

#define DUMMY_INDEX -1

char CharXOR (char v1, char v2);
//lo xor con un - ritorna -
char CharXORalpha (char v1, char v2);
char CharNotXOR (char v1, char v2);
char CharNOT (char v1);
//fa il NOT di - come +
char CharNOTMod (char v1);
char CharAND (char v1, char v2);

// Un prodotto e' una stringa in formato PLA:
// 0: letterale negato
// 1: letterale diretto
// -: letterale assente
typedef char* Product;
typedef Product* vProduct;

Product CreateProduct (int NumInputs);

void DestroyProduct (Product *p);


typedef struct _SOP SOP;
typedef SOP* pSOP;
typedef pSOP* vpSOP;
typedef vpSOP MO_SOP; //Multi-Output SOP

struct _SOP
{
  int NumInputs;
  int NumProducts; // Numero dei prodotti
  Product *Products;   // Vettore dei prodotti (lungo NumProducts)
                       // Data una PLA, contiene le sottostringhe iniziali di ogni riga della PLA stessa
  int NumDCProducts; // Numero dei prodotti "don't care"
  Product *DCProducts;   // Vettore dei prodotti "don't care" (lungo NumDCProducts)
                         // Data una PLA, contiene le sottostringhe iniziali di ogni riga della PLA stessa
/******************/
  int NumCEXProducts;  //numero dei prodotti della CEX form (==0 se non D-Red)???
  Product *CEXProducts;
  Product OutCEX;
  int *indiciCEX;
  int *variabiliNC;//memorizza la variabile non canonica del corrispondente prodotto CEX
  Product CEXLetterali;//Vettore dei prodotti della CEX form (1--1- X0 XOR X3; 1--0- X0 XOR not(X3))
  Product alpha;
  Product varEliminare; //Vettore di NumInputs elementi che indica le var da eliminare (1 elimina,0 tieni)
};

SOP *CreateSOP (int NumInputs, int NumProducts, int NumDCProducts);

void DestroySOP (pSOP *ppSOP);


typedef struct _GEPSOP GEPSOP;
typedef GEPSOP*  pGEPSOP;
typedef pGEPSOP* vpGEPSOP;
typedef vpGEPSOP MO_GEPSOP; //Multi-Output GEPSOP

struct _GEPSOP
{
  int NumInputs;

// Una GEPSOP puo' essere:
// 1) una SOP phi
  SOP *phi;
  // Nome della SOP (per i file BLIF)
  // La convenzione e' che si chiama s, seguita da una lettera per ogni livello:
  // la lettera indica se si e' seguito il ramo affermato ('d') o quello negato ('n')
  // della proiezione, oppure il resto ('r')
  char phiName[NAME_LENGTH];

// 2) uno XOR fra una variabile var e un prodotto di letterali prod,
//    due o tre GEPSOP (le proiezioni psi e psi_neg e l'eventuale resto rho)
  int var;
  char OperandType; // Dice se Product va inteso come prodotto ('A') o XOR ('X')
  Product p;
  GEPSOP *psi;     // Proiezione sullo spazio con var = p
  GEPSOP *psi_neg; // Proiezione sullo spazio con var = !p
  GEPSOP *rho;
};

pGEPSOP CreateGEPSOP (SOP *phi);

void DestroyGEPSOP (pGEPSOP *ppGEPSOP);


// Carica un file in formato PLA e costruisce la MO_SOP corrispondente.
// Poiche' la nostra definizione non tiene in considerazione i don't care in uscita,
// per convenzione li considera come 0.
void LoadPLA (char *InputFile, MO_SOP* pInitGEPSOP, int *pNumOutputs);

// Esplora la MO_SOP e costruisce la PLA corrispondente
void SavePLA (MO_SOP OutSOP, int NumOutputs, char *OutputFile);

// Costruisce la MO_GEPSOP che contiene come uscite banalmente
// le singole uscite della MO_SOP data
MO_GEPSOP BuildMO_GEPSOPfromMO_SOP (MO_SOP S, int NumOutputs);

// Costruisce la MO_SOP che contiene come uscite le SOP componenti delle foglie della MO_GEPSOP
// in ordine di visita diretto-negato-resto per le varie uscite della MO_GEPSOP
MO_SOP BuildMO_SOPfromMO_GEPSOP (MO_GEPSOP S, int NumOutputs);

//void PrintSOP (SOP* PLA);


//SOP *ExtractOutputSOP (SOP *InitPLA, unsigned int o);

// Finds the most frequent pair in the given MO_SOP.
// Returns it as (pv1,pv2) in lexicographic order.
void FindMostFrequentPair (MO_SOP S, int NumOutputs, int *pv1, int *pv2);

// Project a SOP w.r.t. v1 XOR v2; the SOP must be already inserted in a GEPSOP data structure
void ProjectSOPwrtVar (GEPSOP *G, int v1, int v2, boolean Remainder);

// Finds the most frequent triplet in the given MO_SOP.
// Returns as pv1 the variable which appears most often together with one of the other two
// and as pv2 and pv3 the other variables in lexicographic order.
void FindMostFrequentTriplet (MO_SOP SOP, int NumOutputs, int *pv1, int *pv2, int *pv3);

// Project a trivial GEPSOP (i.e. a SOP) w.r.t. v1 == (v2 AND v3) and v1 == NOT (v2 AND v3)
void ProjectSOPwrtAND (GEPSOP *G, int v1, int v2, int v3, boolean Remainder);

// Project a trivial GEPSOP (i.e. a SOP) v1 == (v2 XOR v3) and v1 == NOT (v2 XOR v3)
void ProjectSOPwrtXOR (GEPSOP *G, int v1, int v2, int v3, boolean Remainder);

void Print_MO_GEPSOP_Blif (MO_GEPSOP G, int NumOutputs, char *BlifFile);

// Replace the SOP components in the given GEPSOP with the given SOPs
void ReloadMO_GEPSOPfromMO_SOP (MO_SOP S, int no, MO_GEPSOP G);

void ParseEspresso (char *OutputFile, int *pCostVR, double *pTimeVR);

/**************************/
//data la matrice in echelon form definisce lo spazio affine e lo salva in funzione->CEXProducts
void CreaCEX(binmat *bm, SOP *funzione);
//raggruppa i prodotti della CEX che differiscono di 2 soli elementi e li ordina da quello con meno variabili a quello con più
//void OttimizzaCEX(SOP *funzione);
void OttimizzaCEX(SOP *funzione);

//Inizializza la matrice con lo xor tra on-Set e alpha e sistema i - nell'input
void RiempiMatrice(binmat *bm, vProduct *prodotti);
//stampa su file tutte le possibili combinazioni di stringhe di n bit di 0 e 1, con numero di 1 dispari se tipo =='d', pari se tipo=='p'
void ScriviSuFilegeneraXOR(char *file,int l, char* tipo, char* prodotto, int numeroBit);
//salva in prodottiXOR tutte le possibili combinazioni di stringhe di n bit di 0 e 1, con numero di 1 dispari o pari
void generaXOR(int l, char* tipo, char* prodotto, int numeroBit, int *lung, vProduct prodottiXOR);
//fa lo XOR prima di riempire la matrice
vProduct XorConAlpha(SOP *funzione);

//void Moltiplica(char* membrospazio, char* punto);
//restituisce d se ha un numero dispari di 1, altrimenti p
char Parity(char* punto);
//salva alfa nella struct della SOP
void setAlpha(SOP *funzione, Product prodotto);
//dato un prodotto conta il numero di variabili non -
int ContaVariabiliDefinite(Product prodotto);
//scambia nell'array prodotti quelli in pos p e q
void ScambiaProdotti(vProduct Prodotti, Product uscita,int *varNC,int p, int q);
//quicksort dell'array della CEX
int divideCEX (vProduct Prodotti, Product uscita, int *varNC,int l, int r, int leq);
void quicksortCEX (vProduct Prodotti, Product uscita,int *varNC,int l, int r, int leq);
//se la riduzione è -2D elimina i prodotti CEX a più di 2 elementi e riabilita le variabili che non sono più da eliminare
void TipoRiduzione(SOP *funzione,char* tipo);
//conta quanti prodotti del DC set intersecano con il prodottoCEX
int ContaProssimiDC(SOP *funzione, Product prodottoCEX, char segno);
//fa una prima scrematura del DC-set con i letterali singoli della CEX
void EliminaLetteraliDaDCSet(SOP *funzione);
//trova l'intersezione del DC-set con il membro CEX in posizione prodottoCEX
int TrovaDCSet(SOP *funzione, int prodottoCEX, char segno);
//crea la pla separata per la funzione riducibile e restituisce il nome del file dell'ottimizzata da espresso
char* MinimizzaSingolaUscitaRid(SOP *funzione,char *nomeFile);
//crea la pla separata per la funzione e restituisce il nome del file dell'ottimizzata da espresso
char* MinimizzaSingolaUscita(SOP *funzione,char *nomeFile);
//scrive su file il risultato di espresso per la pla separata ottimizzata
void ScriviFunzRidotta(char *file,char *PlaRidottaMinEspresso);
//scrive su file blif l'espressione relativa allo spazio affine dell'uscita uscita
//void ScriviSpazioAffine(char* file,SOP *funzione,int uscita);
//scrive un'unica PLA ove nelle funzioni ridotte le variabili da eliminare sono sostituite con -
void SavePLADred(MO_SOP OutSOP, int NumOutputs, char *OutputFile);
#endif
