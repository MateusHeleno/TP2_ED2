#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "auxiliares.h"
#include "quicksort.h"

// Tamanho do registro
#define TAM_REGISTRO_TXT 100
#define TAM_MEMORIA 20

bool lerRegistroNaPosicao(FILE *arq, Registro *reg, int pos, Metricas *metricas)
{
    if (fseek(arq, (long)pos * TAM_REGISTRO_TXT, SEEK_SET) != 0)
        return false;
    return lerRegistroTexto(arq, reg, metricas);
}

bool gravarRegistroNaPosicao(FILE *arq, Registro *reg, int pos, Metricas *metricas)
{
    if (fseek(arq, (long)pos * TAM_REGISTRO_TXT, SEEK_SET) != 0)
        return false;
    gravarRegistroTexto(arq, reg, metricas);
    return true;
}

int compararRegistros(const void *a, const void *b)
{
    float notaA = ((Registro *)a)->nota;
    float notaB = ((Registro *)b)->nota;
    if (notaA < notaB)
        return -1;
    if (notaA > notaB)
        return 1;
    return 0;
}

void quicksortExternoRec(FILE *arq, int esq, int dir, Metricas *metricas)
{
    if (dir - esq < 1)
        return;

    // Se o tamanho do arquivo for menor que o tamanho ordenamos tudo na memória
    if (dir - esq + 1 <= TAM_MEMORIA)
    {
        Registro area[TAM_MEMORIA];
        int qtd = dir - esq + 1;

        for (int k = 0; k < qtd; k++)
        {
            lerRegistroNaPosicao(arq, &area[k], esq + k, metricas);
        }

        // ordena usando o quicksort interno da auxiliares.c
        quicksortInterno(area, 0, TAM_MEMORIA - 1, metricas);

        for (int k = 0; k < qtd; k++)
        {
            gravarRegistroNaPosicao(arq, &area[k], esq + k, metricas);
        }
        return;
    }

    Registro area[TAM_MEMORIA];
    int leiInf = esq, escInf = esq, leiSup = dir, escSup = dir;

    for (int k = 0; k < TAM_MEMORIA; k++)
    {
        lerRegistroNaPosicao(arq, &area[k], leiSup, metricas);
        leiSup--;
    }

    // Ordena os 20 registros na RAM
    quicksortInterno(area, 0, TAM_MEMORIA - 1, metricas);

    bool saiEsq = true;
    Registro novo;

    // Le o restante do arquivo da esquerda para a direita
    while (leiInf <= leiSup)
    {
        lerRegistroNaPosicao(arq, &novo, leiInf, metricas);
        leiInf++;

        if (metricas)
            metricas->comparacoes++;

        // Se a nota for menor que o menor da area, vai para a particao esquerda
        if (novo.nota < area[0].nota)
        {
            gravarRegistroNaPosicao(arq, &novo, escInf, metricas);
            escInf++;
        }
        else
        {
            if (metricas)
                metricas->comparacoes++;
            // Se a nota for maior que o maior da area, vai para a partição direita
            if (novo.nota > area[TAM_MEMORIA - 1].nota)
            {
                gravarRegistroNaPosicao(arq, &novo, escSup, metricas);
                escSup--;
            }
            else
            {
                if (saiEsq)
                {
                    // Expulsa o menor registro para o disco
                    gravarRegistroNaPosicao(arq, &area[0], escInf, metricas);
                    escInf++;
                    area[0] = novo; // O novo registro entra

                    for (int k = 0; k < TAM_MEMORIA - 1; k++)
                    {
                        if (metricas)
                            metricas->comparacoes++;
                        if (area[k].nota > area[k + 1].nota)
                        {
                            Registro tmp = area[k];
                            area[k] = area[k + 1];
                            area[k + 1] = tmp;
                        }
                        else
                            break;
                    }
                    saiEsq = false; // Expulsa pela direita
                }
                else
                {
                    // Expulsa o maior registro para o disco
                    gravarRegistroNaPosicao(arq, &area[TAM_MEMORIA - 1], escSup, metricas);
                    escSup--;
                    area[TAM_MEMORIA - 1] = novo; // O novo registro entra

                    for (int k = TAM_MEMORIA - 1; k > 0; k--)
                    {
                        if (metricas)
                            metricas->comparacoes++;
                        if (area[k].nota < area[k - 1].nota)
                        {
                            Registro tmp = area[k];
                            area[k] = area[k - 1];
                            area[k - 1] = tmp;
                        }
                        else
                            break;
                    }
                    saiEsq = true; // Expulsa pela esquerda
                }
            }
        }
    }

    // Joga o restante que esta na RAM para o centro
    for (int k = 0; k < TAM_MEMORIA; k++)
    {
        gravarRegistroNaPosicao(arq, &area[k], escInf + k, metricas);
    }
    quicksortExternoRec(arq, esq, escInf - 1, metricas);
    quicksortExternoRec(arq, escSup + 1, dir, metricas);
}

void metodo3_QuicksortExterno(Config *config, Metricas *metricas)
{
    FILE *arq = fopen("entrada_atual.txt", "r+");
    if (!arq)
    {
        printf("Erro: Nao foi possivel abrir o arquivo de trabalho para o Quicksort Externo.\n");
        return;
    }

    quicksortExternoRec(arq, 0, config->qnt_registros - 1, metricas);
    fclose(arq);

    FILE *origem = fopen("entrada_atual.txt", "r");
    FILE *destino = fopen("resultado_final.txt", "w");

    if (origem && destino)
    {
        Registro r;
        while (lerRegistroTexto(origem, &r, NULL))
        {
            gravarRegistroTexto(destino, &r, NULL);
        }
    }

    if (origem)
        fclose(origem);
    if (destino)
        fclose(destino);
}