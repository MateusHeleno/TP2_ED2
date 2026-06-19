#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <stdio.h>
#include "auxiliares.h"

bool lerRegistroNaPosicao(FILE *arq, Registro *reg, int pos, Metricas *metricas);
bool gravarRegistroNaPosicao(FILE *arq, Registro *reg, int pos, Metricas *metricas);
void particaoEsqDir(FILE *arq, int esq, int dir, int *i, int *j, Metricas *metricas);
void quicksortExternoRec(FILE *arq, int esq, int dir, Metricas *metricas);
void metodo3_QuicksortExterno(Config *config, Metricas *metricas);

#endif