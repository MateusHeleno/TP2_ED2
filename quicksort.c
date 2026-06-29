#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "auxiliares.h"
#include "quicksort.h"

#define TAM_MEMORIA 20

bool lerRegistroNaPosicao(FILE *arq, Registro *reg, long pos, Metricas *metricas) {
    if (fseek(arq, pos * sizeof(Registro), SEEK_SET) != 0)
        return false;

    return lerRegistroTexto(arq, reg, metricas);
}

bool gravarRegistroNaPosicao(FILE *arq, Registro *reg, long pos, Metricas *metricas) {
    if (fseek(arq, pos * sizeof(Registro), SEEK_SET) != 0)
        return false;

    gravarRegistroTexto(arq, reg, metricas);
    return true;
}

// Insere mantendo a área ordenada (RAM) para o pivô
void inserirAreaOrdenada(Registro area[], int *qtd, Registro *reg, Metricas *metricas) {
    int i = *qtd - 1;
    while (i >= 0) {
        if (metricas) metricas->comparacoes++;
        if (area[i].nota > reg->nota) {
            area[i + 1] = area[i];
            i--;
        } else {
            break;
        }
    }
    area[i + 1] = *reg;
    (*qtd)++;
}

void particao(FILE *arq, int esq, int dir, int *i, int *j, Metricas *metricas) {
    int li = esq, ei = esq;
    int ls = dir, es = dir;

    int nrarea = 0;
    double linf = -1.0; // Equivalente ao -infinito para notas
    double lsup = 101.0; // Equivalente ao +infinito para notas

    Registro area[TAM_MEMORIA];
    Registro ultLido;
    bool ondeLer = true; // true = Ler Inferior, false = Ler Superior

    *i = esq - 1;
    *j = dir + 1;

    while (ls >= li) {
        // Fase inicial: Preenche a área (pivô) com os primeiros TAM_MEMORIA - 1 elementos
        if (nrarea < TAM_MEMORIA - 1) {
            if (ondeLer) {
                lerRegistroNaPosicao(arq, &ultLido, li, metricas);
                li++; ondeLer = false;
            } else {
                lerRegistroNaPosicao(arq, &ultLido, ls, metricas);
                ls--; ondeLer = true;
            }
            inserirAreaOrdenada(area, &nrarea, &ultLido, metricas);
            continue;
        }

        // Evita que ponteiros de escrita interceptem/atropelem os de leitura
        if (ls == es) {
            lerRegistroNaPosicao(arq, &ultLido, ls, metricas);
            ls--; ondeLer = true;
        } else if (li == ei) {
            lerRegistroNaPosicao(arq, &ultLido, li, metricas);
            li++; ondeLer = false;
        } else if (ondeLer) {
            lerRegistroNaPosicao(arq, &ultLido, li, metricas);
            li++; ondeLer = false;
        } else {
            lerRegistroNaPosicao(arq, &ultLido, ls, metricas);
            ls--; ondeLer = true;
        }

        // Avalia onde alocar o registro baseado nos limites do pivô (área)
        if (metricas) metricas->comparacoes++;
        if (ultLido.nota > lsup) {
            *j = es;
            gravarRegistroNaPosicao(arq, &ultLido, es, metricas);
            es--;
        }
        else if (ultLido.nota < linf) {
            *i = ei;
            gravarRegistroNaPosicao(arq, &ultLido, ei, metricas);
            ei++;
        }
        else {
            // Se cair no meio, entra no pivô (área) e expulsa o extremo para balancear as partições
            inserirAreaOrdenada(area, &nrarea, &ultLido, metricas);

            int t1 = ei - esq;
            int t2 = dir - es;

            if (t1 < t2) { // Partição inferior menor -> expulsa o mínimo
                *i = ei;
                gravarRegistroNaPosicao(arq, &area[0], ei, metricas);
                linf = area[0].nota;
                ei++;
                // Shift para remover o primeiro item da área
                for (int k = 0; k < nrarea - 1; k++) area[k] = area[k + 1];
                nrarea--;
            } else { // Partição superior menor ou igual -> expulsa o máximo
                *j = es;
                gravarRegistroNaPosicao(arq, &area[nrarea - 1], es, metricas);
                lsup = area[nrarea - 1].nota;
                es--;
                nrarea--;
            }
        }
    }

    // Descarrega o pivô que restou ordenado na RAM de volta para o meio do arquivo
    while (ei <= es) {
        gravarRegistroNaPosicao(arq, &area[0], ei, metricas);
        ei++;
        for (int k = 0; k < nrarea - 1; k++) area[k] = area[k + 1];
        nrarea--;
    }
}

void quickSortExternoRec(FILE* arq, int esq, int dir, Metricas *metricas) {
    if (dir - esq < 1)
        return;

    int i, j;
    particao(arq, esq, dir, &i, &j, metricas);

    if (i - esq < dir - j) {
        quickSortExternoRec(arq, esq, i, metricas);
        quickSortExternoRec(arq, j, dir, metricas);
    }
    else {
        quickSortExternoRec(arq, j, dir, metricas);
        quickSortExternoRec(arq, esq, i, metricas);
    }
}

/*
    O quickSortExterno recebe a struct Config, que contém a situação do arq (ascendente, etc)
    e baseado nisso abre o arquivo correto.
*/
void quicksortExterno(Config *config, Metricas *metricas) {
    FILE *arq;
    if (config->situacao == 1) arq = fopen("arquivos/ascendente.bin", "rb+");
    else if (config->situacao == 2) arq = fopen("arquivos/descendente.bin", "rb+");
    else if (config->situacao == 3) arq = fopen("arquivos/random.bin", "rb+");

    if (!arq) {
        printf("Nao foi possivel abrir o arquivo de leitura para o Quicksort Externo.\n");
        return;
    }

    quickSortExternoRec(arq, 0, config->qnt_registros - 1, metricas);
}