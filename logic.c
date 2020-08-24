#include "logic.h"
#include <string.h>
#include <math.h>

char CharXOR (char v1, char v2)
{
  return ( (v1 == v2) ? '0' : '1' );
}

char CharXORalpha (char v1, char v2)
{
    if(v1==v2)
        return '0';
    else if((v1=='-')||(v2=='-'))
        return '-';
    else
        return '1';
}

Product CreateProduct (int NumInputs)
{
  Product p = (Product) calloc (NumInputs+1,sizeof(char));
  if (p == NULL) {
    printf("Not enough memory to allocate a product!\n");
    exit(EXIT_MEMORY);
  }
  return p;
}


void DestroyProduct (Product* p) {
  free(*p);
  *p = NULL;
}

SOP* CreateSOP (int NumInputs, int NumProducts, int NumDCProducts) {
  int i;

  SOP* S = (SOP*) malloc(sizeof(SOP));
  if (S == NULL) {
    printf("Insufficient memory to allocate a SOP!\n");
    exit(EXIT_MEMORY);
  }

  S->NumInputs = NumInputs;
  S->NumProducts = NumProducts;
  S->Products = (vProduct) calloc(NumProducts,sizeof(Product));
  if (S->Products == NULL) {
    printf("Insufficient memory to allocate the Product vector of a SOP!\n");
    exit(EXIT_MEMORY);
  }

  for (i = 0; i < NumProducts; i++)
    S->Products[i] = CreateProduct(NumInputs);

  S->NumDCProducts = NumDCProducts;
  S->DCProducts = (vProduct) calloc(NumDCProducts,sizeof(Product));
  if (S->DCProducts == NULL) {
    printf("Insufficient memory to allocate the DCProduct vector of a SOP!\n");
    exit(EXIT_MEMORY);
  }

  for (i = 0; i < NumDCProducts; i++)
    S->DCProducts[i] = CreateProduct(NumInputs);

  return S;
}

void DestroySOP (pSOP *ppSOP) {
  int i;
  pSOP pS = *ppSOP;

  for (i = 0; i < pS->NumProducts; i++)
    if (pS->Products[i] != NULL) DestroyProduct(&pS->Products[i]);
  if (pS->Products != NULL) free(pS->Products);

  for (i = 0; i < pS->NumDCProducts; i++)
    if (pS->DCProducts[i] != NULL) DestroyProduct(&pS->DCProducts[i]);
  if (pS->DCProducts != NULL) free(pS->DCProducts);

  free(pS);
  *ppSOP = NULL;
}


// Carica un file in formato PLA e costruisce la MO_SOP corrispondente.
// Poiche' la nostra definizione di MO_SOP non tiene in considerazione i don't care in uscita,
// per convenzione li considera come 0.

void LoadPLA (char *InputFile, MO_SOP *pS, int *pNumOutputs)
{
  FILE *fInputFile;
  char tmp[NAME_LENGTH], tmp2[NAME_LENGTH];
  boolean done;
  int p, o;
  int NumInputs, NumOutputs;
  int *NumProducts, *NumDCProducts;


  fInputFile = fopen(InputFile,"r");
  if (fInputFile == NULL)
  {
    printf("File %s could not be opened!\n",InputFile);
    exit(EXIT_FILEACCESS);
  }


  // Legge dal file PLA il numero di ingressi e uscite
  NumInputs   = 0;
  NumOutputs  = 0;
  done = FALSE;
  while(fscanf(fInputFile, "%s\n",tmp)!= EOF && !done)
  {
    if(tmp[0]=='.')
      {
        // inizio di una sequenza speciale
        switch(tmp[1])
          {
          case 'i'://.input
            {
              if(tmp[2]!=0) // Ci sono anche comandi che COMINCIANO con .i: non ci interessano
                fgets(tmp, 256, fInputFile);
              else
              {
                  fscanf(fInputFile, "%d\n",&NumInputs);
                  done = ( NumInputs * NumOutputs != 0 );
              }
            }break;

          case 'o'://.output
            {
              if(tmp[2]!=0) // Ci sono anche comandi che COMINCIANO con .i: non ci interessano
                fgets(tmp, 256, fInputFile);
              else
              {
                fscanf(fInputFile,"%d\n",&NumOutputs);
                done = ( NumInputs * NumOutputs != 0 );
              }
            }break;
          case 'e':
            {
              // finito di leggere il file pla: posso terminare
              done = 1;
              break;
            }
          default:
            {
              // se son finito qui c'e' un'opzione non riconosciuta o non rilevante
            }break;
          }// fine switch(tmp[1])
      }
  } // fine while(scanf != EOF)


  *pNumOutputs = NumOutputs;

  *pS = (MO_SOP) calloc(NumOutputs,sizeof(pSOP));
  if (*pS == NULL)
  {
    printf("Insufficient memory to allocate a multiple-output SOP!\n");
    exit(EXIT_MEMORY);
  }

  NumProducts = (int *) calloc(NumOutputs,sizeof(int));
  if (NumProducts == NULL)
  {
    printf("Insufficient memory to allocate a vector of number of products for each output of the MO_SOP!\n");
    exit(EXIT_MEMORY);
  }

  NumDCProducts = (int *) calloc(NumOutputs,sizeof(int));
  if (NumDCProducts == NULL)
  {
    printf("Insufficient memory to allocate a vector of number of don't care products for each output of the MO_SOP!\n");
    exit(EXIT_MEMORY);
  }

  for (o = 0; o < NumOutputs; o++)
  {
    NumProducts[o] = 0;
    NumDCProducts[o] = 0;
  }

  rewind(fInputFile);

  p = 0;
  done = 0;
  while(fscanf(fInputFile, "%s\n",tmp)!= EOF && !done)
    {
      if(tmp[0]=='.')
        {
          // inizio di una sequenza speciale
          switch(tmp[1])
            {
            case 'i'://.input
              {
                if(tmp[2]!=0) // Ci sono anche comandi che COMINCIANO con .i: non ci interessano
                  fgets(tmp, 256, fInputFile);
                else
                {
                    fscanf(fInputFile, "%d\n",&NumInputs);
                }
              }break;

            case 'o'://.output
              {
                if(tmp[2]!=0) // Ci sono anche comandi che COMINCIANO con .i: non ci interessano
                  fgets(tmp, 256, fInputFile);
                else
                {
                  fscanf(fInputFile,"%d\n",&NumOutputs);
                }
              }break;
            case 'e':
              {
                // finito di leggere il file pla: posso terminare
                done = 1;
                break;
              }
            case 'p':
              {
                // 3 matching cases: .p, .pair, .phase
                if(tmp[2]==0)
                  {
                    //.p
                    fscanf(fInputFile,"%d\n",&p);
                  }
                else if(tmp[2]=='h')
                  {
                    //.phase
                    // da capire bene implementazione da controllare
                    fscanf(fInputFile,"%s\n", tmp2);
                  }
                else
                  {
                    // .pair
                    // da capire bene
                    fscanf(fInputFile,"%d\n",&p);
                  }
              }break;
            default:
              {
                // se son finito qui c'e' un'opzione non riconosciuta o non rilevante
              }break;
            }// fine switch(tmp[1])
        }
      else if (tmp[0] == '#')
        {
          // stampo il commento PLA
          do
            {
              fgets(tmp, 256, fInputFile);
            } while(strlen(tmp)==255);
        }
      else
        {
          // un minterm
          fscanf(fInputFile, "%s\n",tmp2);
          for (o = 0; o < NumOutputs; o++)
          {
            if (tmp2[o] == '1') NumProducts[o]++;
            if (tmp2[o] == '-') NumDCProducts[o]++;
          }
      }
    } // fine while(scanf != EOF)

  for (o = 0; o < NumOutputs; o++)
  {
    (*pS)[o] = CreateSOP(NumInputs,NumProducts[o],NumDCProducts[o]);

    // Riazzeriamo il numero dei prodotti perche' poi leggendoli ci serve il numero stesso
    // come contatore per scriverli nella SOP
    NumProducts[o] = 0;
    NumDCProducts[o] = 0;
  }

  rewind(fInputFile);

  p = 0;
  done = 0;
  while(fscanf(fInputFile, "%s\n",tmp)!= EOF && !done)
    {
      if(tmp[0]=='.')
        {
          // inizio di una sequenza speciale
          switch(tmp[1])
            {
            case 'i'://.input
              {
                if(tmp[2]!=0) // Ci sono anche comandi che COMINCIANO con .i: non ci interessano
                  fgets(tmp, 256, fInputFile);
                else
                {
                    fscanf(fInputFile, "%d\n",&NumInputs);
                }
              }break;

            case 'o'://.output
              {
                if(tmp[2]!=0) // Ci sono anche comandi che COMINCIANO con .i: non ci interessano
                  fgets(tmp, 256, fInputFile);
                else
                {
                  fscanf(fInputFile,"%d\n",&NumOutputs);
                }
              }break;
            case 'e':
              {
                // finito di leggere il file pla: posso terminare
                done = 1;
                break;
              }
            case 'p':
              {
                // 3 matching cases: .p, .pair, .phase
                if(tmp[2]==0)
                  {
                    //.p
                    fscanf(fInputFile,"%d\n",&p);
                  }
                else if(tmp[2]=='h')
                  {
                    //.phase
                    // da capire bene implementazione da controllare
                    fscanf(fInputFile,"%s\n", tmp2);
                  }
                else
                  {
                    // .pair
                    // da capire bene
                    fscanf(fInputFile,"%d\n",&p);
                  }
              }break;
            default:
              {
                // se son finito qui c'e' un'opzione non riconosciuta o non rilevante
              }break;
            }// fine switch(tmp[1])
        }
      else if (tmp[0] == '#')
        {
          // stampo il commento PLA
          do
            {
              fgets(tmp, 256, fInputFile);
            } while(strlen(tmp)==255);
        }
      else
        {
          // un minterm
          fscanf(fInputFile, "%s\n",tmp2);
          for (o = 0; o < NumOutputs; o++)
          {
            if (tmp2[o] == '1')
            {
              p = NumProducts[o];
              NumProducts[o]++;
              // Pare che copiando una stringa statica lunga su una dinamica corta, non si fermi
              // sul terminatore '\0', per cui bisogna indicare la lunghezza esplicitamente
              strncpy((*pS)[o]->Products[p],tmp,NumInputs+1);
            }
            if (tmp2[o] == '-')
            {
              p = NumDCProducts[o];
              NumDCProducts[o]++;
              // Pare che copiando una stringa statica lunga su una dinamica corta, non si fermi
              // sul terminatore '\0', per cui bisogna indicare la lunghezza esplicitamente
              strncpy((*pS)[o]->DCProducts[p],tmp,NumInputs+1);
            }
          }
      }
    } // fine while(scanf != EOF)

  free(NumProducts);
  free(NumDCProducts);

  fclose(fInputFile);
}

// Salva su file la PLA corrispondente alla MO_SOP data.
void SavePLA (MO_SOP OutSOP, int NumOutputs, char *OutputFile)
{
  FILE *fOutputFile;
  int NumProducts;
  char *ProductOutput;
  int p, o;


  fOutputFile = fopen(OutputFile,"w+");
  if (fOutputFile == NULL)
  {
    printf("File %s could not be opened!\n",OutputFile);
    exit(EXIT_FILEACCESS);
  }

  fprintf(fOutputFile,".i %d\n",OutSOP[0]->NumInputs); // Assume che il numero di ingressi sia lo stesso per tutte le uscite
  fprintf(fOutputFile,".o %d\n",NumOutputs);

  ProductOutput = (char *) calloc(NumOutputs+1,sizeof(char));
  if (ProductOutput == NULL)
  {
    printf("Not enough memory to allocate vector of char ProductOutput!\n");
    exit(EXIT_MEMORY);
  }

  // Il numero di prodotti attualmente e' la somma dei NumProducts delle singole SOP di ogni uscita,
  // cioe' ogni occorrenza sta a se'.
  NumProducts = 0;
  for (o = 0; o < NumOutputs; o++)
    NumProducts += OutSOP[o]->NumProducts;
  fprintf(fOutputFile,".p %d\n",NumProducts);

  if (NumProducts == 0)
  {
    for (p = 0; p < OutSOP[0]->NumInputs; p++)
      fprintf(fOutputFile,"-");
    fprintf(fOutputFile," ");
    for (p = 0; p < NumOutputs; p++)
      fprintf(fOutputFile,"0");
    fprintf(fOutputFile,"\n");
  }

  for (o = 0; o < NumOutputs; o++)
    ProductOutput[o] = '0';
  ProductOutput[NumOutputs] = '\0';

  for (o = 0; o < NumOutputs; o++)
  {
    for (p = 0; p < OutSOP[o]->NumProducts; p++)
    {
      fprintf(fOutputFile,"%s",OutSOP[o]->Products[p]);
      fprintf(fOutputFile," ");

      ProductOutput[o] = '1';
      fprintf(fOutputFile,"%s",ProductOutput);
      ProductOutput[o] = '0';
      fprintf(fOutputFile,"\n");
    }
    for (p = 0; p < OutSOP[o]->NumDCProducts; p++)
    {
      fprintf(fOutputFile,"%s",OutSOP[o]->DCProducts[p]);
      fprintf(fOutputFile," ");

      ProductOutput[o] = '-';
      fprintf(fOutputFile,"%s",ProductOutput);
      ProductOutput[o] = '0';
      fprintf(fOutputFile,"\n");
    }
  }

  fprintf(fOutputFile,".e\n");

  free(ProductOutput);

  fclose(fOutputFile);
}

void CreaCEX(binmat *bm, SOP *funzione)
{
    int *canoniche,*ncanoniche;
    char tempSegno;
    int numVariabili,numCan,numNCan,g,contatore1,contatore2,appog;
    numVariabili=bm->cols;
    g=0;
    int i,y,numeroCEX=0;

    numCan=bm->rows;
    numNCan=(numVariabili-numCan);
    funzione->OutCEX=CreateProduct(numNCan);
    funzione->NumCEXProducts=numNCan;
    
    /*funzione->CEXProducts = (vProduct) calloc(funzione->NumCEXProducts,sizeof(Product));
    for (i = 0; i < funzione->NumCEXProducts; i++) {

    	funzione->CEXProducts[i] = CreateProduct(numVariabili);
            
        for (y = 0; y < numVariabili; y++) {
            
            //lo inizializzo a vuoto 0
            funzione->CEXProducts[i][y] = '0';
        }
    }*/
    
    funzione->CEXProducts = NULL;
    
    funzione->variabiliNC = (int*)calloc(numNCan,sizeof(int));
    canoniche = (int*) calloc (numCan,sizeof(int));
    ncanoniche = (int*) calloc (numNCan,sizeof(int));

    for(contatore1=0;contatore1<numCan;contatore1++) {
		appog=bm_get_row_value(bm, contatore1);
		y=log2(appog);
		canoniche[contatore1]=(numVariabili-1)-y;
    }
    
    for(contatore1=0;contatore1<numVariabili;contatore1++) {
        i=0;
        for(contatore2=0;contatore2<numCan;contatore2++)
            if(contatore1==canoniche[contatore2])
                i=1;
        // non e' canonica
        if(i==0){
        	ncanoniche[g]=contatore1;
            g++;
        }
    }

	//scandisco le colonne delle var non canoniche
    for(contatore1=0;contatore1<numNCan;contatore1++) {
    	//se la colonna e' 0 vuol dire che il prodotto e' solo il letterale della non canonica
        if(bm_get_col_value(bm,ncanoniche[contatore1])==0) {
        
           funzione->CEXLetterali[ncanoniche[contatore1]]=funzione->alpha[ncanoniche[contatore1]];
           
        } else {
          
            //l'uscita e' dritta o negata a seconda del segno delle var corrispondenti in xor in alpha

            //inserisco la non canonica di riferimento
            funzione->variabiliNC[numeroCEX]=ncanoniche[contatore1];
            
            funzione->CEXProducts = (vProduct) realloc(funzione->CEXProducts, (numeroCEX+1)*sizeof(Product));
			funzione->CEXProducts[numeroCEX] = CreateProduct(numVariabili);
            for (y = 0; y < numVariabili; y++) {
            
				//lo inizializzo a vuoto 0
				funzione->CEXProducts[numeroCEX][y] = '0';
			}
			
            //inserisco la non canonica, dritta poiche' il segno l'ho gia' salvato a parte
            funzione->CEXProducts[numeroCEX][ncanoniche[contatore1]]='1';
            tempSegno=funzione->alpha[ncanoniche[contatore1]];

            for(contatore2=0;contatore2<numCan;contatore2++) {
                if(bm_get(bm,contatore2,ncanoniche[contatore1])+bm_get(bm,contatore2,canoniche[contatore2])==2) {
                    funzione->CEXProducts[numeroCEX][canoniche[contatore2]]='1';
                    tempSegno=CharXOR(tempSegno,funzione->alpha[canoniche[contatore2]]);
                }
            }
            
            funzione->OutCEX[numeroCEX]=tempSegno;
            numeroCEX++;
        }
    }
    
    funzione->NumCEXProducts=numeroCEX;
	free(canoniche); free(ncanoniche);
}

void RiempiMatrice(binmat *bm, vProduct prodotti) {
    int j,k,vettoreSpec,input;
    int indiceRiga=0;
    int numProdotti, sum = 0;
    int *verifica;
    char *bin;
    input=bm->cols;
    numProdotti=(bm->rows)-input;
    verifica=(int*) calloc (input,sizeof(int));//controllo quali vettori ho gia' inserito, 0 no, 1 si
    bin=(char*) calloc (input+1,sizeof(char));
    for(j=0;j<numProdotti;j++) {
		sum=0;
		strcpy(bin, prodotti[j]);

		//scandisco la lista
        for(k = 0; k < input; k++) {
            int esp;
            esp=(input-1)-k;
            if(bin[k]=='1'){
                sum=sum+pow(2,esp);}
            else if (bin[k]=='-') {
            	//se non l'ho gia' messo
                if(verifica[k]==0) {
                    
                    vettoreSpec=pow(2,esp);
                    bm_set_row_value(bm, indiceRiga, vettoreSpec);

                    indiceRiga++;
                    verifica[k]=1;
                }
            }
        }
        
        bm_set_row_value(bm, indiceRiga, sum);
        indiceRiga++;

	}
	
	while(indiceRiga!=bm->rows)
		(bm->rows)--;
    
    free(verifica);
    free(bin);
}

void setAlpha(SOP *funzione, Product prodotto) {
    int i;
    funzione->alpha=CreateProduct(funzione->NumInputs);
    for(i=0;i<funzione->NumInputs;i++) {
        if(prodotto[i]!='-')
            funzione->alpha[i]=prodotto[i];
        else
            funzione->alpha[i]='0';
    }
}

vProduct XorConAlpha(SOP *funzione) {
    int i,j;
    vProduct XorInput=(vProduct)calloc(funzione->NumProducts,sizeof(Product));
    for(j=0;j<funzione->NumProducts;j++) {
        XorInput[j]=CreateProduct(funzione->NumInputs);
        for(i=0;i<funzione->NumInputs;i++)
            XorInput[j][i]=CharXORalpha(funzione->Products[j][i],funzione->alpha[i]);
    }
	return XorInput;
}
