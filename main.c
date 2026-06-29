#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "auxiliares.h"
#include "quicksort.h"
#include "balanceadaOI.h"
#include "intercalacaoSS.h"

int main(int argc, char *argv[])
{
    Config config;
    if (!validaEntrada(argc, argv, &config))
        return 1;

    Metricas metricas;

    /*
    Essa parte do código irá gerar 3 arquivos ->
    Ascendente, Descendente e Aleatório com o tamanho máximo dos registros.
    Com esses 3 arquivos, nós lemos apenas n elementos, sendo n a quantidade informada pelo usuário.
    Então se ele quer 1000 registros, ele irá ler apenas os 1000 registros dos 471.705 presentes no arquivo.
    Isso evita com que diversos arquivos de tamanhos diferentes sejam gerados e que eles sejam gerados a cada vez
    que o programa for executado.
    */
    if (!prepararArquivoInicial(&config))
    {
        printf("\nFalha na preparacao do arquivos iniciais.\n");
        return 1;
    }

    if (config.metodo == 3)
    {
        if (!prepararCopiaQuicksort(&config)) // prepara a copia do arquivo pro quicksort
        {
            printf("Erro ao preparar a copia isolada para o Quicksort Externo.\n");
            return 1;
        }
    }

    inicializaMetricas(&metricas); // subir se der erro de compliaçao

    clock_t inicio_relogio, fim_relogio;
    inicio_relogio = clock();

    switch (config.metodo)
    {
    case 1:
        printf("\nIniciando o Metodo 1: Intercalacao Balanceada...\n");
        intercalacaoOI(&config, &metricas);
        break;

    case 2:
        printf("\nIniciando o Metodo 2: Selecao por Substituicao...\n");
        intercalacaoSS(&config, &metricas);
        break;

    case 3:
        printf("\nIniciando o Metodo 3: Quicksort Externo...\n");
        quicksortExterno(&config, &metricas);
        break;
    default:
        printf("Metodo invalido.\n");
        return 1;    
    }
    fim_relogio = clock();
    metricas.tempo = (double)(fim_relogio - inicio_relogio) / CLOCKS_PER_SEC;

    printMetricas(metricas, config);

    return 0;
}