#include "logic.h"
#include <string.h>
#include <math.h>

char CharXOR (char v1, char v2)
{
  return ( (v1 == v2) ? '0' : '1' );
}


char CharNotXOR (char v1, char v2)
{
  return ( (v1 == v2) ? '1' : '0' );
}


char CharNOT (char v1)
{
  return ( (v1 == '1') ? '0' : '1' );
}
char CharNOTMod (char v1)
{
    if(v1=='0')
        return '1';
    else if(v1=='1')
        return '0';
    else if(v1=='-')
        return '+';
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

char CharAND (char v1, char v2)
{
  return ( (v1 == '1') && (v2 == '1') ? '1' : '0' );
}


Product CreateProduct (int NumInputs)
{
  Product p = (Product) calloc(NumInputs+1,sizeof(char));
  if (p == NULL)
  {
    printf("Not enough memory to allocate a product!\n");
    exit(EXIT_MEMORY);
  }
  return p;
}


void DestroyProduct (Product *p)
{
  free(*p);
  *p = NULL;
}


GEPSOP *CreateGEPSOP (SOP *phi)
{
  GEPSOP *G = (GEPSOP *) malloc(sizeof(GEPSOP));
  if (G == NULL)
  {
    printf("Insufficient memory to allocate a GEPSOP!\n");
    exit(EXIT_MEMORY);
	 }

  G->NumInputs = ( (phi == NULL) ? 0 : phi->NumInputs);
  G->phi = phi;
  strcpy(G->phiName,"f");

  G->var = DUMMY_INDEX;
  G->OperandType = ' ';
  G->p = NULL;
  G->psi = NULL;
  G->psi_neg = NULL;
  G->rho = NULL;

  return G;
}

void DestroyGEPSOP (pGEPSOP *ppGEPSOP)
{
  pGEPSOP pG = *ppGEPSOP;

  if (pG->phi != NULL) DestroySOP(&pG->phi);
  if (pG->p != NULL) free(pG->p);
  if (pG->psi != NULL) DestroyGEPSOP(&pG->psi);
  if (pG->psi_neg != NULL) DestroyGEPSOP(&pG->psi_neg);
  if (pG->rho != NULL) DestroyGEPSOP(&pG->rho);
  free(pG);
  *ppGEPSOP = NULL;
}


SOP *CreateSOP (int NumInputs, int NumProducts, int NumDCProducts)
{
  int i;

  SOP *S = (SOP *) malloc(sizeof(SOP));
  if (S == NULL)
  {
    printf("Insufficient memory to allocate a SOP!\n");
    exit(EXIT_MEMORY);
  }

  S->NumInputs = NumInputs;
  S->NumProducts = NumProducts;
  S->Products = (vProduct) calloc(NumProducts,sizeof(Product));
  if (S->Products == NULL)
  {
    printf("Insufficient memory to allocate the Product vector of a SOP!\n");
    exit(EXIT_MEMORY);
  }

  for (i = 0; i < NumProducts; i++)
    S->Products[i] = CreateProduct(NumInputs);

  S->NumDCProducts = NumDCProducts;
  S->DCProducts = (vProduct) calloc(NumDCProducts,sizeof(Product));
  if (S->DCProducts == NULL)
  {
    printf("Insufficient memory to allocate the DCProduct vector of a SOP!\n");
    exit(EXIT_MEMORY);
  }

  for (i = 0; i < NumDCProducts; i++)
    S->DCProducts[i] = CreateProduct(NumInputs);

  return S;
}

void DestroySOP (pSOP *ppSOP)
{
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
  //char trash_string[NAME_LENGTH];
  //int trash_int;
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


// Costruisce la MO_GEPSOP che contiene come uscite banalmente
// le singole uscite della MO_SOP data.
// ATTENZIONE: Le singole SOP sono condivise fra MO_SOP e MO_GEPSOP
MO_GEPSOP BuildMO_GEPSOPfromMO_SOP (MO_SOP S, int NumOutputs)
{
  int o;
  MO_GEPSOP G;


  G = (MO_GEPSOP) calloc(NumOutputs,sizeof(pGEPSOP));
  if (G == NULL)
  {
    printf("Not enough memory to allocate a MO_GEPSOP!\n");
    exit(EXIT_MEMORY);
  }

  for (o = 0; o < NumOutputs; o++)
  {
    G[o] = CreateGEPSOP(S[o]);
    sprintf(G[o]->phiName,"y%d",o);
  }

  return G;
}


// Costruisce la MO_SOP che contiene come uscite le SOP componenti delle foglie della MO_GEPSOP
// in ordine di visita diretto-negato-resto per le varie uscite della MO_GEPSOP
MO_SOP BuildMO_SOPfromMO_GEPSOP (MO_GEPSOP G, int NumOutputs)
{
  MO_SOP S;
  int no;
  int o;

  if (G[0]->rho == NULL)
    no = 2*NumOutputs;
  else
    no = 3*NumOutputs;

  S = (MO_SOP) calloc(no,sizeof(pSOP));
  if (S == NULL)
  {
    printf("Not enough memory to allocate a MO_SOP!\n");
    exit(EXIT_MEMORY);
  }

  no = 0;
  for (o = 0; o < NumOutputs; o++)
  {
    S[no] = G[o]->psi->phi;
    no++;
    S[no] = G[o]->psi_neg->phi;
    no++;
    if (G[0]->rho != NULL)
    {
      S[no] = G[o]->rho->phi;
      no++;
    }
  }

  return S;
}


// Finds the most frequent pair in the given SOP.
// Returns it as (pv1,pv2) in lexicographic order.
void FindMostFrequentPair (MO_SOP S, int NumOutputs, int *pv1, int *pv2)
{
  int o, v1, v2, p;
  int FreqTot;      // Frequenza della tripletta corrente
  int Freq1, Freq2; // Frequenza della variabile v1, v2
  int MaxFreqTot;
  int MaxFreq12;

  // Nel caso in cui la SOP non contenga nessun prodotto di due letterali,
  // risulta MaxFreq = 0 e si considerano le due variabili piu' frequenti
  MaxFreqTot = -1;
  MaxFreq12 = -1;
  for (v1 = 0; v1 < S[0]->NumInputs-2; v1++)
    for (v2 = v1+1; v2 < S[0]->NumInputs-1; v2++)
    {
      // Conta il numero di prodotti in cui compare questa coppia
      FreqTot = Freq1 = Freq2 = 0;

      for (o = 0; o < NumOutputs; o++)
        for (p = 0; p < S[o]->NumProducts; p++)
        {
          if ( (S[o]->Products[p][v1] != '-') &&
               (S[o]->Products[p][v2] != '-') )
            FreqTot++;

          if (S[o]->Products[p][v1] != '-')
            Freq1++;

          if (S[o]->Products[p][v2] != '-')
            Freq2++;
        }

      if ( (FreqTot > MaxFreqTot) || ( (FreqTot == MaxFreqTot) && (Freq1+Freq2 > MaxFreq12) ) )
      {
        MaxFreqTot = FreqTot;
        MaxFreq12 = Freq1+Freq2;
        *pv1 = v1;
        *pv2 = v2;
      }
    }
}


// Project a trivial GEPSOP (i.e. a SOP) w.r.t. v1 XOR v2
void ProjectSOPwrtVar (GEPSOP *G, int v1, int v2, boolean Remainder)
{
  int i, p;
  int np_psi, np_psineg, np_rho;
  int ndcp_psi, ndcp_psineg, ndcp_rho;
  SOP *S, *pSOP;


  if ( (G->phi == NULL) || (G->psi != NULL) || (G->psi_neg != NULL) || (G->rho != NULL) )
  {
    printf("Only trivial GEPSOPs (i.e. SOPs) can be projected!\n");
    exit(EXIT_INCONSISTENCY);
  }

  pSOP = G->phi;

  G->var = v1;
  G->p = CreateProduct(G->NumInputs);
  G->OperandType = 'A';
  for (i = 0; i < G->NumInputs; i++)
    G->p[i] = '-';
  G->p[v2] = '1';

  if (Remainder == FALSE)
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto, il prodotto finisce immutato nei due spazi
      // Se ne compare una sola, il prodotto finisce nei due spazi opportunamente modificato
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') )
      {
        np_psi++;
        np_psineg++;
      }
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == pSOP->Products[p][v2])
          np_psi++;
        else
          np_psineg++;
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto, il prodotto finisce immutato nei due spazi
      // Se ne compare una sola, il prodotto finisce nei due spazi opportunamente modificato
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') )
      {
        ndcp_psi++;
        ndcp_psineg++;
      }
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == pSOP->DCProducts[p][v2])
          ndcp_psi++;
        else
          ndcp_psineg++;
      }
    }
  }
  else // Con resto
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto o se ne compare una sola,
      // il prodotto finisce nel resto
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') )
        np_rho++;
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == pSOP->Products[p][v2])
          np_psi++;
        else
          np_psineg++;
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto o se ne compare una sola,
      // il prodotto finisce nel resto
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') )
        ndcp_rho++;
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == pSOP->DCProducts[p][v2])
          ndcp_psi++;
        else
          ndcp_psineg++;
      }
    }
  }

  S = CreateSOP(pSOP->NumInputs,np_psi,ndcp_psi);
  G->psi = CreateGEPSOP(S);
  strcpy(G->psi->phiName,G->phiName);
  strcat(G->psi->phiName,"d");

  S = CreateSOP(pSOP->NumInputs,np_psineg,ndcp_psineg);
  G->psi_neg = CreateGEPSOP(S);
  strcpy(G->psi_neg->phiName,G->phiName);
  strcat(G->psi_neg->phiName,"n");

  if (Remainder == TRUE)
  {
    S = CreateSOP(pSOP->NumInputs,np_rho,ndcp_rho);
    G->rho = CreateGEPSOP(S);
    strcpy(G->rho->phiName,G->phiName);
    strcat(G->rho->phiName,"r");
  }

  if (Remainder == FALSE)
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto, il prodotto finisce immutato nei due spazi
      if ( (pSOP->Products[p][v1] == '-') && (pSOP->Products[p][v2] == '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        np_psi++;
        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        np_psineg++;
      }
      // Se v1 compare nel prodotto e v2 no, il prodotto finisce nei due spazi:
      // nello spazio affermato compare v2 con il letterale identico a quello di v1
      // nello spazio negato compare v2 con il letterale opposto a quello di v1
      else if ( (pSOP->Products[p][v1] != '-') && (pSOP->Products[p][v2] == '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = pSOP->Products[p][v1];
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = CharNOT(pSOP->Products[p][v1]);
        np_psineg++;
      }
      // Se v2 compare nel prodotto e v1 no, il prodotto finisce nei due spazi:
      // in entrambi, v2 compare cosi' com'e'
      else if ( (pSOP->Products[p][v1] == '-') && (pSOP->Products[p][v2] != '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        np_psineg++;
      }
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == pSOP->Products[p][v2])
        {
          strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
          G->psi->phi->Products[np_psi][v1] = '-';
          np_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          np_psineg++;
        }
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto, il prodotto finisce immutato nei due spazi
      if ( (pSOP->DCProducts[p][v1] == '-') && (pSOP->DCProducts[p][v2] == '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        ndcp_psi++;
        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        ndcp_psineg++;
      }
      // Se v1 compare nel prodotto e v2 no, il prodotto finisce nei due spazi:
      // nello spazio affermato compare v2 con il letterale identico a quello di v1
      // nello spazio negato compare v2 con il letterale opposto a quello di v1
      else if ( (pSOP->DCProducts[p][v1] != '-') && (pSOP->DCProducts[p][v2] == '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = pSOP->DCProducts[p][v1];
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = CharNOT(pSOP->DCProducts[p][v1]);
        ndcp_psineg++;
      }
      // Se v2 compare nel prodotto e v1 no, il prodotto finisce nei due spazi:
      // in entrambi, v2 compare cosi' com'e'
      else if ( (pSOP->DCProducts[p][v1] == '-') && (pSOP->DCProducts[p][v2] != '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        ndcp_psineg++;
      }
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == pSOP->DCProducts[p][v2])
        {
          strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
          G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
          ndcp_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          ndcp_psineg++;
        }
      }
    }
  }
  else // Con resto
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto o se ne compare una sola,
      // il prodotto finisce nel resto
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') )
      {
        strcpy(G->rho->phi->Products[np_rho],pSOP->Products[p]);
        np_rho++;
      }
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == pSOP->Products[p][v2])
        {
          strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
          G->psi->phi->Products[np_psi][v1] = '-';
          np_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          np_psineg++;
        }
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 e v2 non compaiono nel prodotto o se ne compare una sola,
      // il prodotto finisce nel resto
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') )
      {
        strcpy(G->rho->phi->DCProducts[ndcp_rho],pSOP->DCProducts[p]);
        ndcp_rho++;
      }
      // Se v1 e v2 compaiono entrambe nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == pSOP->DCProducts[p][v2])
        {
          strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
          G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
          ndcp_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          ndcp_psineg++;
        }
      }
    }
  }

  // ATTENZIONE: Poiche' la MP_GEPSOP potrebbe avere le SOP in comune
  //             con una MO_SOP, non deallochiamo, ma semplicemente cancelliamo i puntatori
  G->phi = NULL;
}


void FindMostFrequentTriplet (MO_SOP S, int NumOutputs, int *pv1, int *pv2, int *pv3)
{
  int o, v1, v2, v3, p;
  int FreqTot;             // Frequenza della tripletta corrente
  int Freq1, Freq2, Freq3; // Frequenza della variabile v1, v2, v3 con le altre due
  int MaxFreqTot;
  int MaxFreq123;

  // Nel caso in cui la SOP non contenga nessun prodotto di tre letterali,
  // risulta MaxFreqTot = 0, e si considerano le terne fusione delle coppie piu' frequenti
  MaxFreqTot = -1;
  MaxFreq123 = -1;
  for (v1 = 0; v1 < S[0]->NumInputs-2; v1++)
    for (v2 = v1+1; v2 < S[0]->NumInputs-1; v2++)
      for (v3 = v2+1; v3 < S[0]->NumInputs; v3++)
      {
        // Conta il numero di prodotti in cui compare questa tripletta
        FreqTot = Freq1 = Freq2 = Freq3 = 0;

        for (o = 0; o < NumOutputs; o++)
          for (p = 0; p < S[o]->NumProducts; p++)
          {
            if ( (S[o]->Products[p][v1] != '-') &&
                 (S[o]->Products[p][v2] != '-') &&
                 (S[o]->Products[p][v3] != '-') )
              FreqTot++;

            if ( (S[o]->Products[p][v1] != '-') &&
                 ( (S[o]->Products[p][v2] != '-') ||
                   (S[o]->Products[p][v3] != '-') ) )
              Freq1++;

            if ( (S[o]->Products[p][v2] != '-') &&
                 ( (S[o]->Products[p][v1] != '-') ||
                   (S[o]->Products[p][v3] != '-') ) )
              Freq2++;

            if ( (S[o]->Products[p][v3] != '-') &&
                 ( (S[o]->Products[p][v1] != '-') ||
                   (S[o]->Products[p][v2] != '-') ) )
              Freq3++;
          }

        if ( (FreqTot > MaxFreqTot) || ( (FreqTot == MaxFreqTot) && (MAX(Freq1,Freq2,Freq3) > MaxFreq123) ) )
        {
          MaxFreqTot = FreqTot;
          MaxFreq123 = MAX(Freq1,Freq2,Freq3);
          if (Freq1 == MaxFreq123)
          {
            *pv1 = v1;
            *pv2 = v2;
            *pv3 = v3;
          }
          else if (Freq2 == MaxFreq123)
          {
            *pv1 = v2;
            *pv2 = v1;
            *pv3 = v3;
          }
          else
          {
            *pv1 = v3;
            *pv2 = v1;
            *pv3 = v2;
          }
        }
      }
}


// Project a trivial GEPSOP (i.e. a SOP) w.r.t. v1 == (v2 AND v3) and v1 == NOT (v2 AND v3)
void ProjectSOPwrtAND (GEPSOP *G, int v1, int v2, int v3, boolean Remainder)
{
  int i, p;
  int np_psi, np_psineg, np_rho;
  int ndcp_psi, ndcp_psineg, ndcp_rho;
  SOP *S, *pSOP;


  if ( (G->phi == NULL) || (G->psi != NULL) || (G->psi_neg != NULL) || (G->rho != NULL) )
  {
    printf("Only trivial GEPSOPs (i.e. SOPs) can be projected!\n");
    exit(EXIT_INCONSISTENCY);
  }

  pSOP = G->phi;

  G->var = v1;
  G->p = CreateProduct(G->NumInputs);
  G->OperandType = 'A';
  for (i = 0; i < G->NumInputs; i++)
    G->p[i] = '-';
  G->p[v2] = '1';
  G->p[v3] = '1';

  if (Remainder == FALSE)
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->Products[p][v1] == '-')
      {
        np_psi++;
        np_psineg++;
      }
      // Se v1 compare diretto nel prodotto, e nessuna fra v2 e v3 vi compare
      else if ( (pSOP->Products[p][v1] == '1') && (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        np_psi++;
        np_psineg += 2;
      }
      // Se v1 compare negato nel prodotto, e nessuna fra v2 e v3 vi compare
      else if ( (pSOP->Products[p][v1] == '0') && (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        np_psi += 2;
        np_psineg++;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      else if ( ( (pSOP->Products[p][v2] != '-') && (pSOP->Products[p][v3] == '-') ) ||
                ( (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] != '-') ) )
      {
        np_psi++;
        np_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharAND(pSOP->Products[p][v2],pSOP->Products[p][v3]))
          np_psi++;
        else
          np_psineg++;
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->DCProducts[p][v1] == '-')
      {
        ndcp_psi++;
        ndcp_psineg++;
      }
      // Se v1 compare diretto nel prodotto, e nessuna fra v2 e v3 vi compare
      else if ( (pSOP->DCProducts[p][v1] == '1') && (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        ndcp_psi++;
        ndcp_psineg += 2;
      }
      // Se v1 compare negato nel prodotto, e nessuna fra v2 e v3 vi compare
      else if ( (pSOP->DCProducts[p][v1] == '0') && (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        ndcp_psi += 2;
        ndcp_psineg++;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      else if ( ( (pSOP->DCProducts[p][v2] != '-') && (pSOP->DCProducts[p][v3] == '-') ) ||
                ( (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] != '-') ) )
      {
        ndcp_psi++;
        ndcp_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharAND(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]))
          ndcp_psi++;
        else
          ndcp_psineg++;
      }
    }
  }
  else // Con resto
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se almeno una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce nel resto
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') || (pSOP->Products[p][v3] == '-') )
        np_rho++;
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharAND(pSOP->Products[p][v2],pSOP->Products[p][v3]))
          np_psi++;
        else
          np_psineg++;
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se almeno una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce nel resto
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') || (pSOP->DCProducts[p][v3] == '-') )
        ndcp_rho++;
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharAND(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]))
          ndcp_psi++;
        else
          ndcp_psineg++;
      }
    }

  }


  S = CreateSOP(pSOP->NumInputs,np_psi,ndcp_psi);
  G->psi = CreateGEPSOP(S);
  strcpy(G->psi->phiName,G->phiName);
  strcat(G->psi->phiName,"d");

  S = CreateSOP(pSOP->NumInputs,np_psineg,ndcp_psineg);
  G->psi_neg = CreateGEPSOP(S);
  strcpy(G->psi_neg->phiName,G->phiName);
  strcat(G->psi_neg->phiName,"n");

  if (Remainder == TRUE)
  {
    S = CreateSOP(pSOP->NumInputs,np_rho,ndcp_rho);
    G->rho = CreateGEPSOP(S);
    strcpy(G->rho->phiName,G->phiName);
    strcat(G->rho->phiName,"r");
  }

  if (Remainder == FALSE)
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->Products[p][v1] == '-')
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        np_psi++;
        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        np_psineg++;
      }
      // Se v1 compare affermato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con v2 v3 al posto di v1
      // nello spazio negato: uno con !v2 e uno con !v3 al posto di v1
      else if ( (pSOP->Products[p][v1] == '1') && (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '1';
        G->psi->phi->Products[np_psi][v3] = '1';
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '0';
        G->psi_neg->phi->Products[np_psineg][v3] = '-';
        np_psineg++;
        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '-';
        G->psi_neg->phi->Products[np_psineg][v3] = '0';
        np_psineg++;
      }
      // Se v1 compare negato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con !v2 e uno con !v3 al posto di v1
      // nello spazio negato: uno con v2 v3 al posto di v1
      else if ( (pSOP->Products[p][v1] == '0') && (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '0';
        G->psi->phi->Products[np_psi][v3] = '-';
        np_psi++;
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '-';
        G->psi->phi->Products[np_psi][v3] = '0';
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '1';
        G->psi_neg->phi->Products[np_psineg][v3] = '1';
        np_psineg++;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      // In psi, la variabile mancante e' l'AND delle due presenti
      // In psineg, dipende dalla variabile presente .
      else if ( ( (pSOP->Products[p][v2] != '-') && (pSOP->Products[p][v3] == '-') ) ||
                ( (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] != '-') ) )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '1';
        G->psi->phi->Products[np_psi][v3] = '1';
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        if (pSOP->Products[p][v2] != '-') // se compare v2
        {
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          G->psi_neg->phi->Products[np_psineg][v2] = '1';
          G->psi_neg->phi->Products[np_psineg][v3] = '0';
        }
        else // compare v3
        {
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          G->psi_neg->phi->Products[np_psineg][v2] = '0';
          G->psi_neg->phi->Products[np_psineg][v3] = '1';
        }
        np_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharAND(pSOP->Products[p][v2],pSOP->Products[p][v3]) )
        {
          strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
          G->psi->phi->Products[np_psi][v1] = '-';
          np_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          np_psineg++;
        }
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->DCProducts[p][v1] == '-')
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        ndcp_psi++;
        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        ndcp_psineg++;
      }
      // Se v1 compare affermato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con v2 v3 al posto di v1
      // nello spazio negato: uno con !v2 e uno con !v3 al posto di v1
      else if ( (pSOP->DCProducts[p][v1] == '1') && (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '1';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '1';
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '0';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '-';
        ndcp_psineg++;
        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '0';
        ndcp_psineg++;
      }
      // Se v1 compare negato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con !v2 e uno con !v3 al posto di v1
      // nello spazio negato: uno con v2 v3 al posto di v1
      else if ( (pSOP->DCProducts[p][v1] == '0') && (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '0';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '-';
        ndcp_psi++;
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '0';
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '1';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '1';
        ndcp_psineg++;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      // In psi, la variabile mancante e' l'AND delle due presenti
      // In psineg, dipende dalla variabile presente .
      else if ( ( (pSOP->DCProducts[p][v2] != '-') && (pSOP->DCProducts[p][v3] == '-') ) ||
                ( (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] != '-') ) )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '1';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '1';
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        if (pSOP->DCProducts[p][v2] != '-') // se compare v2
        {
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '1';
          G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '0';
        }
        else // compare v3
        {
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '0';
          G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '1';
        }
        ndcp_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharAND(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]) )
        {
          strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
          G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
          ndcp_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          ndcp_psineg++;
        }
      }
    }
  }
  else // Con resto
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se almeno una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce nel resto
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') || (pSOP->Products[p][v3] == '-') )
      {
        strcpy(G->rho->phi->Products[np_rho],pSOP->Products[p]);
        np_rho++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharAND(pSOP->Products[p][v2],pSOP->Products[p][v3]) )
        {
          strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
          G->psi->phi->Products[np_psi][v1] = '-';
          np_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          np_psineg++;
        }
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se almeno una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce nel resto
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') || (pSOP->DCProducts[p][v3] == '-') )
      {
        strcpy(G->rho->phi->DCProducts[ndcp_rho],pSOP->DCProducts[p]);
        ndcp_rho++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharAND(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]) )
        {
          strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
          G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
          ndcp_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          ndcp_psineg++;
        }
      }
    }
  }

  // ATTENZIONE: Poiche' la MP_GEPSOP potrebbe avere le SOP in comune
  //             con una MO_SOP, non deallochiamo, ma semplicemente cancelliamo i puntatori
  G->phi = NULL;
}


// Project a trivial GEPSOP (i.e. a SOP) v1 == (v2 XOR v3) and v1 == NOT (v2 XOR v3)
void ProjectSOPwrtXOR (GEPSOP *G, int v1, int v2, int v3, boolean Remainder)
{
  int i, p;
  int np_psi, np_psineg, np_rho;
  int ndcp_psi, ndcp_psineg, ndcp_rho;
  SOP *S, *pSOP;


  if ( (G->phi == NULL) || (G->psi != NULL) || (G->psi_neg != NULL) || (G->rho != NULL) )
  {
    printf("Only trivial GEPSOPs (i.e. SOPs) can be projected!\n");
    exit(EXIT_INCONSISTENCY);
  }

  pSOP = G->phi;

  G->var = v1;
  G->p = CreateProduct(G->NumInputs);
  G->OperandType = 'X';
  for (i = 0; i < G->NumInputs; i++)
    G->p[i] = '-';
  G->p[v2] = '1';
  G->p[v3] = '1';

  if (Remainder == FALSE)
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->Products[p][v1] == '-')
      {
        np_psi++;
        np_psineg++;
      }
      // Se v1 compare nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti
      else if ( (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        np_psi += 2;
        np_psineg += 2;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      else if ( ( (pSOP->Products[p][v2] != '-') && (pSOP->Products[p][v3] == '-') ) ||
                ( (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] != '-') ) )
      {
        np_psi++;
        np_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharXOR(pSOP->Products[p][v2],pSOP->Products[p][v3]))
          np_psi++;
        else
          np_psineg++;
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->DCProducts[p][v1] == '-')
      {
        ndcp_psi++;
        ndcp_psineg++;
      }
      // Se v1 compare nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti
      else if ( (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        ndcp_psi += 2;
        ndcp_psineg += 2;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      else if ( ( (pSOP->DCProducts[p][v2] != '-') && (pSOP->DCProducts[p][v3] == '-') ) ||
                ( (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] != '-') ) )
      {
        ndcp_psi++;
        ndcp_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharXOR(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]))
          ndcp_psi++;
        else
          ndcp_psineg++;
      }
    }
  }
  else // Con resto
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se almeno una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce nel resto
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') || (pSOP->Products[p][v3] == '-') )
        np_rho++;
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharXOR(pSOP->Products[p][v2],pSOP->Products[p][v3]))
          np_psi++;
        else
          np_psineg++;
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se almeno una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce nel resto
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') || (pSOP->DCProducts[p][v3] == '-') )
        ndcp_rho++;
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharXOR(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]))
          ndcp_psi++;
        else
          ndcp_psineg++;
      }
    }
  }

  S = CreateSOP(pSOP->NumInputs,np_psi,ndcp_psi);
  G->psi = CreateGEPSOP(S);
  strcpy(G->psi->phiName,G->phiName);
  strcat(G->psi->phiName,"d");

  S = CreateSOP(pSOP->NumInputs,np_psineg,ndcp_psineg);
  G->psi_neg = CreateGEPSOP(S);
  strcpy(G->psi_neg->phiName,G->phiName);
  strcat(G->psi_neg->phiName,"n");

  if (Remainder == TRUE)
  {
    S = CreateSOP(pSOP->NumInputs,np_rho,ndcp_rho);
    G->rho = CreateGEPSOP(S);
    strcpy(G->rho->phiName,G->phiName);
    strcat(G->rho->phiName,"r");
  }

  if (Remainder == FALSE)
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->Products[p][v1] == '-')
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        np_psi++;
        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        np_psineg++;
      }
      // Se v1 compare affermato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con v2 !v3 e uno con !v2 v3 al posto di v1 (cioe' lo XOR)
      // nello spazio negato: uno con v2 v3 e uno con !v2 !v3 al posto di v1
      else if ( (pSOP->Products[p][v1] == '1') && (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '0';
        G->psi->phi->Products[np_psi][v3] = '1';
        np_psi++;
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '1';
        G->psi->phi->Products[np_psi][v3] = '0';
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '0';
        G->psi_neg->phi->Products[np_psineg][v3] = '0';
        np_psineg++;
        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '1';
        G->psi_neg->phi->Products[np_psineg][v3] = '1';
        np_psineg++;
      }
      // Se v1 compare negato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con v2 v3 e uno con !v2 !v3 al posto di v1
      // nello spazio negato: uno con v2 !v3 e uno con !v2 v3 al posto di v1 (cioe' lo XOR)
      else if ( (pSOP->Products[p][v1] == '0') && (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] == '-') )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '0';
        G->psi->phi->Products[np_psi][v3] = '0';
        np_psi++;
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';
        G->psi->phi->Products[np_psi][v2] = '1';
        G->psi->phi->Products[np_psi][v3] = '1';
        np_psi++;

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '0';
        G->psi_neg->phi->Products[np_psineg][v3] = '1';
        np_psineg++;
        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';
        G->psi_neg->phi->Products[np_psineg][v2] = '1';
        G->psi_neg->phi->Products[np_psineg][v3] = '0';
        np_psineg++;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      // In psi, la variabile mancante e' lo XOR delle due presenti
      // In psineg, la variabile mancante e' lo XOR negato delle due presenti
      // La variabile presente non cambia, la v1 sparisce.
      else if ( ( (pSOP->Products[p][v2] != '-') && (pSOP->Products[p][v3] == '-') ) ||
                ( (pSOP->Products[p][v2] == '-') && (pSOP->Products[p][v3] != '-') ) )
      {
        strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
        G->psi->phi->Products[np_psi][v1] = '-';

        strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
        G->psi_neg->phi->Products[np_psineg][v1] = '-';

        if (pSOP->Products[p][v2] != '-') // se compare v2
        {
          G->psi->phi->Products[np_psi][v3] = CharXOR(pSOP->Products[p][v1],pSOP->Products[p][v2]);
          G->psi_neg->phi->Products[np_psineg][v3] = CharNotXOR(pSOP->Products[p][v1],pSOP->Products[p][v2]);
        }
        else // compare v3
        {
          G->psi->phi->Products[np_psi][v2] = CharXOR(pSOP->Products[p][v1],pSOP->Products[p][v3]);
          G->psi_neg->phi->Products[np_psineg][v2] = CharNotXOR(pSOP->Products[p][v1],pSOP->Products[p][v3]);
        }
        np_psi++;
        np_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharXOR(pSOP->Products[p][v2],pSOP->Products[p][v3]) )
        {
          strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
          G->psi->phi->Products[np_psi][v1] = '-';
          np_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          np_psineg++;
        }
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se v1 non compare nel prodotto, il prodotto finisce immutato nei due spazi
      if (pSOP->DCProducts[p][v1] == '-')
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        ndcp_psi++;
        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        ndcp_psineg++;
      }
      // Se v1 compare affermato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con v2 !v3 e uno con !v2 v3 al posto di v1 (cioe' lo XOR)
      // nello spazio negato: uno con v2 v3 e uno con !v2 !v3 al posto di v1
      else if ( (pSOP->DCProducts[p][v1] == '1') && (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '0';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '1';
        ndcp_psi++;
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '1';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '0';
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '0';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '0';
        ndcp_psineg++;
        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '1';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '1';
        ndcp_psineg++;
      }
      // Se v1 compare negato nel prodotto, e nessuna fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi spezzato in due prodotti:
      // nello spazio affermato: uno con v2 v3 e uno con !v2 !v3 al posto di v1
      // nello spazio negato: uno con v2 !v3 e uno con !v2 v3 al posto di v1 (cioe' lo XOR)
      else if ( (pSOP->DCProducts[p][v1] == '0') && (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] == '-') )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '0';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '0';
        ndcp_psi++;
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
        G->psi->phi->DCProducts[ndcp_psi][v2] = '1';
        G->psi->phi->DCProducts[ndcp_psi][v3] = '1';
        ndcp_psi++;

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '0';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '1';
        ndcp_psineg++;
        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = '1';
        G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = '0';
        ndcp_psineg++;
      }
      // Se v1 compare nel prodotto, e una sola fra v2 e v3 vi compare,
      // il prodotto finisce nei due spazi
      // In psi, la variabile mancante e' lo XOR delle due presenti
      // In psineg, la variabile mancante e' lo XOR negato delle due presenti
      // La variabile presente non cambia, la v1 sparisce.
      else if ( ( (pSOP->DCProducts[p][v2] != '-') && (pSOP->DCProducts[p][v3] == '-') ) ||
                ( (pSOP->DCProducts[p][v2] == '-') && (pSOP->DCProducts[p][v3] != '-') ) )
      {
        strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
        G->psi->phi->DCProducts[ndcp_psi][v1] = '-';

        strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
        G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';

        if (pSOP->DCProducts[p][v2] != '-') // se compare v2
        {
          G->psi->phi->DCProducts[ndcp_psi][v3] = CharXOR(pSOP->DCProducts[p][v1],pSOP->DCProducts[p][v2]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v3] = CharNotXOR(pSOP->DCProducts[p][v1],pSOP->DCProducts[p][v2]);
        }
        else // compare v3
        {
          G->psi->phi->DCProducts[ndcp_psi][v2] = CharXOR(pSOP->DCProducts[p][v1],pSOP->DCProducts[p][v3]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v2] = CharNotXOR(pSOP->DCProducts[p][v1],pSOP->DCProducts[p][v3]);
        }
        ndcp_psi++;
        ndcp_psineg++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharXOR(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]) )
        {
          strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
          G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
          ndcp_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          ndcp_psineg++;
        }
      }
    }
  }
  else // Con resto
  {
    np_psi = np_psineg = np_rho = 0;
    for (p = 0; p < pSOP->NumProducts; p++)
    {
      // Se una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce immutato nel resto
      if ( (pSOP->Products[p][v1] == '-') || (pSOP->Products[p][v2] == '-') || (pSOP->Products[p][v3] == '-') )
      {
        strcpy(G->rho->phi->Products[np_rho],pSOP->Products[p]);
        np_rho++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->Products[p][v1] == CharXOR(pSOP->Products[p][v2],pSOP->Products[p][v3]) )
        {
          strcpy(G->psi->phi->Products[np_psi],pSOP->Products[p]);
          G->psi->phi->Products[np_psi][v1] = '-';
          np_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->Products[np_psineg],pSOP->Products[p]);
          G->psi_neg->phi->Products[np_psineg][v1] = '-';
          np_psineg++;
        }
      }
    }

    ndcp_psi = ndcp_psineg = ndcp_rho = 0;
    for (p = 0; p < pSOP->NumDCProducts; p++)
    {
      // Se una fra v1, v2 e v3 non compare nel prodotto, il prodotto finisce immutato nel resto
      if ( (pSOP->DCProducts[p][v1] == '-') || (pSOP->DCProducts[p][v2] == '-') || (pSOP->DCProducts[p][v3] == '-') )
      {
        strcpy(G->rho->phi->DCProducts[ndcp_rho],pSOP->DCProducts[p]);
        ndcp_rho++;
      }
      // Se v1, v2 e v3 compaiono tutte nel prodotto,
      // il prodotto finisce solo da una parte (quale, dipende dai letterali)
      else
      {
        if (pSOP->DCProducts[p][v1] == CharXOR(pSOP->DCProducts[p][v2],pSOP->DCProducts[p][v3]) )
        {
          strcpy(G->psi->phi->DCProducts[ndcp_psi],pSOP->DCProducts[p]);
          G->psi->phi->DCProducts[ndcp_psi][v1] = '-';
          ndcp_psi++;
        }
        else
        {
          strcpy(G->psi_neg->phi->DCProducts[ndcp_psineg],pSOP->DCProducts[p]);
          G->psi_neg->phi->DCProducts[ndcp_psineg][v1] = '-';
          ndcp_psineg++;
        }
      }
    }
  }

  // ATTENZIONE: Poiche' la MP_GEPSOP potrebbe avere le SOP in comune
  //             con una MO_SOP, non deallochiamo, ma semplicemente cancelliamo i puntatori
  G->phi = NULL;
}


void Rec_Print_GEPSOP_Blif (GEPSOP *G, FILE *fBlifFile)
{
  int i, p;
  int v2, v3;
  Product P;

  P = CreateProduct(G->NumInputs);

  // Se la GEPSOP e' una semplice SOP, la salva
  if (G->phi != NULL)
  {
    fprintf(fBlifFile,".names ");
    for (i = 0; i < G->phi->NumInputs; i++)
      fprintf(fBlifFile,"x%d ",i);

    fprintf(fBlifFile,"%s ",G->phiName);
    fprintf(fBlifFile,"\n");
    for (p = 0; p < G->phi->NumProducts; p++)
      //fprintf(fBlifFile,"%s %c\n",G->phi->Products[p],G->Products->Product_Output[p][o]);
      fprintf(fBlifFile,"%s %c\n",G->phi->Products[p],'1');
  }
  else
  {
    // Altrimenti, stampa
    // 1) il prodotto
    fprintf(fBlifFile,".names ");
    for (i = 0; i < G->NumInputs; i++)
      fprintf(fBlifFile,"x%d ",i);

    fprintf(fBlifFile,"%sp",G->phiName);
    fprintf(fBlifFile,"\n");
    if (G->OperandType == 'A')
      fprintf(fBlifFile,"%s %c\n",G->p,'1');
    else if (G->OperandType == 'X')
    {
      // ATTENZIONE: SI DA PER SCONTATO CHE LO XOR COINVOLGA ESATTAMENTE DUE VARIABILI
      v2 = v3 = -1;
      for (i = 0; i < G->NumInputs; i++)
        if (G->p[i] != '-')
        {
          if (v2 < 0)
            v2 = i;
          else if (v3 < 0)
            v3 = i;
          else
          {
            printf("The XOR has more than two variables!\n");
            exit(EXIT_INCONSISTENCY);
          }
        }

      strcpy(P,G->p);
      P[v3] = '0';
      fprintf(fBlifFile,"%s %c\n",P,'1');

      strcpy(P,G->p);
      P[v2] = '0';
      fprintf(fBlifFile,"%s %c\n",P,'1');
    }

    // 2) lo XOR fra variabile e prodotto
    fprintf(fBlifFile,".names ");
    fprintf(fBlifFile,"x%d ",G->var);
    fprintf(fBlifFile,"%sp ",G->phiName);
    fprintf(fBlifFile,"%se",G->phiName);
    fprintf(fBlifFile,"\n");
    fprintf(fBlifFile,"00 1\n");
    fprintf(fBlifFile,"11 1\n");

    // 3) stampa ricorsivamente le SOP componenti
    Rec_Print_GEPSOP_Blif(G->psi,fBlifFile);
    Rec_Print_GEPSOP_Blif(G->psi_neg,fBlifFile);
    if (G->rho != NULL) Rec_Print_GEPSOP_Blif(G->rho,fBlifFile);

    // 4) mette insieme il tutto
    fprintf(fBlifFile,".names ");
    fprintf(fBlifFile,"%s ",G->psi->phiName);
    fprintf(fBlifFile,"%s ",G->psi_neg->phiName);
    fprintf(fBlifFile,"%sr ",G->phiName); // Sarebbe il nome del rho, che pero' non sempre c'e'.
    fprintf(fBlifFile,"%se ",G->phiName);
    fprintf(fBlifFile,"%s ",G->phiName);
    fprintf(fBlifFile,"\n");
    if (G->rho != NULL)
    {
    }
    else
    {
      fprintf(fBlifFile,"1--0 1\n");
      fprintf(fBlifFile,"-1-1 1\n");
    }
    if (G->rho != NULL) fprintf(fBlifFile,"--1- 1\n");
  }

  DestroyProduct(&P);
}


void Print_MO_GEPSOP_Blif (MO_GEPSOP G, int NumOutputs, char *BlifFile)
{
  FILE *fBlifFile;
  int NumInputs;
  int i, o;


  fBlifFile = fopen(BlifFile,"w+");
  if (fBlifFile == NULL)
  {
    printf("File %s could not be opened!\n",BlifFile);
    exit(EXIT_FILEACCESS);
  }

  fprintf(fBlifFile,".model %s\n",BlifFile);

  NumInputs = G[0]->NumInputs;
  fprintf(fBlifFile,".inputs ",BlifFile);
  for (i = 0; i < NumInputs; i++)
    fprintf(fBlifFile,"x%d ",i);
  fprintf(fBlifFile,"\n");

  fprintf(fBlifFile,".outputs ",BlifFile);
  for (o = 0; o < NumOutputs; o++)
    fprintf(fBlifFile,"%s ",G[o]->phiName);
  fprintf(fBlifFile,"\n");

  // Si procede ricorsivamente, salvando prima le SOP foglia e poi risalendo via via
  for (o = 0; o < NumOutputs; o++)
    Rec_Print_GEPSOP_Blif(G[o],fBlifFile);

  fprintf(fBlifFile,".end\n");

  fclose(fBlifFile);
}


// Replace the SOP components in the given GEPSOP with the given SOPs
void ReloadMO_GEPSOPfromMO_SOP (MO_SOP S, int no, MO_GEPSOP G)
{
  boolean Remainder;
  int NumOutputs, o;
  // ATTENZIONE: PER ORA L FACCIAMO CON GEPSOP A UN SOLO LIVELLO

  Remainder = (G[0]->rho != NULL);

  if (Remainder == FALSE)
  {
    NumOutputs = no / 2;
    for (o = 0; o < NumOutputs; o++)
    {
      G[o]->psi->phi = S[2*o];
      G[o]->psi_neg->phi = S[2*o+1];
    }
  }
  else
  {
    NumOutputs = no / 3;
    for (o = 0; o < NumOutputs; o++)
    {
      G[o]->psi->phi = S[3*o];
      G[o]->psi_neg->phi = S[3*o+1];
      G[o]->rho->phi = S[3*o+2];
    }
  }
}


void ParseEspresso (char *OutputFile, int *pCost, double *pTime)
{
  FILE *fOutputFile;
  char s[ROW_LENGTH];
  int n;
  double t;

  fOutputFile = fopen(OutputFile,"r");
  if (fOutputFile == NULL)
  {
    printf("Errore while opening file %s!\n",OutputFile);
    exit(EXIT_FILEACCESS);
  }

  do {
    n = fscanf(fOutputFile,"%s",s);
    if ( (n == 1) && ( (strcmp(s,"ESPRESSO") == 0) || (strcmp(s,"exact") == 0) ) )
    {
      fscanf(fOutputFile,"%*s");
      fscanf(fOutputFile,"%*s");
     fscanf(fOutputFile,"%s",s);
      sscanf(s,"%lf",pTime);
      //*pTime = t;

      fscanf(fOutputFile,"%*s %*s %*s %*s %*s %*s %s",s);
      strcpy(s,s+4);
      sscanf(s,"%d",pCost);
      //fscanf(fOutputFile,"Time was %s sec, cost is %*s %*s %*s %s",s);
    }

  } while (n != EOF);

  fclose(fOutputFile);
}


/************************/
void CreaCEX(binmat *bm, SOP *funzione)
{
    int *canoniche,*ncanoniche;
    char tempSegno;
    int numVariabili,numCan,numNCan,g,contatore1,contatore2,col,appog;
    numVariabili=bm->cols;
    g=0;
    int i,y,numeroCEX=0;

    numCan=bm->rows;
    numNCan=(numVariabili-numCan);
    funzione->OutCEX=CreateProduct(numNCan);
    funzione->NumCEXProducts=numNCan;
    funzione->CEXProducts  = (vProduct) calloc(funzione->NumCEXProducts,sizeof(Product));
    funzione->variabiliNC = (int*)calloc(numNCan,sizeof(int));
    canoniche = (int*) calloc (numCan,sizeof(int));
    ncanoniche = (int*) calloc (numNCan,sizeof(int));

    for(contatore1=0;contatore1<numCan;contatore1++)
        {
            appog=bm_get_row_value(bm, contatore1);
            y=log2(appog);
            canoniche[contatore1]=(numVariabili-1)-y;
        }
    for(contatore1=0;contatore1<numVariabili;contatore1++)
        {
            i=0;
            for(contatore2=0;contatore2<numCan;contatore2++)
                if(contatore1==canoniche[contatore2])
                    i=1;
            if(i==0) // non e' canonica
                {
                    ncanoniche[g]=contatore1;
                    g++;
                }
        }

    for(contatore1=0;contatore1<numNCan;contatore1++)//scandisco le colonne delle var non canoniche
    {
        if(bm_get_col_value(bm,ncanoniche[contatore1])==0){//se la colonna e' 0 vuol dire che il prodotto \E8 solo il letterale della non canonica
           funzione->CEXLetterali[ncanoniche[contatore1]]=funzione->alpha[ncanoniche[contatore1]];
           printf("cex letterale %d\n",ncanoniche[contatore1]);}
        else
        {
            funzione->CEXProducts[numeroCEX] = CreateProduct(numVariabili);
           // printf("prima %s\n",funzione->CEXProducts[numeroCEX]);
            for(i=0;i<numVariabili;i++)
                funzione->CEXProducts[numeroCEX][i]='0';//lo inizializzo a vuoto 0
            //l'uscita \E8 dritta o negata a seconda del segno delle var corrispondenti in xor in alpha


            //inserisco la non canonica di riferimento
        //    printf("assegno la non canonica %d\n",ncanoniche[contatore1]);
            funzione->variabiliNC[numeroCEX]=ncanoniche[contatore1];
          //  printf("assegno la non canonica al prodotto\n");
            //inserisco la non canonica, dritta poich\E8 il segno l'ho gi\E0 salvato a parte
            funzione->CEXProducts[numeroCEX][ncanoniche[contatore1]]='1';
            tempSegno=funzione->alpha[ncanoniche[contatore1]];
            for(contatore2=0;contatore2<numCan;contatore2++)
            {
                if(bm_get(bm,contatore2,ncanoniche[contatore1])+bm_get(bm,contatore2,canoniche[contatore2])==2)
                    {
                        funzione->CEXProducts[numeroCEX][canoniche[contatore2]]='1';
                        tempSegno=CharXOR(tempSegno,funzione->alpha[canoniche[contatore2]]);
                    }
            }
            funzione->OutCEX[numeroCEX]=tempSegno;
            numeroCEX++;
        }
    }

//printf("letterali singoli %s\n",funzione->CEXLetterali);
funzione->NumCEXProducts=numeroCEX;
//printf("prodotti composti %d\n",funzione->NumCEXProducts);
//salviamo le var da eliminare, che sono le non canoniche
funzione->varEliminare=CreateProduct(funzione->NumInputs);
for(i=0;i<funzione->NumInputs;i++)
    funzione->varEliminare[i]='0';
for(i=0;i<numNCan;i++)
    funzione->varEliminare[ncanoniche[i]]='1';
//printf("**********************\n");
//printf("alpha %s\n",funzione->alpha);
 //printf("**********************\n");
 //for(i=0;i<funzione->NumCEXProducts;i++)
 //printf("CEX %s con uscita %c\n",funzione->CEXProducts[i],funzione->OutCEX[i]);
 //printf("**********************\n");

}

/*void OttimizzaCEX(SOP *funzione)
{
int i,j,k,variabili,variabili2,diff,sum;//se due prodotti differiscono di 2 elementi, significa che differiscono solo per se stessi,
                //quindi il resto del prodotto \E8 uguale

for(i=0;i<(funzione->NumCEXProducts)-1;i++)
    {
        diff=0;
        variabili=ContaVariabiliDefinite(funzione->CEXProducts[i]);
        for(j=i+1;j<funzione->NumCEXProducts;j++)
            {
               variabili2=ContaVariabiliDefinite(funzione->CEXProducts[j]);
               if((variabili>2)&&(variabili==variabili2))
                {
                    sum=0;
                    for(k=0;k<funzione->NumInputs;k++)
                        if(funzione->CEXProducts[i][k]!=funzione->CEXProducts[j][k])
                            diff++;
                    if(diff==2)
                        {
                        for(k=0;k<funzione->NumInputs;k++)
                            {
                                if(funzione->CEXProducts[i][k]==funzione->CEXProducts[j][k])
                                    funzione->CEXProducts[j][k]='-';
                                else
                                    {
                                        if(funzione->CEXProducts[i][k]!='-')
                                            funzione->CEXProducts[j][k]=funzione->CEXProducts[i][k];
                                    }
                            }
                            //devo negare una variabile delle nuove 2:
                            //se sono entrambe 0, ne metto una a 1
                            //se sono entrambe 1, ne metto una a 0
                            //se sono discordi, metto quella a 0 a 1
                        for(k=0;k<funzione->NumInputs;k++)
                            {int a;
                                if(funzione->CEXProducts[j][k]!='-')
                                {
                                a=atoi(&funzione->CEXProducts[j][k]);
                                sum=sum+a;
                                }
                            }

                        for(k=0;k<funzione->NumInputs;k++)
                            {
                                if((sum==1)&&(funzione->CEXProducts[j][k]=='0'))
                                    {
                                    funzione->CEXProducts[j][k]='1';
                                    sum=3;
                                    }
                                else if((sum==0)&&(funzione->CEXProducts[j][k]=='1'))
                                    {
                                    funzione->CEXProducts[j][k]=='1';
                                    sum=3;//ne devo negare solo uno, quindi esco
                                    }
                                else if((sum==0)||(sum==2)&&(funzione->CEXProducts[j][k]=='1'))
                                    {
                                    funzione->CEXProducts[j][k]=='0';
                                    sum=3;//ne devo negare solo uno, quindi esco
                                    }
                            }
                        }

                }
            }
    }

//OrdinaPerNumVariabiliDef(funzione->CEXProducts, funzione->NumCEXProducts);
quicksortCEX(funzione->CEXProducts, 0, (funzione->NumCEXProducts)-1, 1);
}*/
void OttimizzaCEX(SOP *funzione)
{
    char ris,temp;
    int uni=0;
    int uniXor=0;
    int *guadagno;
    int max=0, indiceMax;
    guadagno=(int*)calloc(funzione->NumCEXProducts,sizeof(int));
    int i,j,k,len;
    int modifica=1;
    len=funzione->NumInputs;
    while(modifica==1)
    {
        modifica=0;
        for(i=0;i<funzione->NumCEXProducts;i++)
        {
            //printf("prodotto %d di %d %s\n",i,funzione->NumCEXProducts,funzione->CEXProducts[i]);
            max=0;
            indiceMax=0;
            guadagno[i]=-1;
            for(j=0;j<funzione->NumCEXProducts;j++)
            {
                if(i!=j)
                {   //printf("secondo prodotto %d %s\n",j,funzione->CEXProducts[j]);
                    uni=0;
                    uniXor=0;
                    for(k=0;k<len;k++)
                    {
                        if(funzione->CEXProducts[i][k]=='1')
                            uni++;
                        ris=CharXOR(funzione->CEXProducts[i][k],funzione->CEXProducts[j][k]);
                        if(ris=='1')
                            uniXor++;
                    }
                    //printf("gli uni in %s sono %d\n",funzione->CEXProducts[i],uni);
                     //printf("gli uni dello xor sono %d\n",uniXor);
                    guadagno[j]=uni-uniXor;
                    //printf("%d contro %d guadagno %d\n",i,j,guadagno[j]);
                }
            }
            for(j=0;j<funzione->NumCEXProducts;j++)
            {//printf("j %d guadagno %d\n",j,guadagno[j]);
                if(guadagno[j]>max)
                {//printf("\E8 pi\F9 grande %d\n",j);
                    max=guadagno[j];
                    indiceMax=j;
                }
            }
            if(max>0) //se ho un guadagno, sostituisco prodotto[i] col suo xor col prodotto j(devo agg anche l'output)
            {//printf("valore indice max %d\n",indiceMax);
                for(k=0;k<len;k++)
                {
                    temp=CharXOR(funzione->CEXProducts[i][k],funzione->CEXProducts[indiceMax][k]);
                    funzione->CEXProducts[i][k]=temp;
                }
                funzione->OutCEX[i]=CharXOR(funzione->OutCEX[i],funzione->OutCEX[indiceMax]);
                modifica=1;
            }
        }
    }
    //un giro con anche l'uguale a 0
    /*for(i=0;i<funzione->NumCEXProducts;i++)
        {
            max=0;
            indiceMax=0;
            guadagno[i]=-1;
            for(j=0;j<funzione->NumCEXProducts;j++)
            {
                if(i!=j)
                {
                    uni=0;
                    uniXor=0;
                    for(k=0;k<len;k++)
                    {
                        if(funzione->CEXProducts[i][k]=='1')
                            uni++;
                        ris=CharXOR(funzione->CEXProducts[i][k],funzione->CEXProducts[j][k]);
                        if(ris=='1')
                            uniXor++;
                    }
                    guadagno[j]=uni-uniXor;
                }
            }
            for(j=0;j<funzione->NumCEXProducts;j++)
            {
                if(guadagno[j]>max)
                {
                    max=guadagno[j];
                    indiceMax=j;
                }
            }
            if(max>=0)
            {
                for(k=0;k<len;k++)
                {
                    temp=CharXOR(funzione->CEXProducts[i][k],funzione->CEXProducts[indiceMax][k]);
                    funzione->CEXProducts[i][k]=temp;
                }
                funzione->OutCEX[i]=CharXOR(funzione->OutCEX[i],funzione->OutCEX[indiceMax]);
                modifica=1;
            }
        }
    //riproviamo col strettamente >
    while(modifica==1)
    {
        modifica=0;
        for(i=0;i<funzione->NumCEXProducts;i++)
        {
            max=0;
            indiceMax=0;
            guadagno[i]=-1;
            for(j=0;j<funzione->NumCEXProducts;j++)
            {
                if(i!=j)
                {
                    uni=0;
                    uniXor=0;
                    for(k=0;k<len;k++)
                    {
                        if(funzione->CEXProducts[i][k]=='1')
                            uni++;
                        ris=CharXOR(funzione->CEXProducts[i][k],funzione->CEXProducts[j][k]);
                        if(ris=='1')
                            uniXor++;
                    }
                    guadagno[j]=uni-uniXor;
                }
            }
            for(j=0;j<funzione->NumCEXProducts;j++)
            {
                if(guadagno[j]>max)
                {
                    max=guadagno[j];
                    indiceMax=j;
                }
            }
            if(max>0)
            {
                for(k=0;k<len;k++)
                {
                    temp=CharXOR(funzione->CEXProducts[i][k],funzione->CEXProducts[indiceMax][k]);
                    funzione->CEXProducts[i][k]=temp;
                }
                funzione->OutCEX[i]=CharXOR(funzione->OutCEX[i],funzione->OutCEX[indiceMax]);
                modifica=1;
            }
        }
    }*/
funzione->indiciCEX=(int*)calloc(funzione->NumCEXProducts,sizeof(int));
}
//quicksortCEX(funzione->CEXProducts, funzione->OutCEX,funzione->variabiliNC,0, (funzione->NumCEXProducts)-1, 1);


void RiempiMatrice(binmat *bm, vProduct *prodotti)
{
    int j,k,vettoreSpec,input;
    int indiceRiga=0;
    int numProdotti, sum = 0;
    int *verifica;
    char *bin;
    input=bm->cols;
    numProdotti=(bm->rows)-input;
    verifica=(int*) calloc (input,sizeof(int));//controllo quali vettori ho gi\E0 inserito, 0 no, 1 si
    bin=(char*) calloc (input+1,sizeof(char));
    for(j=0;j<numProdotti;j++)
       {
        sum=0;
        strcpy(bin, prodotti[j]);

        for(k = 0; k < input; k++)//scandisco la lista
            {
            int esp;
            esp=(input-1)-k;
            if(bin[k]=='1'){
                sum=sum+pow(2,esp);}
            else if (bin[k]=='-')
                {
                    if(verifica[k]==0)//se non l'ho gi\E0 messo
                    {
                       // printf("riga %d valore %d\n",indiceRiga,vettoreSpec);
                        vettoreSpec=pow(2,esp);
                        //printf("riga %d valore %d k %d\n",indiceRiga,vettoreSpec,k);
                        bm_set_row_value(bm, indiceRiga, vettoreSpec);

                        //bm_set (bm, indiceRiga, k, 1);
                        //printf("valore dopo %d\n",bm_get_row_value(bm, indiceRiga));
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
}

void ScriviSuFilegeneraXOR(char *file,int l, char* tipo, char* prodotto, int numeroBit)
{

if(l==(numeroBit-1))
    {
        if(tipo=='d')
            prodotto[numeroBit-1]='1';
        else
            prodotto[numeroBit-1]='0';
        fprintf(file,"%s 1\n",prodotto);
    }
else
    {
        if(tipo=='d')
            {
                 prodotto[l]='0';
                 ScriviSuFilegeneraXOR(file,l+1,'d',prodotto,numeroBit);
                 prodotto[l]='1';
                 ScriviSuFilegeneraXOR(file,l+1,'p',prodotto,numeroBit);
            }
        else
            {
                 prodotto[l]='0';
                 ScriviSuFilegeneraXOR(file,l+1,'p',prodotto,numeroBit);
                 prodotto[l]='1';
                 ScriviSuFilegeneraXOR(file,l+1,'d',prodotto,numeroBit);
            }
    }
}

char Parity(char* punto)
{
int i, parit=0;
char risultato;
    for(i=0;i<strlen(punto);i++)
        if(punto[i]=='1')
            parit++;
if(parit%2==0)
    risultato='p';
else
    risultato='d';
return risultato;
}


void generaXOR(int l, char* tipo, char* prodotto, int numeroBit, int *lung, vProduct prova)
{
int g;
if(l==(numeroBit-1))
    {
        if(tipo=='d')
            prodotto[numeroBit-1]='1';
        else
            prodotto[numeroBit-1]='0';
        for(g=0;g<numeroBit;g++)
                prova[(*lung)][g]=prodotto[g];
        (*lung)++;
    }
else
    {
        if(tipo=='d')
            {
                 prodotto[l]='0';
                 generaXOR(l+1,'d',prodotto,numeroBit, lung,prova);
                 prodotto[l]='1';
                 generaXOR(l+1,'p',prodotto,numeroBit, lung, prova);
            }
        else
            {
                 prodotto[l]='0';
                 generaXOR(l+1,'p',prodotto,numeroBit, lung,prova);
                 prodotto[l]='1';
                 generaXOR(l+1,'d',prodotto,numeroBit, lung,prova);
            }
    }
}

void setAlpha(SOP *funzione, Product prodotto)
{
    int lung,i,j;
    //Product appoggio;
    //appoggio=CreateProduct(funzione->NumInputs);
    funzione->alpha=CreateProduct(funzione->NumInputs);
    for(i=0;i<funzione->NumInputs;i++)
    {
        if(prodotto[i]!='-')
            funzione->alpha[i]=prodotto[i];
        else
            funzione->alpha[i]='0';
    }
}
//- non definita, 0 o 1 definita (negata o dritta)
int ContaVariabiliDefinite(Product prodotto)
{
    int i,len,VarDef =0;
    for(i=0;i<strlen(prodotto);i++)
        if((prodotto[i]=='0')||(prodotto[i]=='1'))
            VarDef++;
    return VarDef;
}
//0 non definita,1 definita
int ContaVariabiliDefiniteCEX(Product prodotto)
{
    int i,len,VarDef =0;
    for(i=0;i<strlen(prodotto);i++)
        if(prodotto[i]=='1')
            VarDef++;
    return VarDef;
}

void ScambiaProdotti(vProduct Prodotti,Product uscita,int *varNC, int p, int q)
{
    Product temp;
    char tempUscita;
    int tempVar;
    temp=CreateProduct(strlen(Prodotti[p]));
    tempUscita=uscita[p];
    uscita[p]=uscita[q];
    uscita[q]=tempUscita;
    tempVar=varNC[p];
    varNC[p]=varNC[q];
    varNC[q]=tempVar;
    strcpy(temp,Prodotti[p]);
    strcpy(Prodotti[p],Prodotti[q]);
    strcpy(Prodotti[q],temp);
}

int divideCEX (vProduct Prodotti, Product uscita, int *varNC, int l, int r, int leq)
{

  int idx = l;
  int i;
  for (i = l; i < r; i++) {
    if (leq)
    {
      if (ContaVariabiliDefinite(Prodotti[i]) <= ContaVariabiliDefinite(Prodotti[r]))
      {
        ScambiaProdotti(Prodotti,uscita,varNC,idx,i);
        idx++;
      }
    }
    else
    {
      if (ContaVariabiliDefinite(Prodotti[i]) >= ContaVariabiliDefinite(Prodotti[r]))
      {
        ScambiaProdotti(Prodotti,uscita, varNC,idx, i);
        idx++;
      }
    }
  }
  ScambiaProdotti(Prodotti,uscita, varNC,idx, r);
  return idx;
}

void quicksortCEX (vProduct Prodotti, Product uscita, int *varNC,int l, int r, int leq)
{
  int d;
  if (r > l)
  {
    d = divideCEX (Prodotti, uscita,varNC,l, r, leq);
    quicksortCEX (Prodotti, uscita,varNC,l, d - 1, leq);
    quicksortCEX (Prodotti, uscita,varNC,d + 1, r, leq);
  }
}

void TipoRiduzione(SOP *funzione,char* tipo)
{
    int NumVariabili,scandisci;
    if(strcmp(tipo,"-2D")==0)
    {
         if(funzione->NumCEXProducts>0)
        {
        quicksortCEX(funzione->CEXProducts, funzione->OutCEX,funzione->variabiliNC,0, (funzione->NumCEXProducts)-1, 1);
        //printf("qui va bene");
         NumVariabili=ContaVariabiliDefiniteCEX(funzione->CEXProducts[(funzione->NumCEXProducts)-1]);
        // printf("cex in fondo %s num variabili %d\n",funzione->CEXProducts[(funzione->NumCEXProducts)-1],NumVariabili);

         while(NumVariabili>2)
         {
             //scandisci=(funzione->NumInputs)-1;
             //i prodotti ottimizzati sono di sicuro da 2 quindi non rientrano qui
             //gli altri prodotti hanno come variabile pi\F9 a dx quella non canonica, che non va pi\F9 eliminata se
             //si scarta il prodotto dallo spazio affine
             /*while(funzione->CEXProducts[(funzione->NumCEXProducts)-1][scandisci]=='-')
                scandisci--;

             funzione->varEliminare[scandisci]='0';//riabilito la variabile
             */
             funzione->varEliminare[funzione->variabiliNC[(funzione->NumCEXProducts)-1]]='0';
             (funzione->NumCEXProducts)--;

             if(funzione->NumCEXProducts!=0)
               {
                    NumVariabili=ContaVariabiliDefiniteCEX(funzione->CEXProducts[(funzione->NumCEXProducts)-1]);
          //      printf("cex in fondo %s num variabili %d\n",funzione->CEXProducts[(funzione->NumCEXProducts)-1],NumVariabili);

               }

             else if(funzione->NumCEXProducts==0)
                NumVariabili=0; //se non ho pi\F9 prodotti CEX forzo l'uscita dal while
         }
        }
    }
    if(strcmp(tipo,"-1D")==0)
    {

         while(funzione->NumCEXProducts>0)
         {
             funzione->varEliminare[funzione->variabiliNC[(funzione->NumCEXProducts)-1]]='0';
             (funzione->NumCEXProducts)--;
         }
    }

}

int ContaProssimiDC(SOP *funzione, Product prodottoCEX, char segno)
{
    int i,j,numIndefiniti,parziali,prossimi=0;
    Product appoggio;
    appoggio=CreateProduct(funzione->NumInputs);
    for(j=0;j<funzione->NumDCProducts;j++)
        {
           parziali=0;
           for(i=0;i<funzione->NumInputs;i++)
            {
                if(prodottoCEX[i]=='0')
                    appoggio[i]='0';
                else if(prodottoCEX[i]=='1')
                {//se \E8 1 andrebbe dritto, se \E8 0 negato
                    appoggio[i]=funzione->DCProducts[j][i];
                }
            }
            numIndefiniti=(funzione->NumInputs)-ContaVariabiliDefinite(appoggio);
            if(numIndefiniti==0)//devo vedere, se \E8 dispari allora lo tengo
                   {
                  if(((Parity(appoggio)=='d')&&(segno=='1'))||((Parity(appoggio)=='p')&&(segno=='0')))
                      parziali=1;
                   }
            else
                parziali=pow(2,(numIndefiniti-1));
            prossimi=prossimi+parziali;
        }
    return prossimi;
}

void EliminaLetteraliDaDCSet(SOP *funzione)
{//printf("sono in elimina letterali\n");
    int i,j,ok,conta=0;
    //for(i=0;i<funzione->NumDCProducts;i++)
   //     printf("DC %d uguale a %s\n",i,funzione->DCProducts[i]);
    vProduct nuovoDCSet;
    if(ContaVariabiliDefinite(funzione->CEXLetterali)!=0)
    {
        nuovoDCSet=(vProduct) calloc(funzione->NumDCProducts,sizeof(Product));
        for(i=0;i<funzione->NumDCProducts;i++)
            nuovoDCSet[i]=CreateProduct(funzione->NumInputs);
        for(j=0;j<funzione->NumDCProducts;j++)
        {
            ok=1;
            for(i=0;i<funzione->NumInputs;i++)
            {
                if(funzione->CEXLetterali[i]!='-')
                {
                    if(funzione->DCProducts[j][i]=='-')
                        funzione->DCProducts[j][i]=funzione->CEXLetterali[i];
                    else if(funzione->DCProducts[j][i]!=funzione->CEXLetterali[i])
                        ok=0;
                }
            }
            if(ok==1)
            {
            nuovoDCSet[conta]=funzione->DCProducts[j];
            conta++;
            }
        }
        funzione->DCProducts=nuovoDCSet;
        funzione->NumDCProducts=conta;
    }
}



int TrovaDCSet(SOP *funzione, int prodottoCEX, char segno)
{ //printf("prodotto CEX %d su %d\n",prodottoCEX,funzione->NumCEXProducts);
    int i,j,numIndefiniti,prossimi, conta=0;
    vProduct nuovoDCSet;
    Product appoggio;
    appoggio=CreateProduct(funzione->NumInputs);
   // printf("esco con num DC %d numCEXProdo %d, prodotto puntato %d\n",funzione->NumDCProducts,funzione->NumCEXProducts,prodottoCEX);
    if ((funzione->NumDCProducts==0)||(prodottoCEX==funzione->NumCEXProducts))
    //termino quando il DC Set \E8 vuoto o ho controllato tutti i prodotti dello spazio affine
       return 1;
    else
    {
        prossimi=ContaProssimiDC(funzione,funzione->CEXProducts[prodottoCEX],funzione->OutCEX[prodottoCEX]);
        //printf("i prossimi sono %d\n",prossimi);
        nuovoDCSet=(vProduct) calloc(prossimi,sizeof(Product));
        for(i=0;i<prossimi;i++)
            nuovoDCSet[i]=CreateProduct(funzione->NumInputs);
        for(j=0;j<funzione->NumDCProducts;j++)
            {
               for(i=0;i<funzione->NumInputs;i++)
                {
                    if(funzione->CEXProducts[prodottoCEX][i]=='0')
                        appoggio[i]='0';
                    else if(funzione->CEXProducts[prodottoCEX][i]=='1')
                        appoggio[i]=funzione->DCProducts[j][i];

                }
                numIndefiniti=(funzione->NumInputs)-ContaVariabiliDefinite(appoggio);
                if(numIndefiniti==0)
                {
                    if(((Parity(appoggio)=='d')&&(segno=='1'))||((Parity(appoggio)=='p')&&(segno=='0')))
                    {
                    nuovoDCSet[conta]=funzione->DCProducts[j];
                    conta++;
                    }
                }
                else if(numIndefiniti!=0)
                {
                    int k,o;
                    int *lung=0;
                    int neg=0;
                    o=pow(2,numIndefiniti-1);
                    vProduct prova;
                    prova=(vProduct)calloc(o,sizeof(Product));
                    char* prod;
                    prod=(char*)calloc((numIndefiniti+1),sizeof(char));
                    for(k=0;k<o;k++)
                        prova[k]=CreateProduct(numIndefiniti);
                    if(segno=='0')
                            neg=1;
                    if((Parity(appoggio)=='d')&&(neg==0))
                        generaXOR(0,'p',prod,numIndefiniti,&lung,prova);
                    if((Parity(appoggio)=='p')&&(neg==0))
                        generaXOR(0,'d',prod,numIndefiniti,&lung,prova);
                    if((Parity(appoggio)=='d')&&(neg==1))
                        generaXOR(0,'d',prod,numIndefiniti,&lung,prova);
                    if((Parity(appoggio)=='p')&&(neg==1))
                        generaXOR(0,'p',prod,numIndefiniti,&lung,prova);
                    int p,riga=0;
                    for(riga=0;riga<o;riga++)
                    {
                      int elem=0;
                      for(k=0;k<funzione->NumInputs;k++)
                        {
                           if((funzione->DCProducts[j][k]=='0')||(funzione->DCProducts[j][k]=='1'))
                                   nuovoDCSet[conta][k]=funzione->DCProducts[j][k];
                            else if((funzione->DCProducts[j][k]=='-')&&(funzione->CEXProducts[prodottoCEX][k]=='0'))
                                nuovoDCSet[conta][k]='-';
                            else{
                                nuovoDCSet[conta][k]=prova[riga][elem];
                                elem++;
                                }
                        }
                    conta++;
                    }
                }
            }
        //assegnare nuovoDCSet a funzione->DCProducts
        funzione->DCProducts=nuovoDCSet;
        //aggiornare funzione->NumDCProducts=prossimi
        funzione->NumDCProducts=prossimi;
        prodottoCEX++;
        if ((funzione->NumDCProducts!=0)&&(prodottoCEX!=funzione->NumCEXProducts))
            TrovaDCSet(funzione, prodottoCEX, funzione->OutCEX[prodottoCEX]);
    }
}

char* MinimizzaSingolaUscitaRid(SOP *funzione,char *nomeFile)
{
    int i,j,inputsRid=0;
    char *PlaRidottaMinEspresso;
    char *comandoEspresso;

    comandoEspresso = (char*) calloc (NAME_LENGTH,sizeof(char));
    PlaRidottaMinEspresso = (char*) calloc (NAME_LENGTH,sizeof(char));
    FILE *fPlaRidotta;

    for(j=0;j<funzione->NumInputs;j++)
        if(funzione->varEliminare[j]=='0')
            inputsRid++;

    fPlaRidotta = fopen(nomeFile,"w");
    fprintf(fPlaRidotta,".i %d \n",inputsRid);
    fprintf(fPlaRidotta,".o 1\n"); //ho un solo output
    for(i=0;i<funzione->NumProducts;i++) //scrivo l'on set, ridotto
    {
        for(j=0;j<funzione->NumInputs;j++)
        {
            if(funzione->varEliminare[j]=='0')
                fprintf(fPlaRidotta,"%c",funzione->Products[i][j]);
        }
    fprintf(fPlaRidotta," 1\n");
    }
     for(i=0;i<funzione->NumDCProducts;i++) //scrivo il DCSet, ridotto
    {
        for(j=0;j<funzione->NumInputs;j++)
        {
            if(funzione->varEliminare[j]=='0')
                fprintf(fPlaRidotta,"%c",funzione->DCProducts[i][j]);
        }
    fprintf(fPlaRidotta," -\n");
    }
    //Se non ho n\E8 - n\E8 1, allora posso mettere ---.. 0
    if(funzione->NumProducts<1 && funzione->NumDCProducts<1 )
    {
         for(i=0;i<funzione->NumDCProducts;i++) //scrivo il DCSet, ridotto
        {
            for(j=0;j<funzione->NumInputs;j++)
            {
                if(funzione->varEliminare[j]=='0')
                    fprintf(fPlaRidotta,"-");
            }
        fprintf(fPlaRidotta," 0\n");
        }
    }
    fprintf(fPlaRidotta,".e");
    fclose(fPlaRidotta);

    //chiamare espresso
    sprintf(PlaRidottaMinEspresso,"%sopt.pla",nomeFile);
    sprintf(comandoEspresso,"espresso.exe -t %s > %s",nomeFile,PlaRidottaMinEspresso);
    system(comandoEspresso);
    return PlaRidottaMinEspresso;
}

char* MinimizzaSingolaUscita(SOP *funzione,char *nomeFile)
{
    int i,j,inputsRid=0;
    char *PlaRidottaMinEspresso;
    char *comandoEspresso;

    comandoEspresso = (char*) calloc (NAME_LENGTH,sizeof(char));
    PlaRidottaMinEspresso = (char*) calloc (NAME_LENGTH,sizeof(char));
    FILE *fPlaRidotta;

    fPlaRidotta = fopen(nomeFile,"w");
    fprintf(fPlaRidotta,".i %d \n",funzione->NumInputs);
    fprintf(fPlaRidotta,".o 1\n"); //ho un solo output
    for(i=0;i<funzione->NumProducts;i++) //scrivo l'on set
        fprintf(fPlaRidotta,"%s 1\n",funzione->Products[i]);
    for(i=0;i<funzione->NumDCProducts;i++) //scrivo il DCSet, ridotto
        fprintf(fPlaRidotta,"%s -\n",funzione->DCProducts[i]);
    if(funzione->NumProducts<1 && funzione->NumDCProducts<1 )
    {
        for(i=0;i<funzione->NumInputs;i++)
            fprintf(fPlaRidotta,"-");
        fprintf(fPlaRidotta," 0\n");
    }

    fprintf(fPlaRidotta,".e");
    fclose(fPlaRidotta);

    //chiamare espresso
    sprintf(PlaRidottaMinEspresso,"%sopt.pla",nomeFile);
    sprintf(comandoEspresso,"espresso.exe -t %s > %s",nomeFile,PlaRidottaMinEspresso);
    system(comandoEspresso);
    return PlaRidottaMinEspresso;
    //printf("comando espresso %s\n",comandoEspresso);
}


void ScriviFunzRidotta(char *file,char *PlaRidottaMinEspresso)
{
    FILE* fPlaRidottaMinEspresso;
    fPlaRidottaMinEspresso=fopen(PlaRidottaMinEspresso,"r");
    char* prodottoOpt;
    boolean done=FALSE;
    prodottoOpt=(char*)calloc(NAME_LENGTH,sizeof(char));
    fgets(prodottoOpt,256,fPlaRidottaMinEspresso);
    //while(prodottoOpt[0]!='.' && prodottoOpt[0]!='e' && !done)
    while(!feof(fPlaRidottaMinEspresso) && !done)
    {
        fgets(prodottoOpt,256,fPlaRidottaMinEspresso);
        if((prodottoOpt[0]=='0')||(prodottoOpt[0]=='1')||(prodottoOpt[0]=='-'))
            done=TRUE;
    }

    while(!feof(fPlaRidottaMinEspresso)&& done)
    {
        fprintf(file,"%s",prodottoOpt);
        fgets(prodottoOpt,256,fPlaRidottaMinEspresso);
        if(prodottoOpt[0]=='.')
            done=FALSE;
    }
    fclose(fPlaRidottaMinEspresso);
}

// Salva su file la PLA corrispondente alla MO_SOP data.
void SavePLADred (MO_SOP OutSOP, int NumOutputs, char *OutputFile)
{
  FILE *fOutputFile;
  int NumProducts;
  char *ProductOutput;
  int p, o,k;
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
        if(OutSOP[o]->NumCEXProducts==0)
            fprintf(fOutputFile,"%s",OutSOP[o]->Products[p]);

        else if(OutSOP[o]->NumCEXProducts!=0)
        {//printf("entro qui");
        for(k=0;k<OutSOP[0]->NumInputs;k++)
            {
             if(OutSOP[o]->varEliminare[k]=='0')
                fprintf(fOutputFile,"%c",OutSOP[o]->Products[p][k]);
            else
                fprintf(fOutputFile,"-");
            }

        }
      fprintf(fOutputFile," ");

      ProductOutput[o] = '1';
      fprintf(fOutputFile,"%s",ProductOutput);
      ProductOutput[o] = '0';
      fprintf(fOutputFile,"\n");
    }
    //printf("numero DC %d\n\n\n",OutSOP[o]->NumDCProducts);
    for (p = 0; p < OutSOP[o]->NumDCProducts; p++)
    {

      if(OutSOP[o]->NumCEXProducts==0)
            fprintf(fOutputFile,"%s",OutSOP[o]->DCProducts[p]);

        else if(OutSOP[o]->NumCEXProducts!=0)
        {
        for(k=0;k<OutSOP[0]->NumInputs;k++)
            {
             if(OutSOP[o]->varEliminare[k]=='0')
                fprintf(fOutputFile,"%c",OutSOP[o]->DCProducts[p][k]);
            else
                fprintf(fOutputFile,"-");
            }

        }
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

char* MinimizzaMultiUscita(MO_SOP OutSOP,int NumOutputs,char *nomeFile)
{
    SavePLADred(OutSOP, NumOutputs,nomeFile);
    char *PlaRidottaMinEspresso;
    char *comandoEspresso;
    comandoEspresso = (char*) calloc (NAME_LENGTH,sizeof(char));
    PlaRidottaMinEspresso = (char*) calloc (NAME_LENGTH,sizeof(char));
    //chiamare espresso
    sprintf(PlaRidottaMinEspresso,"%sopt.pla",nomeFile);
    sprintf(comandoEspresso,"espresso.exe -t %s > %s",nomeFile,PlaRidottaMinEspresso);
    system(comandoEspresso);
    return PlaRidottaMinEspresso;
}


vProduct XorConAlpha(SOP *funzione)
{
    int i,j;
    vProduct XorInput=(vProduct)calloc(funzione->NumProducts,sizeof(Product));
    for(j=0;j<funzione->NumProducts;j++)
    {
        XorInput[j]=CreateProduct(funzione->NumInputs);
        for(i=0;i<funzione->NumInputs;i++)
            XorInput[j][i]=CharXORalpha(funzione->Products[j][i],funzione->alpha[i]);
    }
return XorInput;
}
