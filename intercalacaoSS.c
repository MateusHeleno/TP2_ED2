#include <stdio.h>
#include <stdlib.h>
#include "auxiliares.h"
#include "intercalacaoSS.h"

// fase de pre-processamento gera blocos de tamanhos variados nas fitas
void gerarBlocosOrdenadosSubstituicao(const char *nomeArquivo, int quantidade, Metricas *metricas)
{
    FILE *arqEntrada = fopen(nomeArquivo, "r");
    if (!arqEntrada)
    {
        printf("Erro ao abrir arquivo base: %s\n", nomeArquivo);
        return;
    }

    FILE *fitas[40];
    if (!abrirFitas(fitas, 0, 20, "w"))
    {
        printf("Erro ao criar fitas temporarias.\n");
        fclose(arqEntrada);
        return;
    }

    MinHeap heap; // vetor de 20 posicoes do tipo NoHeap
    heap.tamanho = 0;
    int lidos_total = 0;

    // preenche o primeiro heap
    while (heap.tamanho < 20 && lidos_total < quantidade)
    {
        if (lerRegistroTexto(arqEntrada, &heap.dados[heap.tamanho].reg, metricas))
        {
            heap.dados[heap.tamanho].fita_origem = 0;
            heap.dados[heap.tamanho].marcado = false;
            heap.tamanho++;
            lidos_total++;
        }
        else
        {
            break;
        }
    }

    construirMinHeap(&heap, metricas); // constroi o heap com o vetor preenchido
    int fita_atual = 0;

    while (heap.tamanho > 0)
    {
        // o menor elemento nao marcado fica na raiz
        NoHeap menor = heap.dados[0];
        // grava o menor registro no bloco
        gravarRegistroTexto(fitas[fita_atual], &menor.reg, metricas);

        // le o proximo registro
        Registro proximo;
        if (lidos_total < quantidade && lerRegistroTexto(arqEntrada, &proximo, metricas))
        {
            lidos_total++;
            NoHeap novoNo;
            novoNo.reg = proximo;
            novoNo.fita_origem = 0;

            if (metricas)
                metricas->comparacoes++;

            //  se a nota do registro  que entrar for maior que a nota do reg que saiu nos marcamos ele
            if (proximo.nota < menor.reg.nota)
            {
                novoNo.marcado = true;
            }
            else
            {
                novoNo.marcado = false;
            }

            substituirRaiz(&heap, novoNo, metricas);
        }
        else
        {
            // quando o arquivo original acaba passamos o heap para os blocos
            removerRaiz(&heap, metricas);
        }

        if (heap.tamanho > 0 && heap.dados[0].marcado) // verifica se a raiz esta marcada para definir se todos estao
        {
            fita_atual = (fita_atual + 1) % 20; // pula pra proxima fita

            // desmarca os itens do heap
            for (int i = 0; i < heap.tamanho; i++)
            {
                heap.dados[i].marcado = false;
            }
            // reconstroi o heap
            construirMinHeap(&heap, metricas);
        }
    }

    fecharFitas(fitas, 0, 20);
    fclose(arqEntrada);
}

// intercalacao com heap
void intercalacaoSS(Config *config, Metricas *metricas)
{
    gerarBlocosOrdenadosSubstituicao("entrada_atual.txt", config->qnt_registros, metricas);

    // indices para a leitura e escrita
    int entrada_base = 0;
    int saida_base = 20;

    bool ordenado = false;
    char nomeFita[30]; // armazena o caminho do arquivo

    FILE *fitas_in[20];  // vetor de ponteiros para as fitas de leitura
    FILE *fitas_out[20]; // vetor de ponteiros para as fitas de escrita

    Registro prox_reg[20];  // registro que vai entrar no heap
    bool fita_tem_dado[20]; // indica se tem itens na fita (vetor de flags)

    while (!ordenado)
    {
        // abre as 20 fitas de entrada e faz a primeira leitura de cada fita
        // marca se a fita tem itens tbm
        for (int i = 0; i < 20; i++)
        {
            sprintf(nomeFita, "fitas/fita%02d.txt", entrada_base + i);
            fitas_in[i] = fopen(nomeFita, "r");
            if (fitas_in[i])
            {
                fita_tem_dado[i] = lerRegistroTexto(fitas_in[i], &prox_reg[i], metricas);
            }
            else
            {
                fita_tem_dado[i] = false;
            }
        }

        // abre as 20 fitas de saida em modo de escrita
        for (int i = 0; i < 20; i++)
        {
            sprintf(nomeFita, "fitas/fita%02d.txt", saida_base + i);
            fitas_out[i] = fopen(nomeFita, "w");
        }

        int blocos_gerados = 0; // quantos blocos foram criados
        int saidaAtual = 0;     // marca em qual fita deve ser escrito o proximo reg

        while (1)
        {
            MinHeap heap;     // vetor de 20 posicoes do tipo NoHeap
            heap.tamanho = 0; // gerencia o tamanho do heap

            // inicializa o heap com base nos dados das fitas
            for (int i = 0; i < 20; i++)
            {
                if (fita_tem_dado[i])
                {
                    NoHeap no;
                    no.reg = prox_reg[i];
                    no.fita_origem = i;
                    no.marcado = false;            // nao eh usado nessa fase
                    heap.dados[heap.tamanho] = no; // insere no final do heap
                    heap.tamanho++;
                }
            }

            // se o heap estiver vazio acabou essa varredura
            if (heap.tamanho == 0)
                break;

            // constroi o heap
            construirMinHeap(&heap, metricas);
            blocos_gerados++;
            float ultima_nota = -1.0; // usado para identificar a quebra de bloco

            // gera um bloco ordenado na fita de saida
            while (heap.tamanho > 0)
            {
                NoHeap menor = heap.dados[0]; // menor elemento esta sempre na raiz
                int fita_origem = menor.fita_origem;

                gravarRegistroTexto(fitas_out[saidaAtual], &menor.reg, metricas); // escreve na fita de saida
                ultima_nota = menor.reg.nota;                                     // atualiza a menor nota

                // tenta ler o proximo reg da msm fita que o ultimo saiu
                fita_tem_dado[fita_origem] = lerRegistroTexto(fitas_in[fita_origem], &prox_reg[fita_origem], metricas);

                // se conseguiu ler
                if (fita_tem_dado[fita_origem])
                {
                    if (metricas)
                        metricas->comparacoes++; // contabiliza a comp nas metricas

                    // verifica se o novo reg pertence ao bloco atual
                    // se pertencer, adiciona e refaz o heap
                    if (prox_reg[fita_origem].nota >= ultima_nota)
                    {
                        NoHeap novoNo;
                        novoNo.reg = prox_reg[fita_origem];
                        novoNo.fita_origem = fita_origem;
                        novoNo.marcado = false;
                        substituirRaiz(&heap, novoNo, metricas);
                    }
                    else
                    {
                        // caso o reg atual for menor que o ultimo que saiu, o bloco acabou, continua lendo do heap
                        removerRaiz(&heap, metricas);
                    }
                }
                else
                {
                    // se o arq tiver acabado diminui o heap
                    removerRaiz(&heap, metricas);
                }
            }
            saidaAtual = (saidaAtual + 1) % 20; // passa a saida pra prox fita
        }

        //fecha todas as fitas
        for (int i = 0; i < 20; i++)
        {
            if (fitas_in[i])
                fclose(fitas_in[i]);
            if (fitas_out[i])
                fclose(fitas_out[i]);
        }

        if (blocos_gerados <= 1) // caso so tenha um bloco a ordenacao acabou
        {
            ordenado = true;
            sprintf(nomeFita, "fitas/fita%02d.txt", saida_base);

            // Deleta o resultado final antigo (se houver) e renomeia a fita vencedora
            remove("resultado_final.txt"); 
            rename(nomeFita, "resultado_final.txt");
        }
        else
        {
            // caso o arquivo nao esteja ordenado, trocamos as fitas de entrada com as de saida
            int temp = entrada_base;
            entrada_base = saida_base;
            saida_base = temp;
        }
    }
}
