#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "auxiliares.h"
#include "quicksort.h"
// #include "intercalacao.h"
// #include "polifasica.h"

int main(int argc, char *argv[]) {
    Config config;
    if (!validaEntrada(argc, argv, &config)) {
        return 1;
    }
    Metricas metricas;
    inicializaMetricas(&metricas);
    printf("Preparando o arquivo de entrada...\n");
    if (!prepararArquivoInicial(&config, &metricas)) {
        printf("\n[ERRO FATAL] Falha na preparacao do arquivo 'entrada_atual.txt'.\n");
        printf("-> Verifique se o arquivo original esta na mesma pasta e se chama EXATAMENTE 'PROVAO.TXT' (tudo maiusculo).\n");
        return 1;
    }
    printf("Arquivo preparado com sucesso!\n\n");
    clock_t inicio_relogio, fim_relogio;
    inicio_relogio = clock();
    switch (config.metodo) {
        case 1:
            printf("Iniciando o Metodo 1: Intercalacao Balanceada...\n");
            // metodo1_IntercalacaoNormal(&config, &metricas);
            printf("Ainda nao implementado no main.\n");
            break;

        case 2:
            printf("Iniciando o Metodo 2: Selecao por Substituicao...\n");
            // metodo2_SelecaoSubstituicao(&config, &metricas);
            printf("Ainda nao implementado no main.\n");
            break;

        case 3:
            printf("Iniciando o Metodo 3: Quicksort Externo...\n");
            metodo3_QuicksortExterno(&config, &metricas);
            break;

        default:
            printf("Metodo invalido escolhido.\n");
            return 1;
    }
    fim_relogio = clock();
    metricas.tempo = (double)(fim_relogio - inicio_relogio) / CLOCKS_PER_SEC;
    if (config.p) {
        printf("\n================ ARQUIVO ORDENADO (-P) ================\n");
        FILE *resultado = fopen("resultado_final.txt", "r");
        if (resultado) {
            Registro reg;
            while (lerRegistroTexto(resultado, &reg, NULL)) {
                imprimirRegistro(&reg);
            }
            fclose(resultado);
        } else {
            printf("Aviso: Arquivo 'resultado_final.txt' nao foi encontrado para impressao.\n");
        }
        printf("========================================================\n");
    }
    printMetricas(metricas);

    return 0;
}