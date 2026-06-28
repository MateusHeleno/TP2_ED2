#ifndef BALANCEADAOI_H
#define BALANCEADAOI_H

#include <stdio.h>
#include "auxiliares.h"

#defineTAM_RAM20

void gerarBlocosOrdenadosOI(const char *nomeArquivo, int quantidade, Metricas *metricas);
void intercalacaoOI(Config *config, Metricas *metricas);

#endif