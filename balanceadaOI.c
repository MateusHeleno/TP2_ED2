#include <stdio.h>
#include <stdlib.h>
#include "auxiliares.h"

// gera blocos ordenados de tamanho igual
void gerarBlocosOrdenadosOI(const char *nomeArquivo, int quantidade, Metricas *metricas)
{
    FILE *arqEntrada = fopen(nomeArquivo, "r");
    if (!arqEntrada)
    {
        printf("Erro ao abrir arquivo base: %s\n", nomeArquivo);
        return;
    }

    FILE *fitas[40]; // gera as 40 fitas - TAM_RAM de entrada e 20 de saida

    if (!abrirFitas(fitas, 0, TAM_RAM, "w")) // tenta abrir as fitas de entrada
    {
        printf("Erro ao criar fitas temporarias.\n");
        fclose(arqEntrada);
        return;
    }

    Registro memoria[TAM_RAM]; // vetor de TAM_RAM reg do tamanho da memoria
    int lidosTotal = 0;
    int fitaAtual = 0;

    while (lidosTotal < quantidade) // para ler a quantidade solicitada pela chamada
    {
        int lidosBloco = 0;

        // para ler um bloco de TAM_RAM reg
        while (lidosBloco < TAM_RAM && lidosTotal < quantidade && lerRegistroTexto(arqEntrada, &memoria[lidosBloco], metricas))
        {
            lidosBloco++;
            lidosTotal++;
        }

        if (lidosBloco > 0) // ordena o bloco com um quicksort interno
        {
            quicksortInterno(memoria, 0, lidosBloco - 1, metricas);

            for (int i = 0; i < lidosBloco; i++)
            {
                gravarRegistroTexto(fitas[fitaAtual], &memoria[i], metricas);
            }

            fitaAtual = (fitaAtual + 1) % TAM_RAM;
        }
    }

    fecharFitas(fitas, 0, TAM_RAM);
    fclose(arqEntrada);
}

void intercalacaoOI(Config *config, Metricas *metricas)
{
    gerarBlocosOrdenadosOI("entrada_atual.bin",
                           config->qnt_registros,
                           metricas);

    int entradaBase = 0;
    int saidaBase = 20;

    int tamanhoBlocoAtual = 20;

    bool ordenado = false;

    char nomeFita[50];

    FILE *fitasIn[20];
    FILE *fitasOut[20];

    Registro prox_reg[20];
    bool fitaTemDado[20];

    while (!ordenado)
    {
        // abre fitas de entrada
        for (int i = 0; i < 20; i++)
        {
            sprintf(nomeFita,
                    "fitas/fita%02d.bin",
                    entradaBase + i);

            fitasIn[i] = fopen(nomeFita, "rb");

            if (fitasIn[i])
            {
                fitaTemDado[i] =
                    lerRegistroBinario(
                        fitasIn[i],
                        &prox_reg[i],
                        metricas
                    );
            }
            else
            {
                fitaTemDado[i] = false;
            }
        }

        // abre fitas de saída
        for (int i = 0; i < 20; i++)
        {
            sprintf(nomeFita,
                    "fitas/fita%02d.bin",
                    saidaBase + i);

            fitasOut[i] = fopen(nomeFita, "wb");
        }

        int blocosGerados = 0;
        int saidaAtual = 0;

        while (1)
        {
            MinHeap heap;
            heap.tamanho = 0;

            int lidosBloco[20] = {0};

            // coloca um registro de cada bloco no heap
            for (int i = 0; i < 20; i++)
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
                        lerRegistroBinario(
                            fitasIn[i],
                            &prox_reg[i],
                            metricas
                        );
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

                gravarRegistroBinario(
                    fitasOut[saidaAtual],
                    &menor.reg,
                    metricas
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
                        lerRegistroBinario(
                            fitasIn[f],
                            &prox_reg[f],
                            metricas
                        );
                }
                else
                {
                    removerRaiz(&heap, metricas);
                }
            }

            saidaAtual = (saidaAtual + 1) % 20;
        }

        // fecha tudo
        for (int i = 0; i < 20; i++)
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

            tamanhoBlocoAtual *= 20;
        }
    }
}