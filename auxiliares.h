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

// Config
bool validaEntrada(int argc, char *argv[], Config *config);

// Metricas
void inicializaMetricas(Metricas *metricas);
void printMetricas(Metricas metricas, Config config);

// Arquivos
bool arquivoExiste(const char *nome);
long quantidadeRegistros(FILE *arq);
FILE *abrirArquivo(const char *nome,const char *modo);
void criarPastaSeNaoExistir(const char *nome);

// Registros
bool lerRegistro(FILE *arq,Registro *reg,Metricas *metricas);
void gravarRegistro(FILE *arq,Registro *reg,Metricas *metricas);
void imprimirRegistro(Registro *reg);

// Fitas
bool abrirFitas(FILE *fitas[],int inicio,int fim,const char *modo);
void fecharFitas(FILE *fitas[],int inicio,int fim);

// Preparação
bool prepararArquivoInicial(Config *config);
bool gerarArquivoRandom(const char *origem);
bool gerarArquivoAscendente();
bool gerarArquivoDescendente();

//sort
void quicksortInterno(Registro *vetor, int esq, int dir, Metricas *metricas);
void insertionSortFitas(RegFita *vetor, int tamanho, Metricas *metricas);
#endif