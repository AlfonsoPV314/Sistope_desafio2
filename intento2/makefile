all: desafio2

funciones.o: funciones.h funciones.c
	gcc -g -c funciones.c

desafio2.o: funciones.h desafio2.c
	gcc -g -c desafio2.c

desafio2: funciones.o desafio2.o
	gcc -g -o desafio2 funciones.o desafio2.o

clean:
	rm -f funciones.o desafio2.o desafio2