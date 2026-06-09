#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "auxiliares.h"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    Config config;
    if(!validaEntrada(argc,argv, &config))
        return 1;

    return 0;
}