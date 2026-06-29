#include <stdio.h>
#include <stdlib.h>

#include "auxiliares.h"
#include "heap.h"
#include "intercalacaoSS.h"

// fase de pre-processamento gera blocos de tamanhos variados nas fitas
void gerarBlocosOrdenadosSubstituicao(const char *nomeArquivo, int quantidade, Metricas *metricas) {
    FILE *arqEntrada = fopen(nomeArquivo, "rb");

    if (!arqEntrada) {
        printf("Erro ao abrir arquivo base: %s\n", nomeArquivo);
        return;
    }

    FILE *fitas[TAM_FITAS];

    if (!abrirFitas(fitas, 0, (TAM_FITAS / 2), "wb")) {
        printf("Erro ao criar fitas temporarias.\n");
        fclose(arqEntrada);
        return;
    }

    MinHeap heap;
    heap.tamanho = 0;

    int lidos_total = 0;

    // preenche o heap inicial
    while (heap.tamanho < TAM_RAM && lidos_total < quantidade) {
        if (lerRegistro(arqEntrada,&heap.dados[heap.tamanho].reg,metricas)) {
            heap.dados[heap.tamanho].fita_origem = 0;
            heap.dados[heap.tamanho].marcado = false;

            heap.tamanho++;
            lidos_total++;
        }
        else
            break;

    }

    construirMinHeap(&heap, metricas);

    int fita_atual = 0;

    while (heap.tamanho > 0) {
        // menor elemento da run atual
        NoHeap menor = heap.dados[0];

        gravarRegistro(fitas[fita_atual],&menor.reg,metricas);

        Registro proximo;

        if (lidos_total < quantidade && lerRegistro(arqEntrada,&proximo,metricas)) {
            lidos_total++;

            NoHeap novoNo;

            novoNo.reg = proximo;
            novoNo.fita_origem = 0;

            if (metricas)
                metricas->comparacoes++;

            // se for menor que o último removido,
            // pertence à próxima run
            novoNo.marcado = (proximo.nota < menor.reg.nota);

            substituirRaiz( &heap,novoNo,metricas);
        }
        else // acabou o arquivo original
            removerRaiz(&heap, metricas);

        // todos os elementos restantes pertencem a proxima run
        if (heap.tamanho > 0 && heap.dados[0].marcado) {
            fita_atual = (fita_atual + 1) % (TAM_FITAS / 2);

            // desmarca todos
            for (int i = 0; i < heap.tamanho; i++)
                heap.dados[i].marcado = false;

            construirMinHeap(&heap, metricas);
        }
    }

    fecharFitas(fitas, 0, (TAM_FITAS / 2));

    fclose(arqEntrada);
}

void intercalacaoSS(Config *config, Metricas *metricas) {
    gerarBlocosOrdenadosSubstituicao("entrada_atual.bin",config->qnt_registros,metricas);

    int entrada_base = 0;
    int saida_base = (TAM_FITAS / 2);

    bool ordenado = false;

    char nomeFita[50];

    FILE *fitas_in[(TAM_FITAS / 2)];
    FILE *fitas_out[(TAM_FITAS / 2)];

    Registro prox_reg[(TAM_FITAS / 2)];
    bool fita_tem_dado[(TAM_FITAS / 2)];

    while (!ordenado) {
        // abre as fitas de entrada
        for (int i = 0; i < (TAM_FITAS / 2); i++) {
            sprintf(nomeFita,"fitas/fita%02d.bin",entrada_base + i);

            fitas_in[i] = fopen(nomeFita, "rb");

            if (fitas_in[i])
                fita_tem_dado[i] = lerRegistro(fitas_in[i],&prox_reg[i],metricas);
            else
                fita_tem_dado[i] = false;
        }

        // abre as fitas de saída
        for (int i = 0; i < (TAM_FITAS / 2); i++) {
            sprintf(nomeFita,"fitas/fita%02d.bin",saida_base + i);
            fitas_out[i] = fopen(nomeFita, "wb");
        }

        int blocos_gerados = 0;
        int saidaAtual = 0;

        while (1) {
            MinHeap heap;
            heap.tamanho = 0;

            // coloca o primeiro registro de cada fita
            for (int i = 0; i < (TAM_FITAS / 2); i++) {
                if (fita_tem_dado[i]) {
                    NoHeap no;

                    no.reg = prox_reg[i];
                    no.fita_origem = i;
                    no.marcado = false;

                    heap.dados[heap.tamanho++] = no;
                }
            }

            if (heap.tamanho == 0)
                break;

            construirMinHeap(&heap, metricas);

            blocos_gerados++;

            while (heap.tamanho > 0) {
                NoHeap menor = heap.dados[0];
                int f = menor.fita_origem;

                gravarRegistro(fitas_out[saidaAtual],&menor.reg,metricas);

                // Lê o próximo elemento da mesma fita que originou o menor
                fita_tem_dado[f] = lerRegistro(fitas_in[f],&prox_reg[f],metricas);

                // Se a fita ainda tem dados, o registro ENTRA DIRETO na heap
                if (fita_tem_dado[f]) {
                    NoHeap novoNo;
                    novoNo.reg = prox_reg[f];
                    novoNo.fita_origem = f;
                    novoNo.marcado = false; // Na intercalação ninguém é marcado

                    substituirRaiz(&heap,novoNo,metricas);
                }
                else
                    removerRaiz(&heap,metricas);
            }

            saidaAtual = (saidaAtual + 1) % (TAM_FITAS / 2);
        }

        // fecha as fitas
        for (int i = 0; i < (TAM_FITAS / 2); i++) {
            if (fitas_in[i])
                fclose(fitas_in[i]);

            if (fitas_out[i])
                fclose(fitas_out[i]);
        }

        if (blocos_gerados <= 1) {
            ordenado = true;

            sprintf(nomeFita,"fitas/fita%02d.bin",saida_base);

            remove("resultado_final.bin");
            rename(nomeFita,"resultado_final.bin");
        }
        else {
            int temp = entrada_base;

            entrada_base = saida_base;
            saida_base = temp;
        }
    }
}