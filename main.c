#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "auxiliares.h"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    Config config;
    if(!validaEntrada(argc,argv, &config))
        return 1;   

    Metricas metricas;
    clock_t comeco, fim;

    switch (config.metodo)
    {
        case 1:
        {
            /* code */
            break;
        }
        case 2:
        {
        
            break;
        }

        case 3:
        {

            break;
        }
    
    }

    return 0;
}