#ifndef BALANCEADAOI_H
#define BALANCEADAOI_H

#include <stdio.h>
#include "auxiliares.h"

void gerarBlocosOrdenadosOI(const char *nomeArquivo, int quantidade, Metricas *metricas);
void intercalacaoOI(Config *config, Metricas *metricas);

#endif