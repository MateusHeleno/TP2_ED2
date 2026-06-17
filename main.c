#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "auxiliares.h"

int main(int argc, char *argv[])
{
    srand(time(NULL));

    Config config;
    if (!validaEntrada(argc, argv, &config))
        return 1;

    Metricas metricas;
    inicializaMetricas(&metricas); // começa contadores em zero

    // define o nome do arquivo com base na situação selecionada
    char *nomeArquivo;
    if (config.situacao == 1)
    {
        nomeArquivo = "ascendente.bin";
    }
    else if (config.situacao == 2)
    {
        nomeArquivo = "descendente.bin";
    }
    else
    {
        nomeArquivo = "provao.bin";
    }

    clock_t comeco = clock();

    if (config.p)
    {
        printf("--- Exibindo os %d primeiros registros do arquivo ---\n", config.qnt_registros);
    }
    lerEExibirRegistros(nomeArquivo, config.qnt_registros, config.p, &metricas);

    switch (config.metodo)
    {
    case 1:
        // Intercalação Balanceada(Ordenação Interna)
        break;

    case 2:
        // Intercalação Balanceada (Seleção por Substituição)
        break;

    case 3:
        // Quicksort Externo
        break;
    }

    clock_t fim = clock();
    metricas.tempo = (double)(fim - comeco) / CLOCKS_PER_SEC;

    // Relatório de desempenho
    // Problema: exibir os registros aumenta o tempo total nas metricas
    printMetricas(metricas);

    return 0;
}