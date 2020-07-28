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

// Un prodotto e' una stringa in formato PLA:
// 0: letterale negato
// 1: letterale diretto
// -: letterale assente
typedef char* Product;
typedef Product* vProduct;

typedef struct _SOP {
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
} SOP;

typedef SOP* pSOP;
typedef pSOP* MO_SOP; //Multi-Output SOP

char CharXOR (char v1, char v2);
char CharXORalpha (char v1, char v2); //lo xor con un - ritorna -
char CharNotXOR (char v1, char v2);
char CharNOT (char v1);
char CharNOTMod (char v1); //fa il NOT di - come +
char CharAND (char v1, char v2);

Product CreateProduct (int NumInputs);
void DestroyProduct (Product *p);

SOP *CreateSOP (int NumInputs, int NumProducts, int NumDCProducts);
void DestroySOP (pSOP *ppSOP);

// Carica un file in formato PLA e costruisce la MO_SOP corrispondente.
// Poiche' la nostra definizione non tiene in considerazione i don't care in uscita,
// per convenzione li considera come 0.
void LoadPLA (char *InputFile, MO_SOP* pInitGEPSOP, int *pNumOutputs);

// Esplora la MO_SOP e costruisce la PLA corrispondente
void SavePLA (MO_SOP OutSOP, int NumOutputs, char *OutputFile);


//data la matrice in echelon form definisce lo spazio affine e lo salva in funzione->CEXProducts
void CreaCEX(binmat *bm, SOP *funzione);

//Inizializza la matrice con lo xor tra on-Set e alpha e sistema i - nell'input
void RiempiMatrice(binmat *bm, vProduct *prodotti);

//fa lo XOR prima di riempire la matrice
vProduct XorConAlpha(SOP *funzione);

//salva alfa nella struct della SOP
void setAlpha(SOP *funzione, Product prodotto);

#endif
