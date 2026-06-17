#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdbool.h>

typedef struct
{
    long int inscricao;
    float nota;         
    char estado[3];   
    char cidade[51];   
    char curso[31];    
} Registro;

typedef struct{
    int metodo;
    int qnt_registros;
    int situacao;
    bool p;
} Config;

typedef struct{
    int ler_reg;
    int escrita_reg;
    int comparacoes;
    double tempo;
} Metricas;

//heap generico
typedef struct
{
    Registro reg;
    int fita_origem; // usado na intercalação 0 a 19
    bool marcado;    // usado na seleção por substituição, true se vai para o próximo bloco
} NoHeap;

typedef struct
{
    NoHeap dados[20]; // tamanho máximo da memória interna
    int tamanho;
} MinHeap;

bool validaEntrada(int argc, char *argv[],Config *config);
void inicializaMetricas(Metricas *metricas);
void printMetricas(Metricas metricas);
int* criaVetor(int tamanho);
void destroiVetor(int *vet);

bool compararNos(NoHeap a, NoHeap b);
void trocarNos(NoHeap *a, NoHeap *b);
void descerNoHeap(MinHeap *heap, int i, Metricas *metricas);
void construirMinHeap(MinHeap *heap, Metricas *metricas);
void substituirRaiz(MinHeap *heap, NoHeap novoNo, Metricas *metricas);
void removerRaiz(MinHeap *heap, Metricas *metricas);

bool lerRegistroBin(FILE *arq, Registro *reg, Metricas *metricas);
void escreverRegistroBin(FILE *arq, Registro *reg, Metricas *metricas);
void lerEExibirRegistros(const char *nomeArquivo, int quantidade, bool imprimir, Metricas *metricas);
bool abrirFitas(FILE *fitas[], int inicio, int fim, const char *modo);
void fecharFitas(FILE *fitas[], int inicio, int fim);

#endif