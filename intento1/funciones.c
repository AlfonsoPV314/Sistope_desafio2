#include "funciones.h"

void iniciar_juego(int num_procesos) {
    pid_t pid;
    int i;
    int token = 123; // Token que se envía a los hijos
    int response;   // Respuesta que se recibe de los hijos
    int pids[num_procesos]; // Array para almacenar los PIDs de los hijos

    // Crear pipes para cada proceso hijo
    int pipes[num_procesos][2][2]; // [cantidad num_procesos de pipes][0 para pipe padre->hijo, 1 para pipe hijo->padre][0 para lectura, 1 para escritura]
    for (i = 0; i < num_procesos; i++) {
        if (pipe(pipes[i][0]) == -1 || pipe(pipes[i][1]) == -1) {
            perror("Error al crear los pipes");
            exit(EXIT_FAILURE);
        }
    }

    printf("[%d] Soy el proceso padre y estoy creando %d procesos hijos\n", getpid(), num_procesos);

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
            printf("[%d] Soy el proceso hijo de indice %d y mi padre es %d\n", getpid(), i, getppid());

            pid_t pid_siguiente, pid_anterior;

            // Cerrar extremos innecesarios
            close(pipes[i][0][1]); // Cerrar escritura del padre->hijo
            close(pipes[i][1][0]); // Cerrar lectura del hijo->padre

            read(pipes[i][0][0], &pid_siguiente, sizeof(token));    // Leer el PID del siguiente hermano
            printf("[%d] Como proceso de indice %d, recibi el PID del hermano siguiente: %d\n", getpid(), i, pid_siguiente);

            read(pipes[i][0][0], &pid_anterior, sizeof(token));   // Leer el PID del hermano anterior
            printf("[%d] Como proceso de indice %d, recibi el PID del herm¿ano anterior: %d\n", getpid(), i, pid_anterior);

            // // Modificar el token y enviarlo de vuelta al padre
            // response = getpid();
            // write(pipes[i][1][1], &response, sizeof(response));

            // // Cerrar extremos restantes
            // close(pipes[i][0][0]);
            // close(pipes[i][1][1]);
            jugar(num_procesos, i);
            exit(0);
        } 

        // Proceso padre
        else {
            printf("[%d] Soy el proceso padre y he creado un hijo con PID %d\n", getpid(), pid);
            // Cerrar extremos innecesarios
            close(pipes[i][0][0]); // Cerrar lectura del padre->hijo
            close(pipes[i][1][1]); // Cerrar escritura del hijo->padre
        }
    }

    // Comunicación con cada hijo
    for (i = 0; i < num_procesos; i++) {

        // Enviar el PID de los hermanos con los que se va a comunicar al hijo
        enviar_pids_hijos(num_procesos, pids, pipes);

        // Cerrar extremos utilizados
        // close(pipes[i][0][1]);  // Cerrar escritura del padre->hijo
        // close(pipes[i][1][0]);  // Cerrar lectura del hijo->padre

        // // Leer respuesta del hijo
        // read(pipes[i][1][0], &response, sizeof(response));
        // printf("[%d] Recibi esta respuesta del hijo de indice %d: %d\n", getpid(), i, response);
    }

    enviar_token_inicial(num_procesos, pipes, token);

    // Esperar a todos los hijos
    for (i = 0; i < num_procesos; i++) {
        wait(NULL);
    }

    printf("[%d] Todos los procesos hijos han terminado\n", getpid());
}

void enviar_pids_hijos(int num_procesos, int pids[], int pipes[][2][2]) {

    //printf("[%d] Soy el proceso padre y estoy enviando los PIDs de los hijos\n", getpid());

    // Enviar el PID de hermanos con los que se comunica al primer hijo (un poco hardcodeado pero funciona)
    printf("[%d] Soy el proceso padre y estoy comunicando con el hijo %d de indice %d\n", getpid(), pids[0], 0);
    write(pipes[0][0][1], &pids[1], sizeof(int)); // Enviar el PID del hijo siguiente
    write(pipes[0][0][1], &pids[num_procesos - 1], sizeof(int)); // Enviar el PID del hijo anterior

    // Enviar el PID de hermanos con los que se comunica a demás hijos
    for (int i = 1; i < num_procesos; i++) {
        printf("[%d] Soy el proceso padre y estoy comunicando con el hijo %d de indice %d\n", getpid(), pids[i], i);

        // si el hijo no es el último
        if (i + 1 < num_procesos) {
            write(pipes[i][0][1], &pids[i + 1], sizeof(int));   // Enviar el PID del hijo siguiente
            write(pipes[i][0][1], &pids[i - 1], sizeof(int));   // Enviar el PID del hijo anterior
        }
        
        // si el hijo es el ultimo
        else {
            write(pipes[i][0][1], &pids[0], sizeof(int));   // Enviar el PID del primer hijo
            write(pipes[i][0][1], &pids[i - 1], sizeof(int));   // Enviar el PID del hijo anterior
        }
    }
}

void enviar_token_inicial(int num_procesos, int pipes[][2][2], int token) {
    for (int i = 0; i < num_procesos; i++) {
        write(pipes[i][0][1], &token, sizeof(token));
    }
}

void jugar(int num_procesos, int id) {
    printf("[%d] Estoy jugando...\n", getpid());
    return;
    // while(1){

    // }
}