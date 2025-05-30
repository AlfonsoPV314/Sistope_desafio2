Estudiantes:
    - Alfonso Palacios Vergara (21.775.623-5)
    - Nicolás Sarabia Aguilera (21.637.371-5)

Descripción:

    Este programa implementa comunicación entre procesos hijos en forma de
    anillo, enviando un token a través de pipes. Cada proceso que recibe este
    token debe decrementar su valor en un número aleatorio (entre 0 y M-1). 
    Si el valor resultante es negativo, el proceso se elimina y el líder
    reinicia al valor inicial hasta que quede solo uno vivo. Este último 
    debe informar al proceso padre que ha ganado.

Instrucciones para compilar:

    1) Verificar que se encuentren los archivos necesarios para compilar:
        - desafio2.c
        - funciones.c
        - funciones.h
        - makefile

    2) En el mismo directorio, abrir una terminal y ejecutar:
        $ make all

    3) Para ejecutar el programa:
        $ ./desafio2 -p A -t B -M C
        Donde:
            A: Cantidad de procesos
            B: Token inicial
            C: Valor máximo para generar números aleatorios (entre 0 y C-1)

Consideraciones:
    
    - Programa fue compilado utilizando gcc versión 14.2.0, en un computador
      con sistema operativo Ubuntu 25.04.
    - Para nuestro programa, el líder será el proceso con el ID más pequeño.
    - Existe un problema conocido: el proceso hijo ganador a veces no logra
      comunicar correctamente al padre que es el único vivo. En este caso, el
      proceso hijo imprime que es el ganador. A su vez, el padre también imprime
      pero si hubo un error al enviar/recibir el ID del ganador imprime ganador -1.
      El problema es que el padre solo lee el pipe que tiene con el primer hijo.
    - Otro problema: A veces no se realiza bien el envío del token y se envía 
      repetidamente el mismo valor (la causa aún no ha sido identificada).
    - Otro problema: En algunas instancias se queda estancado el programa.