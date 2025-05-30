#include <time.h>
#include "funciones.h"

void iniciar_juego(int num_procesos, const int token_inicial,int valor_aletorio) {
    pid_t pid;
    int i;
    int token = -1; // Token que usan los hijos para jugar
    int response;   // Respuesta que se recibe de los hijos
    int pids[num_procesos]; // Array para almacenar los PIDs de los hijos
    int pipes_padre_hijo[num_procesos][2][2]; // [cantidad num_procesos de pipes][0 para pipe padre->hijo, 1 para pipe hijo->padre][0 para lectura, 1 para escritura]
    int** pipes_hermanos = malloc(num_procesos * sizeof(int*));
    for (int i = 0; i < num_procesos; i++) {
        pipes_hermanos[i] = malloc(2 * sizeof(int));
    }
    crear_pipes(num_procesos, pipes_padre_hijo, pipes_hermanos);
    

    for (i = 0; i < num_procesos; i++) {
        pid = fork();
        pids[i] = pid; // Guardar el PID del hijo

        // Verifica si fork() falló
        if (pid < 0) {
            perror("Error al crear el proceso");
            exit(EXIT_FAILURE);
        } 

        // Proceso hijo
        else if (pid == 0) {

            pid_t pid_siguiente, pid_anterior;

            // Cerrar extremos innecesarios
            for (int j = 0; j < num_procesos; j++) {
                if (j != i) {
                    // Cerrar extremos que no pertenecen al hijo actual
                    close(pipes_padre_hijo[j][0][1]); // Cerrar lectura del padre->hijo para otros hijos
                    close(pipes_padre_hijo[j][1][0]); // Cerrar escritura del hijo->padre para otros hijos
                }
            }
            close(pipes_padre_hijo[i][0][1]); // Cerrar lectura del padre->hijo
            close(pipes_padre_hijo[i][1][0]); // Cerrar escritura del hijo->padre
            recibir_pids_hijos(num_procesos, pids, pipes_padre_hijo, i);
            
            int anterior = pipes_hermanos[(i + num_procesos - 1) % num_procesos][0];    // Pipe para comunicación con el hermano anterior
            int siguiente = pipes_hermanos[i % num_procesos][1];  // Pipe para comunicación con el hermano siguiente
    
            close(pipes_hermanos[(i + num_procesos - 1) % num_procesos][1]); // Cerrar escritura del propio pipe de entrada
            close(pipes_hermanos[i % num_procesos][0]); // Cerrar lectura del pipe propio

            // Aquí el hijo usa anterior y siguiente para comunicarse

            jugar(num_procesos, i, token_inicial,valor_aletorio, anterior, siguiente, pipes_hermanos, pids, pipes_padre_hijo);
            
            exit(0);
         }

        // proceso padre
        else{
            close(pipes_padre_hijo[i][0][0]); // Cerrar lectura del padre->hijo
            close(pipes_padre_hijo[i][1][1]); // Cerrar escritura del hijo->padre
        }
    }
    

    // Padre: primero envía los PIDs y luego el token inicial
    enviar_pids_hijos(num_procesos, pids, pipes_padre_hijo);
    
    // Esperar a todos los hijos
    for (i = 0; i < num_procesos - 1; i++) {
        wait(NULL);
    }
    // Padre: leer id enviada por ultimo hijo
    int ganador;
    if(read(pipes_padre_hijo[0][1][0], &ganador, sizeof(ganador)) <= 0){
        perror("Error al leer la pipe ganadora");
        exit(EXIT_FAILURE);
    }
    wait(NULL); // Esperar al último hijo
    printf("!!!Proceso %d es el ganador\n", ganador);
}

void crear_pipes(int num_procesos, int pipes_padre_hijo[][2][2], int **pipes_hermanos) {

    // Crear pipes para cada hijo
    for (int i = 0; i < num_procesos; i++) {
        if (pipe(pipes_padre_hijo[i][0]) == -1 || pipe(pipes_padre_hijo[i][1]) == -1) {
            perror("Error al crear pipes");
            exit(EXIT_FAILURE);
        }
    }

    // Crear pipes para comunicación entre hijos
    for (int i = 0; i < num_procesos; i++) {
        if (pipe(pipes_hermanos[i]) == -1) {
            perror("Error al crear pipes entre hijos");
            exit(EXIT_FAILURE);
        }
    }

}

void enviar_pids_hijos(int num_procesos, int pids[], int pipes_padre_hijo[][2][2]) {
    for (int i = 0; i < num_procesos; i++) {
        for (int j = 0; j < num_procesos; j++) {
            if (write(pipes_padre_hijo[i][0][1], &pids[j], sizeof(pids[j])) <= 0) {
                perror("Error al enviar el PID al hijo");
                exit(EXIT_FAILURE);
            }
        }
    }
}


void recibir_pids_hijos(int num_procesos, int pids[], int pipes_padre_hijo[][2][2], int id) {

    // Recibir el PID de hermanos con los que se comunica a demás hijos
    for (int i = 0; i < num_procesos; i++) {
        if(read(pipes_padre_hijo[id][0][0], &pids[i], sizeof(pids[i])) <= 0){
            perror("Error al leer el PID del padre");
            exit(EXIT_FAILURE);
        } 
    }
}

// void enviar_token_inicial(int num_procesos, int pipes_padre_hijo[][2][2], int token) {

//     // Enviar el token inicial al primer hijo
//     if(write(pipes_padre_hijo[0][0][1], &token, sizeof(token)) <= 0){
//         perror("Error al enviar el token inicial");
//         exit(EXIT_FAILURE);
//     }
// }

// void recibir_token_inicial(int pipes_padre_hijo[][2][2], int* token) {
//     // Leer el token inicial del padre
//     if (read(pipes_padre_hijo[0][0][0], token, sizeof(*token)) <= 0) {
//         perror("Error al leer el token inicial");
//         exit(EXIT_FAILURE);
//     }
// }

void jugar(int num_procesos, int id, const int token_inicial, int M, int anterior, int siguiente, int** pipes_hermanos, int* pids, int pipes_padre_hijo[][2][2]) {
    
    int token = -1;
    int token_anterior = -1;
    int lider = 0;
    int id_display = id;
    int num_procesos_original = num_procesos; // Guardar el número original de procesos
    
    int veces_token_repetido = 0; // Contador de veces que el token se repite
    srand(time(NULL) + getpid()); // Semilla para la función rand()

    if (id == 0) {
        token = token_inicial;
        token -= rand() % M;
        printf("Proceso %d ; Token recibido: %d ; Token resultante: %d\n",id_display, token_inicial, token);
        if(write(siguiente, &token, sizeof(token)) <= 0){
            perror("Error al enviar el primer token");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        // Verificar si soy el unico proceso vivo verificando si hay pids no negativos en el arreglo de pids (por implementar)
        // Leer el token del proceso anterior
        srand(time(NULL) + getpid()); // Semilla para la función rand()

        if (read(anterior, &token, sizeof(token)) <= 0) {
            perror("Error al leer el token");
            exit(EXIT_FAILURE);
        }
        
        int modifique_token = 1;

        // si el token llegó negativo, significa que el proceso con id -token fue eliminado
        if(token < 0){
            //printf("[%d] Proceso %d ; Token recibido: %d\n", getpid(), id_display, token);
            // si es la 2da vez consecutiva que me llega el mismo token negativo
            if(token_anterior == token) {
                veces_token_repetido++;
                // si soy el lider, sé que todos los procesos hermanos ya se actualizaron, y por ende puedo reiniciar el token y continuar el juego
                if(id == /*lider*/ 0 && veces_token_repetido >= 3) {
                    
                    token = token_inicial; // Reiniciar el token
                    veces_token_repetido = 0; // Reiniciar el contador de veces que el token se repite
                }
                // si NO soy el lider, no hago nada y espero a que el líder reinicie el token
            }
            else{
                // procesar la eliminacion
                procesar_eliminacion_hermano(token, &id, &anterior, &siguiente, &pipes_hermanos, &num_procesos, pids, &lider);
                if(verificar_uno_vivo(pids, num_procesos_original) == 1){
                    printf("Proceso %d es el ganador\n", id_display);
                    if(write(pipes_padre_hijo[0][1][1], &id_display, sizeof(id_display)) <= 0){
                        perror("Error al enviar id del ganador");
                        exit(EXIT_FAILURE);
                    }
                    exit(0);
                }
            }
            modifique_token = 0;   // no modifiqué el token
        }
        else {
            // Modificar el token
            // Guardar el token anterior para verificar si se eliminó un hermano
            int token_recibido = token;
            token -= rand() % M;    // Resta un valor aleatorio entre 0 y M
            printf("[%d] Proceso %d ; Token recibido: %d ; Token resultante: %d ", getpid(), id_display, token_recibido, token);
            if(token < 0) {
                printf("(Proceso %d es eliminado)\n", id_display);
                token = -id - 1;
            } else {
                printf("\n");
                if(verificar_uno_vivo(pids, num_procesos_original) == 1){
                    printf("Proceso %d es el ganador\n", id_display);
                    if(write(pipes_padre_hijo[0][1][1], &id_display, sizeof(id_display)) <= 0){
                        perror("Error al enviar id del ganador");
                        exit(EXIT_FAILURE);
                    }
                    exit(0);
                }
            }
            modifique_token = 1;
        }

        // Enviar el token al siguiente proceso
        if (write(siguiente, &token, sizeof(token)) <= 0) {
            perror("Error al enviar el token");
            exit(EXIT_FAILURE);
        }
        // sleep(1); // Esperar un segundo antes de continuar

        if(modifique_token == 1 && token < 0){  // si token < 0 y modifiqué el token, perd ==> si es 1 significa que el token entró negativo, si es 0, el token lo hice negativo dentro de este proceso
            eliminar(num_procesos, id, anterior, siguiente, token);
        }
        token_anterior = token; // Guardar el token anterior para verificar si se eliminó un hermano
    }
}

void procesar_eliminacion_hermano(int token, int* id, int* anterior, int* siguiente, int*** pipes_hermanos, int* num_procesos, int* pids, int* lider) {
    int id_proc_elim = -token - 1;
    pids[id_proc_elim] = -1; // Marcar el proceso como eliminado
    cambiar_pipes(anterior, siguiente, id, id_proc_elim, pipes_hermanos, num_procesos, pids);
} 

int verificar_uno_vivo(int* pids, int num_procesos) {
    
    //printear pids
    // printf("[%d] PIDs actuales: ", getpid());
    // for (int i = 0; i < num_procesos; i++) {
    //     printf("%d ", pids[i]);
    // }
    // printf("\n");    
    
    // Verifica si hay un proceso vivo
    int cantidad_vivos = 0;
    for (int i = 0; i < num_procesos; i++) {
        if (pids[i] != -1) {
            cantidad_vivos++;
        }
    }

    if (cantidad_vivos == 1) {
        // printf("SOLO QUEDO YOOOO!\n");
        return 1; // Hay solo un proceso vivo
    }
    
    return 0; // Hay más de un proceso vivo
}

void cambiar_pipes(int* anterior, int* siguiente, int* id, int id_proc_elim, int*** pipes_hermanos, int* num_procesos, int* pids) {
    int old_id = *id;
    int old_num_p = *num_procesos;
    *num_procesos = *num_procesos - 1; // Reducir el número de procesos en 1

    if(*id > id_proc_elim){
        *id = *id - 1;
    }
    if(((old_id + old_num_p - 1) % old_num_p) == (id_proc_elim % old_num_p)){
        *anterior = (*pipes_hermanos)[(id_proc_elim + old_num_p - 1) % old_num_p][0];
    }
    int** new_pipes_hermanos = actualizar_pipes_hermanos(id_proc_elim, *pipes_hermanos, *num_procesos);
    *pipes_hermanos = new_pipes_hermanos;
}

int** actualizar_pipes_hermanos(int id_eliminado, int** old_pipes_hermanos, int num_procesos) {

    int pipe_eliminada = id_eliminado % (num_procesos + 1); // El arreglo viejo tiene num_procesos+1 pipes

    // Crear el nuevo arreglo de pipes
    int** new_pipes_hermanos = malloc(num_procesos * sizeof(int*));
    for (int i = 0; i < num_procesos; i++) {
        new_pipes_hermanos[i] = malloc(2 * sizeof(int));
    }

    // Copiar todos los pipes excepto el eliminado
    for (int i = 0, j = 0; i < num_procesos + 1; i++) {
        if (i != pipe_eliminada) {
            new_pipes_hermanos[j][0] = old_pipes_hermanos[i][0];
            new_pipes_hermanos[j][1] = old_pipes_hermanos[i][1];
            j++;
        }
    }

    // Liberar la memoria del arreglo viejo
    for (int i = 0; i < num_procesos + 1; i++) {
        free(old_pipes_hermanos[i]);
    }
    free(old_pipes_hermanos);

    return new_pipes_hermanos;
}


// La idea es que cuando el token sea negativo, el hijo se elimine a sí mismo y envíe su indice negativo al hermano para que pueda saber que el proceso de ese índice murió y que lo propague a los demás hermanos para que todos lo sepan. Los procesos de indices adyacentes al eliminado deben editar sus pipes de anterior y siguiente (de alguna forma tienen que saber a cual cambiarlos después lol). Luego, pueden activar el mecanismo de elección de líder, en el cual si el id del proceso es el menor, resetea el token y continúa el juego. Los que no son el menor, se quedan esperando a que el líder les envíe el token nuevamente (la funcion elegir_lider no debe hacer nada en ese caso, deben seguir jugando normal).
void eliminar(int num_procesos, int id, int anterior, int siguiente, int token) {
    close(siguiente);
    exit(0);
}

int elegir_lider(int num_procesos, int id, int token, int anterior, int siguiente, int nuevo_lider,int token_inicial) {
    // Aquí se implementa la lógica para elegir al líder
    // Si el id del proceso es el menor de los vivos, se convierte en líder
    if (id == nuevo_lider) {
        //printf("[%d] Soy el líder y estoy reiniciando el juego\n", getpid());
        token = token_inicial; // Reiniciar el token
        if(write(siguiente, &token, sizeof(token)) <= 0){
            perror("Error al enviar el token");
            exit(EXIT_FAILURE);
        }
    }
}