#include <stdio.h>
#include <stdlib.h>

#include "auxiliares.h"
#include "balanceadaOI.h"
#include "heap.h"

// gera blocos ordenados de tamanho igual
void gerarBlocosOrdenadosOI(const char *nomeArquivo, int quantidade, Metricas *metricas) {
    FILE *arqEntrada = abrirArquivo(nomeArquivo, "rb");
    if (!arqEntrada) {
        printf("Erro ao abrir arquivo base: %s\n", nomeArquivo);
        return;
    }

    FILE *fitas[TAM_FITAS]; // gera as 40 fitas - TAM_RAM de entrada e 20 de saida

    if (!abrirFitas(fitas, 0, TAM_RAM, "wb")) {  // tenta abrir as fitas de entrada
        printf("Erro ao criar fitas temporarias.\n");
        fclose(arqEntrada);
        return;
    }

    Registro memoria[TAM_RAM]; // vetor de TAM_RAM reg do tamanho da memoria
    int lidosTotal = 0;
    int fitaAtual = 0;

    while (lidosTotal < quantidade) { // para ler a quantidade solicitada pela chamada
        int lidosBloco = 0;

        // para ler um bloco de TAM_RAM reg
        while (lidosBloco < TAM_RAM && lidosTotal < quantidade && lerRegistro(arqEntrada, &memoria[lidosBloco], metricas)) {
            lidosBloco++;
            lidosTotal++;
        }

        if (lidosBloco > 0) {// ordena o bloco com um quicksort interno
            quicksortInterno(memoria, 0, lidosBloco - 1, metricas);
            for (int i = 0; i < lidosBloco; i++)
                gravarRegistro(fitas[fitaAtual], &memoria[i], metricas);

            fitaAtual = (fitaAtual + 1) % TAM_RAM;
        }
    }

    fecharFitas(fitas, 0, TAM_RAM);
    fclose(arqEntrada);
}

void intercalacaoOI(Config *config, Metricas *metricas) {
    const char *arq;
    if (config->situacao == 1)      arq = "arquivos/ascendente.bin";
    else if (config->situacao == 2) arq = "arquivos/descendente.bin";
    else if (config->situacao == 3) arq = "arquivos/random.bin";

    gerarBlocosOrdenadosOI(arq, config->qnt_registros, metricas);

    int entradaBase = 0;
    int saidaBase = TAM_FITAS / 2;

    int tamanhoBlocoAtual = TAM_RAM;

    bool ordenado = false;

    char nomeFita[50];

    FILE *fitasIn[(TAM_FITAS / 2)];
    FILE *fitasOut[(TAM_FITAS / 2)];

    Registro prox_reg[TAM_RAM];
    bool fitaTemDado[TAM_RAM];

    while (!ordenado) {
        // abre fitas de entrada
        for (int i = 0; i < (TAM_FITAS / 2); i++)
        {
            sprintf(nomeFita,
                    "fitas/fita%02d.bin",
                    entradaBase + i);

            fitasIn[i] = abrirArquivo(nomeFita, "rb");

            if (fitasIn[i])
            {
                fitaTemDado[i] =
                    lerRegistro(fitasIn[i],&prox_reg[i],metricas);
            }
            else
            {
                fitaTemDado[i] = false;
            }
        }

        // abre fitas de saída
        for (int i = 0; i < (TAM_FITAS / 2); i++)
        {
            sprintf(nomeFita,
                    "fitas/fita%02d.bin",
                    saidaBase + i);

            fitasOut[i] = abrirArquivo(nomeFita, "wb");
        }

        int blocosGerados = 0;
        int saidaAtual = 0;

        while (1)
        {
            MinHeap heap;
            heap.tamanho = 0;

            int lidosBloco[TAM_RAM] = {0};

            // coloca um registro de cada bloco no heap
            for (int i = 0; i < TAM_RAM; i++)
            {
                if (fitaTemDado[i])
                {
                    NoHeap no;

                    no.reg = prox_reg[i];
                    no.fita_origem = i;
                    no.marcado = false;

                    heap.dados[heap.tamanho++] = no;

                    lidosBloco[i] = 1;

                    fitaTemDado[i] =
                        lerRegistro(fitasIn[i],&prox_reg[i],metricas);
                }
            }

            if (heap.tamanho == 0)
                break;

            construirMinHeap(&heap, metricas);

            blocosGerados++;

            while (heap.tamanho > 0)
            {
                NoHeap menor = heap.dados[0];

                int f = menor.fita_origem;

                gravarRegistro(fitasOut[saidaAtual],&menor.reg,metricas
                );

                if (lidosBloco[f] < tamanhoBlocoAtual &&
                    fitaTemDado[f])
                {
                    NoHeap substituto;

                    substituto.reg = prox_reg[f];
                    substituto.fita_origem = f;
                    substituto.marcado = false;

                    substituirRaiz(
                        &heap,
                        substituto,
                        metricas
                    );

                    lidosBloco[f]++;

                    fitaTemDado[f] =
                        lerRegistro(fitasIn[f],&prox_reg[f],metricas);
                }
                else
                {
                    removerRaiz(&heap, metricas);
                }
            }

            saidaAtual = (saidaAtual + 1) % (TAM_FITAS / 2);
        }

        // fecha tudo
        for (int i = 0; i < TAM_FITAS / 2; i++)
        {
            if (fitasIn[i])
                fclose(fitasIn[i]);

            if (fitasOut[i])
                fclose(fitasOut[i]);
        }

        if (blocosGerados <= 1)
        {
            ordenado = true;

            sprintf(nomeFita,
                    "fitas/fita%02d.bin",
                    saidaBase);

            remove("resultado_final.bin");
            rename(nomeFita,
                   "resultado_final.bin");
        }
        else
        {
            int temp = entradaBase;

            entradaBase = saidaBase;
            saidaBase = temp;

            tamanhoBlocoAtual *= TAM_FITAS / 2;
        }
    }
}