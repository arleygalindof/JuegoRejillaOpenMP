// jugador.c
#include <stdio.h>
#include <string.h>
#include "jugador.h"

void jugador_init(Jugador* j, int id, const char* nombre) {
    j->id               = id;
    j->esTurnoActual    = 0;
    j->totalCandidatas  = 0;
    strncpy(j->nombre, nombre, 31);
    j->nombre[31] = '\0';
}

void jugador_activar(Jugador* j) {
    j->esTurnoActual = 1;
}

void jugador_desactivar(Jugador* j) {
    j->esTurnoActual = 0;
}

void jugador_agregarCandidata(Jugador* j, Jugada jugada) {
    if (j->totalCandidatas < MAX_CANDIDATAS) {
        j->candidatas[j->totalCandidatas++] = jugada;
    }
}

void jugador_limpiarCandidatas(Jugador* j) {
    j->totalCandidatas = 0;
}

void jugador_imprimirNombre(const Jugador* j) {
    printf("%s%s%s", COLORES[j->id - 1], j->nombre, RESET);
}