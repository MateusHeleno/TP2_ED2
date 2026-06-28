#ifndef BALANCEADAOI_H
#define BALANCEADAOI_H

#include <stdio.h>
#include "auxiliares.h"

#define TAM_RAM 20
#define TAM_FITAS 40

void gerarBlocosOrdenadosOI(const char *nomeArquivo, int quantidade, Metricas *metricas);
void intercalacaoOI(Config *config, Metricas *metricas);

#endif