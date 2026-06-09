#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxiliares.h"

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
    bool aux;
    for (int i = 100; i <= 100000; i = i * 10){
        if(config->qnt_registros == i || config->qnt_registros == 471705){
            aux = true;
            break;
        }
        aux = false;
    }

    if(!aux){
        printf("A quantidade de registros deve ser exatamente: 100, 1000, 10000, 100000 ou 471705.\n");
        return false;
    }

    config->situacao = atoi(argv[3]);
    if (config->situacao < 1 || config->situacao > 3) { // verifica se o valor recebido é válido
        printf("Situação deve ter os valores entre 1 e 3\n");
        return false;
    }

    if (argc == 5)
        if (strcmp(argv[4], "-P") == 0) {
            config->p = 1;
        } else {
            printf("O quinto argumento deve ser '-P' ou vazio \n");
            return false;
        }
    else
        config->p = 0;

    return true;
}