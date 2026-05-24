// jugada.c
#include "jugada.h"

Jugada crearJugada(int puntoA, int puntoB, int jugadorId) {
    Jugada j;
    j.puntoA    = puntoA;
    j.puntoB    = puntoB;
    j.jugadorId = jugadorId;
    return j;
}