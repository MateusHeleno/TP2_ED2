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

    FILE *fitas[40]; // gera as 40 fitas - 20 de entrada e 20 de saida

    if (!abrirFitas(fitas, 0, 20, "w")) // tenta abrir as fitas de entrada
    {
        printf("Erro ao criar fitas temporarias.\n");
        fclose(arqEntrada);
        return;
    }

    Registro memoria[20]; // vetor de 20 reg do tamanho da memoria
    int lidosTotal = 0;
    int fitaAtual = 0;

    while (lidosTotal < quantidade) // para ler a quantidade solicitada pela chamada
    {
        int lidosBloco = 0;

        // para ler um bloco de 20 reg
        while (lidosBloco < 20 && lidosTotal < quantidade && lerRegistroTexto(arqEntrada, &memoria[lidosBloco], metricas))
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

            fitaAtual = (fitaAtual + 1) % 20;
        }
    }

    fecharFitas(fitas, 0, 20);
    fclose(arqEntrada);
}

void intercalacaoOI(Config *config, Metricas *metricas)
{
    gerarBlocosOrdenadosOI("entradaAtual.txt", config->qnt_registros, metricas); // pre-processamento

    // variaveis para controle das fitas
    int entradaBase = 0;
    int saidaBase = 20;
    bool ordenado = false;
    char nomeFita[30];

    FILE *fitasIn[20];
    FILE *fitasOut[20];

    Registro prox_reg[20];
    bool fitaTemDado[20];

    // intercala até todo o arquivo estar em um unico bloco
    while (!ordenado)
    {
        // abre as fitas para a leitura
        for (int i = 0; i < 20; i++)
        {
            sprintf(nomeFita, "fitas/fita%02d.txt", entradaBase + i);
            fitasIn[i] = fopen(nomeFita, "r");
            if (fitasIn[i]) // marca se tem itens nas fitas
            {
                fitaTemDado[i] = lerRegistroTexto(fitasIn[i], &prox_reg[i], metricas);
            }
            else
            {
                fitaTemDado[i] = false;
            }
        }

        // abre as fitas de escrita
        for (int i = 0; i < 20; i++)
        {
            sprintf(nomeFita, "fitas/fita%02d.txt", saidaBase + i);
            fitasOut[i] = fopen(nomeFita, "w");
        }

        int blocos_gerados = 0;
        int saidaAtual = 0; // marca qual e a fita de saida atual

        while (1)
        {
            int num_ativas = 0;
            RegFita ativas[20]; // vetor para a ordenacao interna dos registros das fitas

            // preenche o vetor com o primeiro reg de cada fita
            for (int i = 0; i < 20; i++)
            {
                if (fitaTemDado[i])
                {
                    ativas[num_ativas].reg = prox_reg[i];
                    ativas[num_ativas].fita_origem = i;
                    num_ativas++;
                }
            }

            if (num_ativas == 0)
                break;

            // aplica o insertion sort interno
            insertionSortFitas(ativas, num_ativas, metricas);
            blocos_gerados++;
            float ultimaNota = -1.0;

            while (num_ativas > 0)
            {
                RegFita menor = ativas[0];
                int origem = menor.fita_origem;

                gravarRegistroTexto(fitasOut[saidaAtual], &menor.reg, metricas);
                ultimaNota = menor.reg.nota;

                fitaTemDado[origem] = lerRegistroTexto(fitasIn[origem], &prox_reg[origem], metricas);

                // remove o menor e empurra todos os outros pra esquerda
                for (int i = 0; i < num_ativas - 1; i++)
                {
                    ativas[i] = ativas[i + 1];
                }
                num_ativas--;

                if (fitaTemDado[origem])
                {
                    if (metricas)
                        metricas->comparacoes++;

                    // se a nota do aluno for maior que a ultima, insere no vetor e ordena
                    if (prox_reg[origem].nota >= ultimaNota)
                    {
                        ativas[num_ativas].reg = prox_reg[origem];
                        ativas[num_ativas].fita_origem = origem;
                        num_ativas++;
                        insertionSortFitas(ativas, num_ativas, metricas);
                    }
                    // se nao ele fica salvo no proximo reg
                }
            }
            saidaAtual = (saidaAtual + 1) % 20;
        }

        // fecha as fitas
        for (int i = 0; i < 20; i++)
        {
            if (fitasIn[i])
                fclose(fitasIn[i]);
            if (fitasOut[i])
                fclose(fitasOut[i]);
        }

        if (blocos_gerados <= 1) // se um unico bloco for gerado
        {
            ordenado = true; // marca a flag de ordenado
            sprintf(nomeFita, "fitas/fita%02d.txt", saidaBase);

            FILE *fFinalIn = fopen(nomeFita, "r");
            FILE *fFinalOut = fopen("resultado_final.txt", "w");
            if (fFinalIn && fFinalOut)
            {
                Registro r;
                while (lerRegistroTexto(fFinalIn, &r, NULL))
                {
                    gravarRegistroTexto(fFinalOut, &r, NULL);
                }
            }
            if (fFinalIn)
                fclose(fFinalIn);
            if (fFinalOut)
                fclose(fFinalOut);
        }
        else
        {
            // se houver mais de um bloco troca as fitas de entrada e saida
            int temp = entradaBase;
            entradaBase = saidaBase;
            saidaBase = temp;
        }
    }
}