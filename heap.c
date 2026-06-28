#include<stdbool.h>
#include<auxiliares.h>

bool compararNos(NoHeap a, NoHeap b)
{
    if (a.marcado != b.marcado) // se os dois forem marcados, fala qual é o maior, se nao fala o marcaddo
    {
        if (a.marcado)
            return true;
        else
            return false;
    }

    if (a.reg.nota < b.reg.nota)
        return false;
    if (a.reg.nota >= b.reg.nota)
        return true;

    return false;
}

void trocarNos(NoHeap *a, NoHeap *b) // so troca os nos
{
    NoHeap aux = *a;
    *a = *b;
    *b = aux;
}

void descerNoHeap(MinHeap *heap, int i, Metricas *metricas)
{
    while (1)
    {
        int menor = i;
        int esq = 2*i + 1;
        int dir = 2*i + 2;

        if (esq < heap->tamanho)
        {
            metricas->comparacoes++;

            if (!compararNos(heap->dados[esq],
                             heap->dados[menor]))
            {
                menor = esq;
            }
        }

        if (dir < heap->tamanho)
        {
            metricas->comparacoes++;

            if (!compararNos(heap->dados[dir],
                             heap->dados[menor]))
            {
                menor = dir;
            }
        }

        if (menor == i)
            break;

        trocarNos(&heap->dados[i],
                  &heap->dados[menor]);

        i = menor;
    }
}
void construirMinHeap(MinHeap *heap, Metricas *metricas)
{
    for (int i = (heap->tamanho / 2) - 1; i >= 0; i--)
    {
        descerNoHeap(heap, i, metricas);
    }
}

void substituirRaiz(MinHeap *heap, NoHeap novoNo, Metricas *metricas)
{
    if (heap->tamanho <= 0)
        return;

    heap->dados[0] = novoNo; // o heap sempre tira a raiz
    descerNoHeap(heap, 0, metricas);
}

void removerRaiz(MinHeap *heap, Metricas *metricas) // se nao tiver nada pra colocar
{
    if (heap->tamanho <= 0)
        return;

    heap->dados[0] = heap->dados[heap->tamanho - 1]; // puxei um pra raiz
    heap->tamanho--;                                 // diminui , pq nao coloquei nd
    descerNoHeap(heap, 0, metricas);                 // e vou refazer
}