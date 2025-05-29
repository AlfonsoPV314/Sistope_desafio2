#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

void iniciar_juego(int num_procesos, const int token_inicial, int valor_aletorio);
void crear_pipes(int num_procesos, int pipes[][2][2], int pipes_hermanos[][2]);
void enviar_pids_hijos(int num_procesos, int pids[], int pipes[][2][2]);
void recibir_pids_hijos(int num_procesos, int pids[], int pipes[][2][2], int id);
void enviar_token_inicial(int num_procesos, int pipes[][2][2], int token);
void jugar(int num_procesos, int id, const int token_inicial,int M, int anterior, int siguiente, int pipes_hermanos[][2], int* pids);
bool verificar_eliminacion_hermano(int token, int id, int* id_pipes, int anterior, int siguiente, int pipes_hermanos[][2], int num_procesos, int* pids);
void cambiar_pipes(int flag, int* anterior, int* siguiente, int* id_pipes, int token, int pipes_hermanos[][2], int num_procesos, int* pids);
void eliminar(int num_procesos, int id, int anterior, int siguiente, int token);
void elegir_lider(int num_procesos, int id, int token, int anterior, int siguiente);
// int** crear_pipes_hijo_hijo();
// void liberar_pipes_hijo_hijo(int** pipes);