#include <time.h>
#include "funciones.h"

// Entradas: Recibe el número de procesos, el token inicial y un valor aleatorio M.
// Salidas: Ninguna, pero inicia el juego y maneja la lógica de los procesos hijos. 
// Descripción: Esta función crea los pipes necesarios, inicia los procesos hijos y 
//              maneja la lógica del juego. Cada hijo juega con un token que se 
//              modifica y se pasa entre ellos. Si un hijo recibe un token negativo,
//              se elimina a sí mismo y notifica a sus hermanos.
void iniciar_juego(int num_procesos, const int token_inicial,int valor_aletorio) {
    pid_t pid;
    int i;
    int token = -1; // Token que usan los hijos para jugar
    int pids[num_procesos]; // Array para almacenar los PIDs de los hijos
    int pipes_padre_hijo[num_procesos][2][2]; // [cantidad num_procesos de pipes][0 para pipe padre->hijo, 1 para pipe hijo->padre][0 para lectura, 1 para escritura]
    int** pipes_hermanos = malloc(num_procesos * sizeof(int*));
    for (int i = 0; i < num_procesos; i++) {
        pipes_hermanos[i] = malloc(2 * sizeof(int)); // Cada pipe tiene dos extremos (lectura y escritura)
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
    int ganador = -1;
    read(pipes_padre_hijo[0][1][0], &ganador, sizeof(ganador));
    wait(NULL); // Esperar al último hijo
    printf("Proceso %d es el ganador\n", ganador);
}

// Entradas: Recibe el número de procesos, un arreglo de pipes para la comunicación padre-hijo y un 
//           arreglo de pipes para la comunicación entre hermanos.
// Salidas: Ninguna, pero crea los pipes necesarios para la comunicación entre el padre y los hijos, 
//          así como entre los hermanos.
// Descripción: Esta función crea los pipes necesarios para la comunicación entre el padre y los hijos,
//              así como entre los hermanos.
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

// Entradas: Recibe el número de procesos, un arreglo de PIDs y un arreglo de pipes para la comunicación padre-hijo.
// Salidas: Ninguna, pero envía los PIDs de los hijos a través de los pipes.
// Descripción: Esta función envía los PIDs de los hijos a través de los pipes para que cada hijo sepa con quién comunicarse.
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

// Entradas: Recibe el número de procesos, un arreglo de PIDs, un arreglo de pipes para la comunicación padre-hijo y el ID del hijo actual.
// Salidas: Ninguna, pero recibe los PIDs de los hijos a través de los pipes.
// Descripción: Esta función recibe los PIDs de los hijos a través de los pipes para que cada hijo sepa con quién comunicarse.
void recibir_pids_hijos(int num_procesos, int pids[], int pipes_padre_hijo[][2][2], int id) {

    // Recibir el PID de hermanos con los que se comunica a demás hijos
    for (int i = 0; i < num_procesos; i++) {
        if(read(pipes_padre_hijo[id][0][0], &pids[i], sizeof(pids[i])) <= 0){
            perror("Error al leer el PID del padre");
            exit(EXIT_FAILURE);
        } 
    }
}

// Entradas: Recibe el número de procesos, un arreglo de pipes para la comunicación padre-hijo y el token inicial.
// Salidas: Ninguna, implementa logica del juego y se envia tokens a traves de los pipes.
// Descripción: Función que maneja la lógica del juego, donde cada hijo juega con un token que se modifica (aleatoriamente) y se pasa entre ellos.
//              Si un hijo recibe un token negativo, se elimina a sí mismo y notifica a sus hermanos.
void jugar(int num_procesos, int id, const int token_inicial, int M, int anterior, int siguiente, int** pipes_hermanos, int* pids, int pipes_padre_hijo[][2][2]) {
    
    int token = -1;
    int token_anterior = -1;
    int id_display = id;
    int cant_proc_vivos = num_procesos;
    // int veces_token_repetido = 0; // Contador de veces que el token se repite
    srand(time(NULL) + getpid()); // Semilla para la función rand()

    if (id == 0) {
        token = token_inicial;
        token -= rand() % M;
        printf("Proceso %d ; Token recibido: %d ; Token resultante: %d\n", id_display, token_inicial, token);
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

        
        int puedo_ser_eliminado = 1;

        // si el token llegó negativo, significa que el proceso con id -token fue eliminado
        if(token < 0){
            
            // si es la 2da vez consecutiva que me llega el mismo token negativo
            if(token_anterior == token) {

                // si soy el lider, sé que todos los procesos hermanos ya se actualizaron, y por ende puedo reiniciar el token y continuar el juego
                if(id == 0) {
                    
                    token = token_inicial; // Reiniciar el token
                    int token_recibido = token;
                    token -= rand() % M;    // Resta un valor aleatorio entre 0 y M
                    printf("Proceso %d ; Token recibido: %d ; Token resultante: %d ", id_display, token_recibido, token);
                }
                
                if(verificar_uno_vivo(&cant_proc_vivos) == 1){
                    printf("Proceso %d es el ganador\n", id_display);
                    write(pipes_padre_hijo[0][1][1], &id_display, sizeof(id_display));
                    exit(0);
                }
                // si NO soy el lider, no hago nada y espero a que el líder reinicie el token
            }
            else{
                // procesar la eliminacion
                procesar_eliminacion_hermano(token, &id, &anterior, &siguiente, &pipes_hermanos, &num_procesos, &cant_proc_vivos);
                if(verificar_uno_vivo(&cant_proc_vivos) == 1){
                    printf("Proceso %d es el ganador\n", id_display);
                    write(pipes_padre_hijo[0][1][1], &id_display, sizeof(id_display));
                    exit(0);
                }
            }
            puedo_ser_eliminado = 0;   // no modifiqué el token
        }
        else {
            // Modificar el token
            int token_recibido = token;
            token -= rand() % M;    // Resta un valor aleatorio entre 0 y M
            printf("Proceso %d ; Token recibido: %d ; Token resultante: %d ", id_display, token_recibido, token);
            if(token < 0) {
                printf("(Proceso %d es eliminado)\n", id_display);
                token = -id - 1;
            } else {
                printf("\n");
                if(verificar_uno_vivo(&cant_proc_vivos) == 1){
                    printf("Proceso %d es el ganador\n", id_display);
                    write(pipes_padre_hijo[0][1][1], &id_display, sizeof(id_display));
                    exit(0);
                }
            }
            puedo_ser_eliminado = 1;
        }

        // Enviar el token al siguiente proceso
        if (write(siguiente, &token, sizeof(token)) <= 0) {
            perror("Error al enviar el token");
            exit(EXIT_FAILURE);
        }
        // sleep(1); // Esperar un segundo antes de continuar

        if(puedo_ser_eliminado == 1 && token < 0){  // si token < 0 y modifiqué el token, perd ==> si es 1 significa que el token entró negativo, si es 0, el token lo hice negativo dentro de este proceso
            eliminar(siguiente);
        }
        token_anterior = token; // Guardar el token anterior para verificar si se eliminó un hermano
    }
}
// Entradas: Recibe un token negativo, un puntero al ID del proceso actual,
//           punteros a los pipes de anterior y siguiente, un puntero a un arreglo de pipes de hermanos,
//           un puntero al número de procesos y un puntero a la cantidad de procesos vivos.
// Salidas: Ninguna, pero actualiza los pipes y el ID del proceso actual.
// Descripción: Esta función procesa la eliminación de un hermano cuando el token es negativo.
void procesar_eliminacion_hermano(int token, int* id, int* anterior, int* siguiente, int*** pipes_hermanos, int* num_procesos, int* cant_proc_vivos) {
    int id_proc_elim = -token - 1;
    *cant_proc_vivos = *cant_proc_vivos - 1; // Marcar el proceso como eliminado
    cambiar_pipes(anterior, siguiente, id, id_proc_elim, pipes_hermanos, num_procesos);
} 

// Entradas: Recibe un puntero a la cantidad de procesos vivos.
// Salidas: Retorna 1 si hay un solo proceso vivo, 0 en caso contrario.
// Descripción: Esta función verifica si hay un solo proceso vivo en el juego.
int verificar_uno_vivo(int* cant_proc_vivos) {

    // Si hay un solo proceso vivo, retornar 1
    if (*cant_proc_vivos == 1) {
        return 1; // Hay solo un proceso vivo
    }
    
    return 0; // Hay más de un proceso vivo
}

// Entradas: Recibe punteros a los pipes de anterior y siguiente, el ID del proceso actual,
//           el ID del proceso eliminado, un puntero a un arreglo de pipes de hermanos y un puntero al número de procesos.
// Salidas: Ninguna, pero actualiza los pipes y el ID del proceso actual.
// Descripción: Esta función actualiza los pipes de comunicación entre los procesos cuando uno de ellos es eliminado.
void cambiar_pipes(int* anterior, int* siguiente, int* id, int id_proc_elim, int*** pipes_hermanos, int* num_procesos) {
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

// Entradas: Recibe el ID del proceso eliminado, un puntero al arreglo viejo de pipes de hermanos y el número de procesos.
// Salidas: Retorna un nuevo arreglo de pipes de hermanos actualizado.
// Descripción: Esta función actualiza el arreglo de pipes de hermanos eliminando el pipe correspondiente al proceso eliminado.
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

// Entradas: Recibe el ID del siguiente proceso.
// Salidas: Ninguna, pero cierra el pipe y termina el proceso actual.
// Descripción: Esta función cierra el pipe del siguiente proceso y termina el proceso actual.
void eliminar(int siguiente) {
    close(siguiente);
    exit(0);
}