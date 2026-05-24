// jugada.h
#ifndef JUGADA_H
#define JUGADA_H

typedef struct {
    int puntoA;
    int puntoB;
    int jugadorId;
} Jugada;

Jugada crearJugada(int puntoA, int puntoB, int jugadorId);

#endif