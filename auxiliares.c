#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxiliares.h"


// codigo de criacao de uma pasta pras fitas
#ifdef _WIN32
#include <direct.h>
#define CRIAR_PASTA(caminho) _mkdir(caminho)
#else
#include <sys/stat.h>
#define CRIAR_PASTA(caminho) mkdir(caminho, 0777)
#endif

bool validaEntrada(int argc, char *argv[], Config *config) {
    if (argc < 4 || argc > 5) {
        printf("O formato é inválido.\nSiga esse formato: ./exe <método> <quantidade> <situação> [-P]\n");
        return false;
    }

    config->metodo = atoi(argv[1]);
    if (config->metodo < 1 || config->metodo > 3) { // verifica se o valor recebido é válido
        printf("Método deve ter os valores entre 1 e 3\n");
        return false;
    }

    config->qnt_registros = atoi(argv[2]);
    if (config->qnt_registros > 471705) {
        printf("A quantidade de registros não pode ser maior do que 471.705!\n");
        return false;
    }
    bool aux = false;
    for (int i = 100; i <= 100000; i = i * 10) {
        if (config->qnt_registros == i || config->qnt_registros == 471705) {
            aux = true;
            break;
        }
        aux = false;
    }

    if (!aux) {
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

        else {
            printf("O quinto argumento deve ser '-P' ou vazio \n");
            return false;
        }
    else
        config->p = 0;

    return true;
}

void inicializaMetricas(Metricas *metricas) {
    metricas->ler_reg = 0;
    metricas->escrita_reg = 0;
    metricas->comparacoes = 0;
    metricas->tempo = 0.0;
}

void printMetricas(Metricas metricas, Config config) {
    if (config.p) {
        printf("\n================ ARQUIVO ORDENADO (-P) ================\n");
        FILE *resultado = fopen("resultado_final.bin", "rb");
        if (resultado) {
            Registro reg;
            while (lerRegistro(resultado, &reg, NULL))
                imprimirRegistro(&reg);

            fclose(resultado);
        }
        else
            printf("Aviso: Arquivo 'resultado_final.bin' nao foi encontrado para impressao.\n");

        printf("========================================================\n");
    }

    printf("\n\t\tMétricas \n");
    printf("  Leituras    : %6d \n", metricas.ler_reg);
    printf("  Escrita     : %6d \n", metricas.escrita_reg);
    printf("  Comparações    : %6d \n", metricas.comparacoes);
    printf("  Tempo Total    : %.6lf s\n\n", metricas.tempo);
}

bool arquivoExiste(const char *nome) {
    FILE *f = fopen(nome, "rb");

    if (f) {
        fclose(f);
        return true;
    }

    return false;
}

long quantidadeRegistros(FILE *arq) {
    fseek(arq, 0, SEEK_END);

    long total = ftell(arq) / sizeof(Registro);

    fseek(arq, 0L, SEEK_SET);

    return total;
}

FILE* abrirArquivo(const char *nome, const char *modo) {
    FILE *arq = fopen(nome, modo);

    if (!arq)
        printf("Erro ao abrir arquivo: %s\n", nome);

    return arq;
}

void criarPastaSeNaoExistir(const char *nome) {
    CRIAR_PASTA(nome);
}

bool lerRegistro(FILE *arq,Registro *reg,Metricas *metricas) {
    if (fread(reg, sizeof(Registro), 1, arq) != 1)
        return false;

    if (metricas)
        metricas->ler_reg++;

    return true;
}

void gravarRegistro(FILE *arq,Registro *reg,Metricas *metricas) {
    fwrite(reg, sizeof(Registro), 1, arq);

    if (metricas)
        metricas->escrita_reg++;
}

void imprimirRegistro(Registro *reg) {
    printf("%ld %.1f %s %s %s\n",
           reg->inscricao,
           reg->nota,
           reg->estado,
           reg->cidade,
           reg->curso);
}

bool abrirFitas(FILE *fitas[],int inicio,int fim,const char *modo) {
    criarPastaSeNaoExistir("fitas");

    char nomeFita[50];

    for (int i = inicio; i < fim; i++) {
        snprintf(nomeFita, sizeof(nomeFita),"fitas/fita%02d.bin", i);

        fitas[i] = abrirArquivo(nomeFita, modo);

        if (!fitas[i])
            return false;
    }

    return true;
}

void fecharFitas(FILE *fitas[],int inicio,int fim) {
    for (int i = inicio; i < fim; i++) {
        if (fitas[i]) {
            fclose(fitas[i]);
            fitas[i] = NULL;
        }
    }
}

bool gerarArquivoRandom(const char *origem) {
    if (arquivoExiste("arquivos/random.bin"))
        return true;

    FILE *arqOrigem = abrirArquivo(origem, "r");

    if (!arqOrigem)
        return false;

    FILE *random = abrirArquivo("arquivos/random.bin", "wb");

    if (!random) {
        fclose(arqOrigem);
        return false;
    }

    char linha[150];
    char temp[51];

    Registro reg;

    while (fgets(linha, sizeof(linha), arqOrigem)) {
        if (strlen(linha) < 99)
            continue;

        strncpy(temp, linha + 0, 8);
        temp[8] = '\0';
        reg.inscricao = atol(temp);

        strncpy(temp, linha + 9, 5);
        temp[5] = '\0';
        reg.nota = atof(temp);

        strncpy(reg.estado, linha + 15, 2);
        reg.estado[2] = '\0';

        strncpy(reg.cidade, linha + 18, 50);
        reg.cidade[50] = '\0';

        strncpy(reg.curso, linha + 69, 30);
        reg.curso[30] = '\0';

        gravarRegistro(random, &reg, NULL);
    }

    fclose(arqOrigem);
    fclose(random);

    return true;
}

bool gerarArquivoAscendente() {
    if (arquivoExiste("arquivos/ascendente.bin"))
        return true;

    FILE *random = abrirArquivo("arquivos/random.bin", "rb");

    if (!random)
        return false;

    long total = quantidadeRegistros(random);

    Registro *vetor = malloc(total * sizeof(Registro));

    if (!vetor) {
        fclose(random);
        return false;
    }

    if(fread(vetor,sizeof(Registro),total,random) != total){
        free(vetor);
        fclose(random);
        return false;
    }

    fclose(random);

    Metricas fake;
    inicializaMetricas(&fake);

    quicksortInterno(vetor,0,total - 1,&fake);

    FILE *asc = abrirArquivo("arquivos/ascendente.bin","wb");

    if (!asc) {
        free(vetor);
        return false;
    }

    fwrite(vetor,sizeof(Registro),total,asc);

    fclose(asc);
    free(vetor);

    return true;
}

bool gerarArquivoDescendente() {
    if (arquivoExiste("arquivos/descendente.bin"))
        return true;

    FILE *asc = abrirArquivo("arquivos/ascendente.bin","rb");

    if (!asc)
        return false;

    long total = quantidadeRegistros(asc);

    Registro *vetor = malloc(total * sizeof(Registro));

    if (!vetor) {
        fclose(asc);
        return false;
    }

    if(fread(vetor,sizeof(Registro),total,asc) != total) {
        free(vetor);
        fclose(asc);
        return false;
    }

    fclose(asc);

    FILE *desc = abrirArquivo("arquivos/descendente.bin","wb");

    if (!desc) {
        free(vetor);
        return false;
    }

    for (long i = total - 1; i >= 0; i--)
        fwrite(&vetor[i],sizeof(Registro),1,desc);


    fclose(desc);
    free(vetor);

    return true;
}

bool prepararArquivoInicial(Config *config) {
    criarPastaSeNaoExistir("arquivos");

    if (!gerarArquivoRandom("PROVAO.TXT"))
        return false;

    if (config->situacao == 3)
        return true;

    if (!gerarArquivoAscendente())
        return false;

    if (config->situacao == 1)
        return true;

    return gerarArquivoDescendente();
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

