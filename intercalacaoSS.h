#ifndef INTERCALACAOSS_H
#define INTERCALACAOSS_H

#include "auxiliares.h"

#define TAM_RAM 20
#define TAM_FITAS 40

void gerarBlocosOrdenadosSubstituicao(const char *nomeArquivo, int quantidade, Metricas *metricas);
void intercalacaoSS(Config *config, Metricas *metricas);

#endif