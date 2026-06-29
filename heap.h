#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include "auxiliares.h"

// heap generico
typedef struct {
    Registro reg;
    int fita_origem; // usado na intercalação 0 a 19
    bool marcado;    // usado na seleção por substituição, true se vai para o próximo bloco
} NoHeap;

typedef struct {
    NoHeap dados[20]; // tamanho máximo da memória interna
    int tamanho;
} MinHeap;

bool compararNos(NoHeap a, NoHeap b);
void trocarNos(NoHeap *a, NoHeap *b);
void descerNoHeap(MinHeap *heap, int i, Metricas *metricas);
void construirMinHeap(MinHeap *heap, Metricas *metricas);
void substituirRaiz(MinHeap *heap, NoHeap novoNo, Metricas *metricas);
void removerRaiz(MinHeap *heap, Metricas *metricas);
NoHeap getMenor(MinHeap *heap);

#endif