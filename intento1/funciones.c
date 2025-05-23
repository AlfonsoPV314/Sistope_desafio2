#include <time.h>
#include "funciones.h"

void iniciar_juego(int num_procesos, const int token_inicial,int valor_aletorio) {
    pid_t pid;
    int i;
    int token = -1; // Token que usan los hijos para jugar
    int response;   // Respuesta que se recibe de los hijos
    int pids[num_procesos]; // Array para almacenar los PIDs de los hijos
    int pipes[num_procesos][2][2]; // [cantidad num_procesos de pipes][0 para pipe padre->hijo, 1 para pipe hijo->padre][0 para lectura, 1 para escritura]
    int pipes_hermanos[num_procesos][2]; // Pipes para comunicación entre hijos

    crear_pipes(num_procesos, pipes, pipes_hermanos);

    printf("[%d] Padre: estoy creando %d procesos hijos\n", getpid(), num_procesos);

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
            printf("[%d] Hola mundo! Soy el proceso hijo de indice %d y mi padre es %d\n", getpid(), i, getppid());

            pid_t pid_siguiente, pid_anterior;

            // Cerrar extremos innecesarios
            for (int j = 0; j < num_procesos; j++) {
                if (j != i) {
                    // Cerrar extremos que no pertenecen al hijo actual
                    close(pipes[j][0][1]); // Cerrar escritura del padre->hijo para otros hijos
                    close(pipes[j][1][0]); // Cerrar lectura del hijo->padre para otros hijos
                }
            }
            close(pipes[i][0][1]); // Cerrar lectura del padre->hijo
            close(pipes[i][1][0]); // Cerrar escritura del hijo->padre

            recibir_pids_hijos(num_procesos, pids, pipes, i);

            // // Modificar el token y enviarlo de vuelta al padre
            // response = getpid();
            // write(pipes[i][1][1], &response, sizeof(response));

            // // Cerrar extremos restantes
            // close(pipes[i][0][0]);
            // close(pipes[i][1][1]);

            int anterior = pipes_hermanos[(i + num_procesos - 1) % num_procesos][0];    // Pipe para comunicación con el hermano anterior
            int siguiente = pipes_hermanos[i % num_procesos][1];  // Pipe para comunicación con el hermano siguiente
            printf("[%d] Soy el hijo %d y me comunico mediante el pipe anterior %d y el siguiente %d\n", getpid(), i, anterior, siguiente);

            // Cerrar extremos innecesarios
            for (int j = 0; j < num_procesos; j++) {
                if (j != i && j != i % num_procesos && j != (i + num_procesos - 1) % num_procesos) {
                    close(pipes_hermanos[j][0]); // Cerrar lectura de pipes que no usa
                    close(pipes_hermanos[j][1]); // Cerrar escritura de pipes que no usa
                }
            }
            close(pipes_hermanos[(i + num_procesos - 1) % num_procesos][1]); // Cerrar escritura del propio pipe de entrada
            close(pipes_hermanos[i % num_procesos][0]); // Cerrar lectura del pipe propio

            // close(pipes_hermanos[(i + 1) % num_procesos][0]); // Cerrar lectura del pipe hacia el siguiente 
            // close(pipes_hermanos[(i + num_procesos - 1) % num_procesos][1]); // Cerrar escritura del pipe hacia el anterior

            // Aquí el hijo usa anterior y siguiente para comunicarse
            jugar(num_procesos, i, token_inicial,valor_aletorio, anterior, siguiente);
            wait(NULL); // Esperar a que el hijo termine
            exit(0);
        } 

        // Proceso padre
        else {
            printf("[%d] Padre: he creado un hijo con PID %d\n", getpid(), pid);
            // Cerrar extremos innecesarios
            close(pipes[i][0][0]); // Cerrar lectura del padre->hijo
            close(pipes[i][1][1]); // Cerrar escritura del hijo->padre
        }
    }

    // Comunicación con cada hijo
    // for (i = 0; i < num_procesos; i++) {

        // Enviar el PID de los hermanos con los que se va a comunicar al hijo
        // enviar_pids_hijos(num_procesos, pids, pipes);

        // Cerrar extremos utilizados
        // close(pipes[i][0][1]);  // Cerrar escritura del padre->hijo
        // close(pipes[i][1][0]);  // Cerrar lectura del hijo->padre

        // // Leer respuesta del hijo
        // read(pipes[i][1][0], &response, sizeof(response));
        // printf("[%d] Recibi esta respuesta del hijo de indice %d: %d\n", getpid(), i, response);
    // }

    enviar_pids_hijos(num_procesos, pids, pipes);

    enviar_token_inicial(num_procesos, pipes, token_inicial);

    // Esperar a todos los hijos
    for (i = 0; i < num_procesos; i++) {
        wait(NULL);
    }

    printf("[%d] Padre: Todos los procesos hijos han terminado\n", getpid());
}

void crear_pipes(int num_procesos, int pipes[][2][2], int pipes_hermanos[][2]) {

    // Crear pipes para cada hijo
    for (int i = 0; i < num_procesos; i++) {
        if (pipe(pipes[i][0]) == -1 || pipe(pipes[i][1]) == -1) {
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

void enviar_pids_hijos(int num_procesos, int pids[], int pipes[][2][2]) {
    for (int i = 0; i < num_procesos; i++) {
        for (int j = 0; j < num_procesos; j++) {
            if (write(pipes[i][0][1], &pids[j], sizeof(pids[j])) == -1) {
                perror("Error al enviar el PID al hijo");
                exit(EXIT_FAILURE);
            }
            printf("[%d] Enviando el PID %d al hijo %d de indice %d\n", getpid(), pids[j], pids[i], i);
        }
    }
    printf("[%d] Padre: envie todos los PIDs de los hijos\n", getpid());
    // for (int i = 0; i < num_procesos; i++)
    // {
    //     printf("%d ", pids[i]);
    // }
    
}


void recibir_pids_hijos(int num_procesos, int pids[], int pipes[][2][2], int id) {
    //printf("[%d] Soy el proceso hijo y estoy recibiendo los PIDs de los hijos\n", getpid());

    // Recibir el PID de hermanos con los que se comunica a demás hijos
    for (int i = 0; i < num_procesos; i++) {
        if(read(pipes[id][0][0], &pids[i], sizeof(pids[i])) <= 0){
            perror("Error al leer el PID del padre");
            exit(EXIT_FAILURE);
        } 
        else {
            printf("[%d] Recibi el PID %d del padre\n", getpid(), pids[i]);
        }
    }
    printf("[%d] Recibi todos los PIDs de los hijos\n", getpid());
}

void enviar_token_inicial(int num_procesos, int pipes[][2][2], int token) {
    
    printf("[%d] Padre: estoy enviando el token inicial %d al hijo inicial\n", getpid(), token);

    // Enviar el token inicial al primer hijo
    write(pipes[0][0][1], &token, sizeof(token));
}

void jugar(int num_procesos, int id, int token_inicial,int M, int anterior, int siguiente) {
    int token = -1;

    if (id == 0) {
        token = token_inicial;
        printf("[%d] Soy el hijo inicial y estoy enviando el token %d\n", getpid(), token);
        write(siguiente, &token, sizeof(token));
    }

    while (1) {
        srand(time(NULL) + getpid()); // Semilla para la función rand()
        // Leer el token del proceso anterior
        if (read(anterior, &token, sizeof(token)) <= 0) {
            perror("Error al leer el token");
            break;
        }
        printf("[%d] Recibí el token %d desde el pipe %d\n", getpid(), token, anterior);

        // Modificar el token
        token -= rand() % M;    // Resta un valor aleatorio entre 0 y M
        printf("[%d] Modifiqué el token a %d\n", getpid(), token);

        // Enviar el token al siguiente proceso
        if (write(siguiente, &token, sizeof(token)) <= 0) {
            perror("Error al enviar el token");
            break;
        }
        //printf("[%d] Envié el token %d hacia el pipe %d. Espera un minuto... el token es negativo? hmmm vamos a verificar\n", getpid(), token, siguiente);
        sleep(1); // Esperar un segundo antes de continuar
        // Condición para terminar el juego
        if (token < 0) {
            printf("[%d] Fin del juego.\n", getpid());
            break;
        }
        // eliminacion(num_procesos, id, anterior, siguiente, token);
    }
}

// La idea es que cuando el token sea negativo, el hijo se elimine a sí mismo y envíe su indice negativo al hermano para que pueda saber que el proceso de ese índice murió y que lo propague a los demás hermanos para que todos lo sepan. Los procesos de indices adyacentes al eliminado deben editar sus pipes de anterior y siguiente (de alguna forma tienen que saber a cual cambiarlos después lol). Luego, pueden activar el mecanismo de elección de líder, en el cual si el id del proceso es el menor, resetea el token y continúa el juego. Los que no son el menor, se quedan esperando a que el líder les envíe el token nuevamente (la funcion elegir_lider no debe hacer nada en ese caso, deben seguir jugando normal).
void eliminacion(int num_procesos, int id, int anterior, int siguiente, int token) {
    if(token < 0){
        //printf("[%d] Soy el proceso hijo y estoy eliminando el proceso %d\n", getpid(), pids[id]);
        // Cerrar los pipes
        close(anterior); // Cerrar escritura del pipe hijo->padre

        id *= -1;

        if(write(siguiente, &id, sizeof(token)) == -1) {
            perror("Error al enviar el token al padre");
            exit(EXIT_FAILURE);
        }

        close(siguiente); // Cerrar lectura del pipe padre->hijo

        // Terminar el proceso hijo
        exit(0);
    }
}



// int** crear_pipes_hijo_hijo() {
//     // Crear dos pipes para comunicación entre hijos
//     int** pipes = malloc(2 * sizeof(int*));
//     pipes[0] = malloc(2 * sizeof(int));
//     pipes[1] = malloc(2 * sizeof(int));

//     // Pipe para comunicación con el hijo anterior
//     if (pipe(pipes[0]) == -1) {
//         perror("Error al crear el pipe hijo->hijo anterior");
//         exit(EXIT_FAILURE);
//     }

//     // Pipe para comunicación con el hijo siguiente
//     if (pipe(pipes[1]) == -1) {
//         perror("Error al crear el pipe hijo->hijo siguiente");
//         exit(EXIT_FAILURE);
//     }

//     return pipes;
// }

// void liberar_pipes_hijo_hijo(int** pipes) {
//     // Cerrar los pipes
//     close(pipes[0][0]); // Cerrar lectura del pipe hijo->hijo anterior
//     close(pipes[0][1]); // Cerrar escritura del pipe hijo->hijo anterior
//     close(pipes[1][0]); // Cerrar lectura del pipe hijo->hijo siguiente
//     close(pipes[1][1]); // Cerrar escritura del pipe hijo->hijo siguiente

//     // Liberar memoria
//     free(pipes[0]);
//     free(pipes[1]);
//     free(pipes);
// }