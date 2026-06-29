#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <stdio.h>
#include "auxiliares.h"

bool lerRegistroNaPosicao(FILE *arq, Registro *reg, long pos, Metricas *metricas);
bool gravarRegistroNaPosicao(FILE *arq, Registro *reg, long pos, Metricas *metricas);
void quickSortExternoRec(FILE *arq, int esq, int dir, Metricas *metricas);
void quicksortExterno(Config *config, Metricas *metricas);

#endif