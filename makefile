all: desafio

funciones.o: funciones.h funciones.c
	gcc -g -c funciones.c

desafio.o: funciones.h desafio.c
	gcc -g -c desafio.c

desafio: funciones.o desafio.o
	gcc -g -o desafio funciones.o desafio.o

clean:
	rm -f funciones.o desafio.o desafio