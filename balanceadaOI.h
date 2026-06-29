#ifndef BALANCEADAOI_H
#define BALANCEADAOI_H

#include <stdio.h>
#include "auxiliares.h"

#define TAM_RAM 20
#define TAM_FITAS 40
#define TAM_FITAS_IN  (TAM_FITAS / 2)
#define TAM_FITAS_OUT (TAM_FITAS / 2)

typedef struct
{
    FILE *fita;
    int registros_lidos;
    int tamanho_bloco;
    bool ativo;
} BlocoFita;

void gerarBlocosOrdenadosOI(const char *nomeArquivo, int quantidade, Metricas *metricas);
void intercalacaoOI(Config *config, Metricas *metricas);

#endif