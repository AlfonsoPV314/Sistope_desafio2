#include <time.h>
#include "funciones.h"

void iniciar_juego(int num_procesos, const int token_inicial,int valor_aletorio) {
    pid_t pid;
    int i;
    int token = -1; // Token que usan los hijos para jugar
    int response;   // Respuesta que se recibe de los hijos
    int pids[num_procesos]; // Array para almacenar los PIDs de los hijos
    int pipes[num_procesos][2][2]; // [cantidad num_procesos de pipes][0 para pipe padre->hijo, 1 para pipe hijo->padre][0 para lectura, 1 para escritura]
    int** pipes_hermanos = malloc(num_procesos * sizeof(int*));
    for (int i = 0; i < num_procesos; i++) {
        pipes_hermanos[i] = malloc(2 * sizeof(int));
        //pipe(pipes_hermanos[i]);
    }
    crear_pipes(num_procesos, pipes, pipes_hermanos);

    //printf("[%d] Padre: estoy creando %d procesos hijos\n", getpid(), num_procesos);

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
            //printf("[%d] Hola mundo! Soy el proceso hijo de indice %d y mi padre es %d\n", getpid(), i, getppid());

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
            //printf("[%d] Soy el hijo %d y me comunico mediante el pipe anterior %d y el siguiente %d\n", getpid(), i, anterior, siguiente);

            // Cerrar extremos innecesarios
            // for (int j = 0; j < num_procesos; j++) {
            //     if (j != i && j != i % num_procesos && j != (i + num_procesos - 1) % num_procesos) {
            //         close(pipes_hermanos[j][0]); // Cerrar lectura de pipes que no usa
            //         close(pipes_hermanos[j][1]); // Cerrar escritura de pipes que no usa
            //     }
            // }
            close(pipes_hermanos[(i + num_procesos - 1) % num_procesos][1]); // Cerrar escritura del propio pipe de entrada
            close(pipes_hermanos[i % num_procesos][0]); // Cerrar lectura del pipe propio

            // close(pipes_hermanos[(i + 1) % num_procesos][0]); // Cerrar lectura del pipe hacia el siguiente 
            // close(pipes_hermanos[(i + num_procesos - 1) % num_procesos][1]); // Cerrar escritura del pipe hacia el anterior

            // Aquí el hijo usa anterior y siguiente para comunicarse
            jugar(num_procesos, i, token_inicial,valor_aletorio, anterior, siguiente, pipes_hermanos, pids);
            
            exit(0);
        } 

        // Proceso padre
        else {
            //printf("[%d] Padre: he creado un hijo con PID %d\n", getpid(), pid);
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
        // //printf("[%d] Recibi esta respuesta del hijo de indice %d: %d\n", getpid(), i, response);
    // }

    enviar_pids_hijos(num_procesos, pids, pipes);

    enviar_token_inicial(num_procesos, pipes, token_inicial);

    // Esperar a todos los hijos
    for (i = 0; i < num_procesos; i++) {
        wait(NULL);
    }

    //printf("[%d] Padre: Todos los procesos hijos han terminado\n", getpid());
}

void crear_pipes(int num_procesos, int pipes[][2][2], int **pipes_hermanos) {

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
    //imprimir pipes_hermanos
    //printf("[%d] Pipes hermanos:\n", getpid());
    for (int i = 0; i < num_procesos; i++) {
        //printf("[%d] Pipe %d: (%d %d) | \n", getpid(), i, pipes_hermanos[i][0], pipes_hermanos[i][1]);
    }

}

void enviar_pids_hijos(int num_procesos, int pids[], int pipes[][2][2]) {
    for (int i = 0; i < num_procesos; i++) {
        for (int j = 0; j < num_procesos; j++) {
            if (write(pipes[i][0][1], &pids[j], sizeof(pids[j])) == -1) {
                perror("Error al enviar el PID al hijo");
                exit(EXIT_FAILURE);
            }
            //printf("[%d] Enviando el PID %d al hijo %d de indice %d\n", getpid(), pids[j], pids[i], i);
        }
    }
    //printf("[%d] Padre: envie todos los PIDs de los hijos\n", getpid());
    // for (int i = 0; i < num_procesos; i++)
    // {
    //     //printf("%d ", pids[i]);
    // }
    
}


void recibir_pids_hijos(int num_procesos, int pids[], int pipes[][2][2], int id) {
    ////printf("[%d] Soy el proceso hijo y estoy recibiendo los PIDs de los hijos\n", getpid());

    // Recibir el PID de hermanos con los que se comunica a demás hijos
    for (int i = 0; i < num_procesos; i++) {
        if(read(pipes[id][0][0], &pids[i], sizeof(pids[i])) <= 0){
            perror("Error al leer el PID del padre");
            exit(EXIT_FAILURE);
        } 
        else {
            //printf("[%d] Recibi el PID %d del padre\n", getpid(), pids[i]);
        }
    }
    //printf("[%d] Recibi todos los PIDs de los hijos\n", getpid());
}

void enviar_token_inicial(int num_procesos, int pipes[][2][2], int token) {
    
    //printf("[%d] Padre: estoy enviando el token inicial %d al hijo inicial\n", getpid(), token);

    // Enviar el token inicial al primer hijo
    write(pipes[0][0][1], &token, sizeof(token));
}

void jugar(int num_procesos, int id, int token_inicial, int M, int anterior, int siguiente, int** pipes_hermanos, int* pids) {
    int token = -1;
    int token_anterior = -1;
    int lider = 0;
    if (id == lider) {
        token = token_inicial;
        //printf("[%d] Soy el hijo inicial y estoy enviando el token %d\n", getpid(), token);
        write(siguiente, &token, sizeof(token));
    }

    


    while (1) {
        srand(time(NULL) + getpid()); // Semilla para la función rand()
        // Verificar si soy el unico proceso vivo verificando si hay pids no negativos en el arreglo de pids (por implementar)
        // Leer el token del proceso anterior
        
        if(verificar_uno_vivo(pids, num_procesos) == 0){
            //printf("[%d] Queda un proceso, terminando el juego...\n", getpid());
            exit(0);
        }

        //printear cuantos procesos vivos hay
        //printf("[%d] Hay %d procesos vivos:\n", getpid(), num_procesos);
        for (int i = 0; i < num_procesos; i++) {
            if (pids[i] != -1) {
                //printf("[%d] Proceso %d con PID %d\n", getpid(), i, pids[i]);
            }
        }

        //printear pipes_hermanos
        //printf("[%d] Pipes hermanos:\n", getpid());
        for (int i = 0; i < num_procesos; i++) {
            //printf("[%d] Pipe %d: (%d %d)\n", getpid(), i, pipes_hermanos[i][0], pipes_hermanos[i][1]);
        }


        if (read(anterior, &token, sizeof(token)) <= 0) {
            //printf("[%d] Error al leer el token del pipe %d\n", getpid(), anterior);
            perror("Error al leer el token");
            exit(EXIT_FAILURE);
        }
        //printf("[%d] Recibí el token %d desde el pipe %d\n", getpid(), token, anterior);
        
        int modifique_token = 1;

        // si el token llegó negativo, significa que el proceso con id -token fue eliminado
        if(token < 0){
            // si es la 2da vez consecutiva que me llega el mismo token negativo
            if(token_anterior == token) {
                // si soy el lider, sé que todos los procesos hermanos ya se actualizaron, y por ende puedo reiniciar el token y continuar el juego
                if(id == /*lider*/ 0){
                    //printf("[%d] Soy el lider y el token no ha cambiado, reiniciando el juego...\n", getpid());
                    token = token_inicial; // Reiniciar el token
                }
                // si NO soy el lider, no hago nada y espero a que el líder reinicie el token
                else{
                    //printf("[%d] No soy el lider y el token no ha cambiado, pasando token sin cambios...\n", getpid());
                }
            }
            else{
                // procesar la eliminacion
                procesar_eliminacion_hermano(token, &id, &anterior, &siguiente, &pipes_hermanos, &num_procesos, pids, &lider);
            }
            modifique_token = 0;   // no modifiqué el token
        }
        else {
            // Modificar el token
            token -= rand() % M;    // Resta un valor aleatorio entre 0 y M
            //printf("[%d] Modifiqué el token a %d\n", getpid(), token);
            modifique_token = 1;
            if(token < 0){
                token = -id - 1;
                //printf("[%d] Token es negativo! enviando token %d (mi id es %d)\n", getpid(), token, id);
            }
        }
            

        // Enviar el token al siguiente proceso
        if (write(siguiente, &token, sizeof(token)) <= 0) {
            //printf("[%d] Error al enviar el token\n",getpid());
            exit(EXIT_FAILURE);
        }

        //printf("[%d] Envié, como proc de indice %d, el token %d hacia el pipe %d de indice %d.\n", getpid(), id, token, siguiente, id % num_procesos);
        sleep(1); // Esperar un segundo antes de continuar

        if(modifique_token == 1 && token < 0){  // si token < 0 y modifiqué el token, perd ==> si es 1 significa que el token entró negativo, si es 0, el token lo hice negativo dentro de este proceso
            eliminar(num_procesos, id, anterior, siguiente, token);
        }
        token_anterior = token; // Guardar el token anterior para verificar si se eliminó un hermano
    }
}

void procesar_eliminacion_hermano(int token, int* id, int* anterior, int* siguiente, int*** pipes_hermanos, int* num_procesos, int* pids, int* lider) {
    int id_proc_elim = -token - 1;
    //printf("[%d] El token es negativo. Procediendo a cambiar pipes (mi id es %d)\n", getpid(), *id);
    pids[id_proc_elim] = -1; // Marcar el proceso como eliminado
    cambiar_pipes(anterior, siguiente, id, id_proc_elim, pipes_hermanos, num_procesos, pids);
    verificacion_lider(id_proc_elim, lider, pids);
} 


void verificacion_lider(int id_proc_elim, int *lider, int* pids) {
    // Solo buscar nuevo líder si el eliminado era el líder actual
    //printf("[%d] Verificando si el proceso eliminado %d era el líder actual %d\n", getpid(), id_proc_elim, *lider);
    if(id_proc_elim == *lider){
        int i = 0;
        int encontrado = 0;
        while (!encontrado) {
            if(pids[i] != -1){
                *lider = i; // El nuevo líder es el proceso con menor id vivo
                //printf("[%d] Nuevo líder es el proceso de id %d\n", getpid(), *lider);
                encontrado = 1;
            }
            i++;
            // Si asumes que siempre queda al menos un proceso vivo, esto es seguro
        }
    }
}

int verificar_uno_vivo(int* pids, int num_procesos) {
    // Verifica si hay al menos un proceso vivo
    for (int i = 0; i < num_procesos; i++) {
        if (pids[i] != -1) {
            return 1; // Hay al menos un proceso vivo
        }
    }
    return 0; // No hay procesos vivos
}

// La logica de esta cosa esta un poco rara, pero la idea es cuando le llega un token negativo a un hijo, marca como -1 el proceso que se elimino, y luego verifica si el id del proceso eliminado es el siguiente o el anterior. Si es el siguiente, entonces cambia el pipe de siguiente por el siguiente del siguiente, y si es el anterior, entonces cambia el pipe de anterior por el anterior del anterior. Pero lo de id no se si es necesario, ya que cuando se elimine un proceso va a quedar un hueco en los ids. Idk, a lo mejor funciona, tengo sueño y no quiero pensar mucho en esto xd.
void cambiar_pipes(int* anterior, int* siguiente, int* id, int id_proc_elim, int*** pipes_hermanos, int* num_procesos, int* pids) {
    //printf("[%d] HOLAAAA CAMBIANDO PIPES\n", getpid());
    int old_id = *id;
    int old_num_p = *num_procesos;
    *num_procesos = *num_procesos - 1; // Reducir el número de procesos en 1
    //printf("[%d] Soy el proceso de id %d y estoy cambiando pipes. Mi id es %d, el id del eliminado es %d, y el numero de procesos es %d (antes de la eliminacion) y %d (despues)\n", getpid(), *id, old_id, id_proc_elim, old_num_p, *num_procesos);

    if(*id > id_proc_elim){
        //printf("[%d] Soy el proceso de id %d y es mayor al id del eliminado %d. Ahora mi id va a ser %d\n", getpid(), *id, id_proc_elim, *id - 1);
        *id = *id - 1;
    }
    if(((old_id + old_num_p - 1) % old_num_p) == (id_proc_elim % old_num_p)){
        //*anterior = pipes_hermanos[(*id + *num_procesos - 1) % *num_procesos][0];
        *anterior = (*pipes_hermanos)[(id_proc_elim + old_num_p - 1) % old_num_p][0];
        //printf("[%d] Cambiando pipe anterior %d de indice %d por %d de indice %d\n", getpid(), (*pipes_hermanos)[(old_id + old_num_p - 1) % old_num_p][0], (old_id + old_num_p - 1) % old_num_p, *anterior, (id_proc_elim + old_num_p - 1) % old_num_p);
    }
    int** new_pipes_hermanos = actualizar_pipes_hermanos(id_proc_elim, *pipes_hermanos, *num_procesos);
    *pipes_hermanos = new_pipes_hermanos;
}

int** actualizar_pipes_hermanos(int id_eliminado, int** old_pipes_hermanos, int num_procesos) {
    //printf("[%d] Entrando a actualizar_pipes_hermanos! id_elim es %d y num_p es %d\n", getpid(), id_eliminado, num_procesos);

    int pipe_eliminada = id_eliminado % (num_procesos + 1); // El arreglo viejo tiene num_procesos+1 pipes
    //printf("[%d] La pipe eliminada es de indice %d\n", getpid(), pipe_eliminada);

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
    //printf("[%d] Perdi! Eliminandome...\n", getpid());
    // int id_neg = -id;
    // write(siguiente, &id_neg, sizeof(id_neg)); // Notifica a los demás
    // close(anterior);
    //printf("[%d] cerrando pipe %d de indice %d\n", getpid(), siguiente, id % num_procesos);
    close(siguiente);
    exit(0);
}

int elegir_lider(int num_procesos, int id, int token, int anterior, int siguiente, int nuevo_lider,int token_inicial) {
    // Aquí se implementa la lógica para elegir al líder
    // Si el id del proceso es el menor de los vivos, se convierte en líder
    if (id == nuevo_lider) {
        //printf("[%d] Soy el líder y estoy reiniciando el juego\n", getpid());
        token = token_inicial; // Reiniciar el token
        write(siguiente, &token, sizeof(token));
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
//     free(pipes[1]);
//     free(pipes);
// }