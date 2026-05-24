CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -fopenmp
SRC     = src/main.c src/rejilla.c src/jugador.c \
          src/jugada.c src/validador.c src/medidor.c
OUT     = bin/juego

all:
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -rf bin/

run:
	./$(OUT) 4 4 2 3