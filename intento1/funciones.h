#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void iniciar_juego(int num_procesos, const int token_inicial, int valor_aletorio);
void crear_pipes(int num_procesos, int pipes[][2][2], int pipes_hermanos[][2]);
void enviar_pids_hijos(int num_procesos, int pids[], int pipes[][2][2]);
void recibir_pids_hijos(int num_procesos, int pids[], int pipes[][2][2], int id);
void enviar_token_inicial(int num_procesos, int pipes[][2][2], int token);
void jugar(int num_procesos, int id, const int token_inicial,int M, int anterior, int siguiente);
void eliminacion(int num_procesos, int id, int anterior, int siguiente, int token);
// int** crear_pipes_hijo_hijo();
// void liberar_pipes_hijo_hijo(int** pipes);