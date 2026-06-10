#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdbool.h>

typedef struct{
    int metodo;
    int qnt_registros;
    int situacao;
    bool p;
} Config;

typedef struct{
    int transferencias;
    int comparacoes;
    double tempo;
} Metricas;

bool validaEntrada(int argc, char *argv[],Config *config);
void inicializaMetricas(Metricas *metricas);
void printMetricas(Metricas metricas);
int *criaVetor(int tamanho);
void destroiVetor(int *vet);

#endif