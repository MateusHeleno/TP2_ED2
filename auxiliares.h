#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdbool.h>
#include <stdio.h>

typedef struct {
    long int inscricao;
    float nota;
    char estado[3];
    char cidade[51];
    char curso[31];
} Registro;

typedef struct {
    int metodo;
    int qnt_registros;
    int situacao;
    bool p;
} Config;

typedef struct {
    int ler_reg;
    int escrita_reg;
    int comparacoes;
    double tempo;
} Metricas;

typedef struct {
    Registro reg;
    int fita_origem;
} RegFita;

bool validaEntrada(int argc, char *argv[], Config *config);
void inicializaMetricas(Metricas *metricas);
void printMetricas(Metricas metricas, Config config);

bool abrirFitas(FILE *fitas[], int inicio, int fim, const char *modo);
void fecharFitas(FILE *fitas[], int inicio, int fim);
bool lerRegistroTexto(FILE *arq, Registro *reg, Metricas *metricas);
bool prepararArquivoInicial(Config *config, Metricas *metricas);
void gravarRegistroTexto(FILE *arq, Registro *reg, Metricas *metricas);
void imprimirRegistro(Registro *reg);
void quicksortInterno(Registro *vetor, int esq, int dir, Metricas *metricas);
void insertionSortFitas(RegFita *vetor, int tamanho, Metricas *metricas);

#endif