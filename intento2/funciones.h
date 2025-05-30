#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void iniciar_juego(int num_procesos, const int token_inicial, int valor_aletorio);
void crear_pipes(int num_procesos, int pipes_padre_hijo[][2][2], int** pipes_hermanos);
void enviar_pids_hijos(int num_procesos, int pids[], int pipes_padre_hijo[][2][2]);
void recibir_pids_hijos(int num_procesos, int pids[], int pipes_padre_hijo[][2][2], int id);
void enviar_token_inicial(int num_procesos, int pipes_padre_hijo[][2][2], int token);
void jugar(int num_procesos, int id, const int token_inicial, int M, int anterior, int siguiente, int** pipes_hermanos, int* pids, int pipes_padre_hijo[][2][2]);
void procesar_eliminacion_hermano(int token, int* id, int* anterior, int* siguiente, int*** pipes_hermanos, int* num_procesos, int* pids, int* lider);
void cambiar_pipes(int* anterior, int* siguiente, int* id, int id_proc_elim, int*** pipes_hermanos, int* num_procesos, int* pids);
int** actualizar_pipes_hermanos(int id_eliminado, int** old_pipes_hermanos, int num_procesos);
void eliminar(int num_procesos, int id, int anterior, int siguiente, int token);
int elegir_lider(int num_procesos, int id, int token, int anterior, int siguiente, int nuevo_lider,int token_inicial);
int verificar_uno_vivo(int* pids,int num_procesos);
