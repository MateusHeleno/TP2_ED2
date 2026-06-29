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

    FILE *fitas[TAM_FITAS]; // gera as 40 fitas - 20 de entrada e 20 de saida

    if (!abrirFitas(fitas, 0, TAM_FITAS_IN, "wb")) {  // tenta abrir as fitas de entrada
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

            fitaAtual = (fitaAtual + 1) % tam TAM_FITAS_IN;
        }
    }

    fecharFitas(fitas, 0, TAM_FITAS_IN);
    fclose(arqEntrada);
}

void intercalacaoOI(Config *config, Metricas *metricas) {
    const char *arq;
    switch (config->situacao)
    {
        case 1:
            arq = "arquivos/ascendente.bin";
            break;

        case 2:
            arq = "arquivos/descendente.bin";
            break;

        case 3:
            arq = "arquivos/random.bin";
            break;

        default:
            return;
    }

    gerarBlocosOrdenadosOI(arq, config->qnt_registros, metricas);

    int entradaBase = 0;
    int saidaBase = TAM_FITAS_IN;

    int tamanhoBlocoAtual = TAM_RAM;

    bool ordenado = false;

    char nomeFita[50];

    FILE *fitasIn[TAM_FITAS_IN];
    FILE *fitasOut[TAM_FITAS_OUT];

    BlocoFita blocos[TAM_FITAS_IN];
    Registro prox_reg[TAM_FITAS_IN];

    while (!ordenado) {
        // abre fitas de entrada
        for (int i = 0; i < TAM_FITAS_IN; i++)
        {
            sprintf(nomeFita,"fitas/fita%02d.bin",entradaBase + i);

            fitasIn[i] = abrirArquivo(nomeFita, "rb");

            blocos[i].fita = fitasIn[i];
            blocos[i].registros_lidos = 0;
            blocos[i].tamanho_bloco = tamanhoBlocoAtual

                if (fitasIn[i])
            {
                blocos[i].ativo =lerRegistro(fitasIn[i],&prox_reg[i],metricas);
            }
            else
            {
                blocos[i].ativo = false;
            }
        }

        // abre fitas de saída
        for (int i = 0; i < TAM_FITAS_OUT; i++)
        {
            sprintf(nomeFita,"fitas/fita%02d.bin",saidaBase + i);

            fitasOut[i] = abrirArquivo(nomeFita, "wb");
        }

        int blocosGerados = 0;
        int saidaAtual = 0;

        while (1)
        {
            MinHeap heap;
            heap.tamanho = 0;

            // coloca um registro de cada bloco no heap
            for (int i = 0; i < TAM_FITAS_IN; i++)
            {
                if (blocos[i].ativo)
                {
                    NoHeap no;

                    no.reg = prox_reg[i];
                    no.fita_origem = i;
                    no.marcado = false;

                    heap.dados[heap.tamanho++] = no;

                    blocos[i].registros_lidos = 1;

                    blocos[i].ativo = lerRegistro(fitasIn[i],&prox_reg[i],metricas);
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

                gravarRegistro(fitasOut[saidaAtual],&menor.reg,metricas);

                if (blocos[f].registros_lidos < blocos[f].tamanho_bloco && blocos[f].ativo)
                {
                    NoHeap substituto;

                    substituto.reg = prox_reg[f];
                    substituto.fita_origem = f;
                    substituto.marcado = false;

                    substituirRaiz(&heap,substituto,metricas);

                    blocos[f].registros_lidos++;

                    blocos[f].ativo =lerRegistro(fitasIn[f],&prox_reg[f],metricas);
                }
                else
                {
                    removerRaiz(&heap, metricas);
                }
            }

            saidaAtual = (saidaAtual + 1) % TAM_FITAS_OUT;
        }

        // fecha tudo
        for (int i = 0; i < TAM_FITAS_IN / 2; i++)
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

            tamanhoBlocoAtual *= TAM_FITAS_IN;
        }
    }
}