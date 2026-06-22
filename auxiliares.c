#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxiliares.h"
#include "quicksort.h"

// codigo de criacao de uma pasta pras fitas
#ifdef _WIN32
#include <direct.h>
#define CRIAR_PASTA(caminho) _mkdir(caminho)
#else
#include <sys/stat.h>
#define CRIAR_PASTA(caminho) mkdir(caminho, 0777)
#endif

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
    bool aux = false;
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

bool abrirFitas(FILE *fitas[], int inicio, int fim, const char *modo)
{
    CRIAR_PASTA("fitas");

    char nomeFita[30];
    for (int i = inicio; i < fim; i++)
    {
        sprintf(nomeFita, "fitas/fita%02d.txt", i);
        fitas[i] = fopen(nomeFita, modo);
        if (!fitas[i])
            return false;
    }
    return true;
}

void fecharFitas(FILE *fitas[], int inicio, int fim)
{
    for (int i = inicio; i < fim; i++)
    {
        if (fitas[i])
        {
            fclose(fitas[i]);
            fitas[i] = NULL;
        }
    }
}

bool lerRegistroTexto(FILE *arq, Registro *reg, Metricas *metricas)
{
    char linha[200];

    // Tenta ler uma linha inteira do arquivo.
    if (fgets(linha, sizeof(linha), arq) == NULL)
        return false;

    // Se a linha for vazia ou curta
    if (strlen(linha) < 5)
        return false;

    // Retirada dos dados especificos

    // Inscrição
    char insc[9] = {0};
    strncpy(insc, &linha[0], 8);
    reg->inscricao = atol(insc); // Converte o texto para long

    // Nota
    char notaStr[6] = {0};
    strncpy(notaStr, &linha[9], 5);
    reg->nota = atof(notaStr); // Converte o texto para float

    // Estado
    strncpy(reg->estado, &linha[15], 2);
    reg->estado[2] = '\0';

    // Cidade
    strncpy(reg->cidade, &linha[18], 50);
    reg->cidade[50] = '\0';

    // Curso
    strncpy(reg->curso, &linha[69], 30);
    reg->curso[30] = '\0';

    if (metricas)
        metricas->ler_reg++;
    return true;
}

bool prepararArquivoInicial(Config *config, Metricas *metricas)
{
    // Tenta abrir o arquivo base
    FILE *origem = fopen("PROVAO.TXT", "r");
    if (!origem)
    {
        printf("Erro: Arquivo PROVAO.TXT nao encontrado no diretorio.\n");
        return false;
    }

    // Tenta abrir o arquivo a ser preparado
    FILE *destino = fopen("entrada_atual.txt", "w");
    if (!destino)
    {
        fclose(origem);
        return false;
    }

    // Copia as N primeiras linhas do arquivo de dados para o que vai ser preparado
    NoHeap no;
    int totalLidos = 0;
    while (totalLidos < config->qnt_registros && lerRegistroTexto(origem, &no.reg, NULL))
    {
        gravarRegistroTexto(destino, &no.reg, NULL);
        totalLidos++;
    }
    fclose(origem);
    fclose(destino);

    // Situacao 1
    if (config->situacao == 1)
    {
        // Metricas falsas para a fase do preparo
        Metricas metricasPreparo;
        inicializaMetricas(&metricasPreparo);

        // Usando o quicksort para fazer a ordenacao inicial solicitada
        metodo3_QuicksortExterno(config, &metricasPreparo);
    }

    // Situacao 2
    else if (config->situacao == 2)
    {
        // Metricas falsas para a fase do preparo
        Metricas metricasPreparo;
        inicializaMetricas(&metricasPreparo);

        // Ordena crescentemente primeiro
        metodo3_QuicksortExterno(config, &metricasPreparo);

        // Inversao
        FILE *fAsc = fopen("resultado_final.txt", "r");     // Arquivo final do quicksort
        FILE *fDesc = fopen("entrada_atual_desc.txt", "w"); // Arquivo que recebera invertido

        if (fAsc && fDesc)
        {
            Registro r;
            // Le de tras para frente
            for (int i = config->qnt_registros - 1; i >= 0; i--)
            {
                fseek(fAsc, (long)i * 100, SEEK_SET);
                if (lerRegistroTexto(fAsc, &r, NULL))
                {
                    gravarRegistroTexto(fDesc, &r, NULL);
                }
            }
        }

        if (fAsc)
            fclose(fAsc);
        if (fDesc)
            fclose(fDesc);

        // Troca o nome do arquivo preparado
        remove("entrada_atual.txt");
        rename("entrada_atual_desc.txt", "entrada_atual.txt");
    }
    //-P
    if (config->p)
    {
        // Imprime o arquivo
        FILE *fVerificacao = fopen("entrada_atual.txt", "r");
        if (fVerificacao)
        {
            Registro r;
            while (lerRegistroTexto(fVerificacao, &r, NULL))
            {
                printf("[Preparo Inicial] ");
                imprimirRegistro(&r);
            }
            fclose(fVerificacao);
        }
    }

    return true;
}

void gravarRegistroTexto(FILE *arq, Registro *reg, Metricas *metricas)
{
    // Gravacao no arquivo
    //%08ld  : Inscrição
    //%5.1f  : Nota
    //%-2s   : Estado
    //%-50s  : Cidade
    //%-30s  : Curso
    fprintf(arq, "%08ld %5.1f %-2s %-50s %-30s\n",
            reg->inscricao,
            reg->nota,
            reg->estado,
            reg->cidade,
            reg->curso);
    if (metricas != NULL)
    {
        metricas->escrita_reg++;
    }
}

void imprimirRegistro(Registro *reg)
{
    printf("%08ld | Nota: %5.1f | %s | %-20s | %-20s\n",
           reg->inscricao,
           reg->nota,
           reg->estado,
           reg->cidade,
           reg->curso);
}

void quicksortInterno(Registro *vetor, int esq, int dir, Metricas *metricas)
{
    int i = esq, j = dir;
    Registro pivo = vetor[(esq + dir) / 2];

    while (i <= j)
    {
        if (metricas)
            metricas->comparacoes++;
        while (vetor[i].nota < pivo.nota)
        {
            i++;
            if (metricas)
                metricas->comparacoes++;
        }
        if (metricas)
            metricas->comparacoes++;
        while (vetor[j].nota > pivo.nota)
        {
            j--;
            if (metricas)
                metricas->comparacoes++;
        }
        if (i <= j)
        {
            Registro temp = vetor[i];
            vetor[i] = vetor[j];
            vetor[j] = temp;
            i++;
            j--;
        }
    }
    if (esq < j)
        quicksortInterno(vetor, esq, j, metricas);
    if (i < dir)
        quicksortInterno(vetor, i, dir, metricas);
}

void insertionSortFitas(RegFita *vetor, int tamanho, Metricas *metricas)
{
    for (int i = 1; i < tamanho; i++)
    {
        RegFita chave = vetor[i];
        int j = i - 1;

        while (j >= 0)
        {
            if (metricas)
                metricas->comparacoes++;
            // Ordena crescentemente pela nota
            if (vetor[j].reg.nota > chave.reg.nota)
            {
                vetor[j + 1] = vetor[j];
                j--;
            }
            else
            {
                break;
            }
        }
        vetor[j + 1] = chave;
    }
}