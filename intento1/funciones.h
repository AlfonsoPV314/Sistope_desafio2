#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void iniciar_juego(int num_procesos);
void enviar_pids_hijos(int num_procesos, int pids[], int pipes[][2][2]);
void enviar_token_inicial(int num_procesos, int pipes[][2][2], int token);
void jugar(int num_procesos, int id);