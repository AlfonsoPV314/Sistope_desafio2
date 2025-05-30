#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "funciones.h"

int main(int argc, char *argv[]) {

    int p = 0, M = 0, t = 0;
    int opt;
    
    while((opt = getopt(argc, argv, "p:M:t:")) != -1) {
        switch(opt) {
            case 'p':
                p = atoi(optarg);
                break;
            case 'M':
                M = atoi(optarg);
                break;
            case 't':
                t = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Uso: %s -p <num_procesos> -M <num_maximo> -t <token_inicial>\n", argv[0]);
                return 1;
        }
    }
    if (p <= 0 || M <= 0 || t < 0) {
        fprintf(stderr, "Error: Los valores de -p y -M deben ser mayores que 0 y -t debe ser mayor o igual a 0.\n");
        return 1;
    }

    iniciar_juego(p, t, M);
    return 0;
}