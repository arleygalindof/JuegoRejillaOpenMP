// jugador.h
#ifndef JUGADOR_H
#define JUGADOR_H

#include "jugada.h"

#define MAX_CANDIDATAS  100
#define MAX_JUGADORES   6
#define RESET           "\033[0m"

// Colores ANSI indexados por jugadorId - 1
static const char* COLORES[MAX_JUGADORES] = {
    "\033[31m",   // Rojo     - jugador 1
    "\033[34m",   // Azul     - jugador 2
    "\033[32m",   // Verde    - jugador 3
    "\033[33m",   // Amarillo - jugador 4
    "\033[35m",   // Magenta  - jugador 5
    "\033[36m"    // Cyan     - jugador 6
};

typedef struct {
    int     id;
    char    nombre[32];
    int     esTurnoActual;
    Jugada  candidatas[MAX_CANDIDATAS];
    int     totalCandidatas;
} Jugador;

void    jugador_init(Jugador* j, int id, const char* nombre);
void    jugador_activar(Jugador* j);
void    jugador_desactivar(Jugador* j);
void    jugador_agregarCandidata(Jugador* j, Jugada jugada);
void    jugador_limpiarCandidatas(Jugador* j);
void    jugador_imprimirNombre(const Jugador* j);

#endif