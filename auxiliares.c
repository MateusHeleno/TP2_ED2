#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxiliares.h"

bool validaEntrada(int argc, char *argv[], Config *config)
{
    if (argc < 4 || argc > 5)
    {
        printf("O formato é inválido.\nSiga esse formato: ./exe <método> <quantidade> <situação> [-P]\n");
        return false;
    }

    config->metodo = atoi(argv[1]);
    if (config->metodo < 1 || config->metodo > 3)
    { // verifica se o valor recebido é válido
        printf("Método deve ter os valores entre 1 e 3\n");
        return false;
    }

    config->qnt_registros = atoi(argv[2]);
    if (config->qnt_registros > 471705)
    {
        printf("A quantidade de registros não pode ser maior do que 471.705!\n");
        return false;
    }
    bool aux;
    for (int i = 100; i <= 100000; i = i * 10)
    {
        if (config->qnt_registros == i || config->qnt_registros == 471705)
        {
            aux = true;
            break;
        }
        aux = false;
    }

    if (!aux)
    {
        printf("A quantidade de registros deve ser exatamente: 100, 1000, 10000, 100000 ou 471705.\n");
        return false;
    }

    config->situacao = atoi(argv[3]);
    if (config->situacao < 1 || config->situacao > 3)
    { // verifica se o valor recebido é válido
        printf("Situação deve ter os valores entre 1 e 3\n");
        return false;
    }

    if (argc == 5)
        if (strcmp(argv[4], "-P") == 0)
        {
            config->p = 1;
        }
        else
        {
            printf("O quinto argumento deve ser '-P' ou vazio \n");
            return false;
        }
    else
        config->p = 0;

    return true;
}

void inicializaMetricas(Metricas *metricas)
{
    metricas->ler_reg = 0;
    metricas->escrita_reg = 0;
    metricas->comparacoes = 0;
    metricas->tempo = 0.0;
}

void printMetricas(Metricas metricas)
{
    printf("\n\t\tMétricas \n");
    printf("  Leituras    : %6d \n", metricas.ler_reg);
    printf("  Escrita     : %6d \n", metricas.escrita_reg);
    printf("  Comparações    : %6d \n", metricas.comparacoes);
    printf("  Tempo Total    : %.6lf s\n\n", metricas.tempo);
}

int *criaVetor(int tamanho)
{
    int *vetor = (int *)malloc(sizeof(int) * tamanho);
    if (!vetor)
    {
        printf("Erro ao alocar memória para o vetor.\n");
        exit(1);
    }
    return vetor;
}

void destroiVetor(int *vet)
{
    free(vet);
}

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
    int menor = i;       // posição recebida
    int esq = 2 * i + 1; // pos do filho da esq no vetor
    int dir = 2 * i + 2; // pos do filho da dir no vetor

    if (esq < heap->tamanho)
    {
        metricas->comparacoes++;
        if (!compararNos(heap->dados[esq], heap->dados[menor]))
        {
            menor = esq;
        }
    }

    if (dir < heap->tamanho)
    {
        metricas->comparacoes++;
        if (!compararNos(heap->dados[dir], heap->dados[menor])) // s eo menor foi atualizado antes, ele ta compaando os irmaos
        {
            menor = dir;
        }
    }

    if (menor != i)
    {
        trocarNos(&heap->dados[i], &heap->dados[menor]);
        descerNoHeap(heap, menor, metricas);
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

bool lerRegistroBin(FILE *arq, Registro *reg, Metricas *metricas)
{
    if (fread(reg, sizeof(Registro), 1, arq) == 1)
    {
        metricas->ler_reg++;
        return true;
    }
    return false;
}

void escreverRegistroBin(FILE *arq, Registro *reg, Metricas *metricas)
{
    fwrite(reg, sizeof(Registro), 1, arq);
    metricas->escrita_reg++;
}

void lerEExibirRegistros(const char *nomeArquivo, int quantidade, bool imprimir, Metricas *metricas)
{
    FILE *arq = fopen(nomeArquivo, "rb");
    if (arq == NULL)
    {
        printf("Erro ao abrir o arquivo %s para leitura.\n", nomeArquivo);
        return;
    }

    Registro reg;
    int lidos = 0;

    while (lidos < quantidade && lerRegistroBin(arq, &reg, metricas))
    {
        lidos++;

        if (imprimir) // imprime registros quando a flag -P for aplicada
        {
            printf("Inscricao: %08ld | Nota: %5.1f | Estado: %s | Cidade: %-25s | Curso: %s\n",
                   reg.inscricao, reg.nota, reg.estado, reg.cidade, reg.curso);
        }
    }

    if (lidos < quantidade)
    {
        printf("Aviso: O arquivo continha apenas %d registros, mas %d foram solicitados.\n", lidos, quantidade);
    }

    fclose(arq);
}