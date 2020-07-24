#include <stdio.h>
#include <stdlib.h>
#include "logic.h"

#define LUNGHEZZA_STRINGA 50

// argv[1] = file .pla

int main(int argc, char *argv[]) {

	vProduct XorInput=NULL;
	binmat *bm=NULL; MO_SOP funzione=NULL;
	char *fileInput=NULL, *fileInputNome=NULL;
    int i, j, uscite, prodotti, numRi;
    
    fileInput     = (char*) calloc (NAME_LENGTH, sizeof (char));
    fileInputNome = (char*) calloc (NAME_LENGTH, sizeof (char));
    
    fileInput = argv[1];
    strncpy (fileInputNome, fileInput, strlen(fileInput)-4);
    
    //carico il file di ingresso in una MO_SOP
    LoadPLA (fileInput, &funzione, &uscite);
    
    for (i = 0; i < uscite; i++) {
	
        funzione[i]->NumCEXProducts = 0;
        prodotti = funzione[i]->NumProducts;
        funzione[i]->CEXLetterali = CreateProduct (funzione[i]->NumInputs);

        for (j = 0; j < funzione[i]->NumInputs; j++)
            funzione[i]->CEXLetterali[j]='-';
            
        /***
          1.  Reduced echelon form.
        ***/
        if (funzione[i]->NumProducts > 1) {
        
            setAlpha (funzione[i], funzione[i]->Products[0]);
            XorInput = XorConAlpha(funzione[i]);
            numRi = prodotti + funzione[i]->NumInputs;
            
            bm = bm_new (numRi, funzione[i]->NumInputs);
            RiempiMatrice (bm,XorInput);
            bm_sort_by_rows (bm);
            
            printf ("la matrice prima della gauss elimination\n");
            bm_print (bm);

            bm_unique_row_echelon_form (bm);
            
            printf ("la matrice dopo la gauss elimination\n");
            bm_print (bm);
            
            /***
              2. Se ho meno righe che variabili, definire spazio affine.
            ***/
            if (bm->rows <= funzione[i]->NumInputs) {
            
         		printf ("definisco il più piccolo spazio affine che contiene f\n");
            
                CreaCEX(bm, funzione[i]);
                
                printf("Numero di prodotti della CEX che rappresenta lo spazio affine: ");
                printf ("%d\n", funzione[i]->NumCEXProducts);

                for (j = 0; j < funzione[i]->NumCEXProducts; j++)
                	printf ("%s\n", funzione[i]->CEXProducts[j]);
            }
		}
	}
	
	free (fileInputNome); free (XorInput);
	DestroySOP (funzione); bm_free (bm);
	free (funzione);
	
	return 0;
}
