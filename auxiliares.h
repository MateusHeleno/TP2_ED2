#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdbool.h>

typedef struct{
    int metodo;
    int qnt_registros;
    int situacao;
    bool p;
} Config;

bool validaEntrada(int argc, char *argv[],Config *config);

#endif