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
    if (config->situacao < 1 || config->situacao > 3) { // verifica se o valor recebido é válido
        printf("Situação deve ter os valores entre 1 e 3\n");
        return false;
    }

    if (argc == 5)
        if (strcmp(argv[4], "-P") == 0)
            config->p = 1;

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

void printMetricas(Metricas metricas, Config config) {
    if (config.p) {
        printf("\n================ ARQUIVO ORDENADO (-P) ================\n");
        FILE *resultado = fopen("resultado_final.txt", "r");
        if (resultado) {
            Registro reg;
            while (lerRegistroTexto(resultado, &reg, NULL))
                imprimirRegistro(&reg);

            fclose(resultado);
        }
        else
            printf("Aviso: Arquivo 'resultado_final.txt' nao foi encontrado para impressao.\n");

        printf("========================================================\n");
    }

    printf("\n\t\tMétricas \n");
    printf("  Leituras    : %6d \n", metricas.ler_reg);
    printf("  Escrita     : %6d \n", metricas.escrita_reg);
    printf("  Comparações    : %6d \n", metricas.comparacoes);
    printf("  Tempo Total    : %.6lf s\n\n", metricas.tempo);
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

bool lerRegistroTexto(FILE *arq, Registro *reg, Metricas *metricas) {
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

bool prepararArquivoInicial(Config *config, Metricas *metricas) {
    // Abre o arquivo original
    FILE *origem = fopen("PROVAO.TXT", "r");
    if (!origem) {
        printf("Arquivo PROVAO.TXT nao encontrado no diretório.\n");
        return false;
    }

    /*
        Situacao 1 -> Ordenado ascendentemente
        Cria (se já não existir) o arquivo ascendente.bin e salva os registros do PROVAO.TXT nele de maneira ascendente
    */
    if (config->situacao == 1) {
        // Metricas falsas para a fase do preparo
        Metricas metricasPreparo;
        inicializaMetricas(&metricasPreparo);

        FILE *ascend = fopen("ascendente", "wb+");
        if (!ascend) {
            printf("Erro ao criar/abrir arquivo ascendente.\n");
            return false;
        }

        // Usando o quicksort para fazer a ordenacao inicial solicitada
        quicksortExterno(config, &metricasPreparo, ascend);
        fclose(ascend);
    }

    // Situacao 2 -> Ordenado descendentemente
    else if (config->situacao == 2) {
        FILE *fAsc = fopen("ascendente.bin", "rb");
        if (!fAsc) {
            printf("O arquivo 'ascendente.bin' precisa existir para gerar o descendente.\n");
            printf("Execute o sistema na situacao 1 primeiro.\n");
            return false;
        }

        FILE *fDesc = fopen("descendente.bin", "wb");
        if (!fDesc) {
            fclose(fAsc);
            return false;
        }

        Registro r;
        // Leitura de trás para frente exata usando sizeof(Registro)
        for (int i = 471705 - 1; i >= 0; i--) { // Pega o tamanho total do arquivo original
            fseek(fAsc, i * sizeof(Registro), SEEK_SET);
            fread(&r, sizeof(Registro), 1, fAsc);
            fwrite(&r, sizeof(Registro), 1, fDesc);
        }

        fclose(fAsc);
        fclose(fDesc);
    }

    // Como o PROVAO.TXT já está desordenado aleatoriamente, só transformamos-o em binário
    else if (config->situacao == 3) {
        FILE *random = fopen("random", "wb+");
        if (!random) {
            printf("Erro ao criar/abrir arquivo aleatório.\n");
            return false;
        }
        char linha[150];
            char temp[51];
            Registro reg;

            // Lendo o PROVAO.TXT até o fim para criar a base de dados binária completa
            while (fgets(linha, sizeof(linha), origem) != NULL) {
                if (strlen(linha) < 99)
                    continue; // Ignora linhas em branco ou mal formatadas

                // Inscrição: colunas 1 a 8 do arquivo texto
                strncpy(temp, linha + 0, 8); temp[8] = '\0';
                reg.inscricao = atol(temp);

                // Nota: colunas 10 a 14 do arquivo texto
                strncpy(temp, linha + 9, 5); temp[5] = '\0';
                reg.nota = atof(temp);

                // Estado: colunas 16 e 17 do arquivo texto
                strncpy(reg.estado, linha + 15, 2); reg.estado[2] = '\0';

                // Cidade: colunas 19 a 68 do arquivo texto
                strncpy(reg.cidade, linha + 18, 50); reg.cidade[50] = '\0';

                // Curso: colunas 70 a 99 do arquivo texto
                strncpy(reg.curso, linha + 69, 30); reg.curso[30] = '\0';

                // Grava no cache binário
                fwrite(&reg, sizeof(Registro), 1, random);
            }
            fclose(random);
        }

    return true;
}

void gravarRegistroTexto(FILE *arq, Registro *reg, Metricas *metricas) {
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
        metricas->escrita_reg++;
}

void imprimirRegistro(Registro *reg) {
    printf("%08ld | Nota: %5.1f | %s | %-20s | %-20s\n",
           reg->inscricao,
           reg->nota,
           reg->estado,
           reg->cidade,
           reg->curso);
}

void quicksortInterno(Registro *vetor, int esq, int dir, Metricas *metricas) {
    int i = esq, j = dir;
    Registro pivo = vetor[(esq + dir) / 2];

    while (i <= j) {
        if (metricas)
            metricas->comparacoes++;
        while (vetor[i].nota < pivo.nota) {
            i++;
            if (metricas)
                metricas->comparacoes++;
        }
        if (metricas)
            metricas->comparacoes++;
        while (vetor[j].nota > pivo.nota) {
            j--;
            if (metricas)
                metricas->comparacoes++;
        }
        if (i <= j) {
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

void insertionSortFitas(RegFita *vetor, int tamanho, Metricas *metricas) {
    for (int i = 1; i < tamanho; i++) {
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

bool lerRegistroBinario(FILE *arq, Registro *reg, Metricas *metricas)
{
    if (fread(reg, sizeof(Registro), 1, arq) != 1)
        return false;

    if (metricas)
        metricas->ler_reg++;

    return true;
}

void gravarRegistroBinario(FILE *arq, Registro *reg, Metricas *metricas)
{
    fwrite(reg, sizeof(Registro), 1, arq);

    if (metricas)
        metricas->escrita_reg++;
}