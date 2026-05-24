// rejilla.h
#ifndef REJILLA_H
#define REJILLA_H

#include <omp.h>
#include "jugador.h"

#define MAX_PUNTOS 100

typedef struct {
    int filas;
    int columnas;
    int totalPuntos;
    int conexiones[MAX_PUNTOS][MAX_PUNTOS];
    omp_lock_t lock;
} Rejilla;

void    rejilla_init(Rejilla* r, int filas, int columnas);
void    rejilla_destruir(Rejilla* r);
int     rejilla_esConexionValida(Rejilla* r, int a, int b);
int     rejilla_estaLibre(Rejilla* r, int a, int b);
int     rejilla_conectar(Rejilla* r, int a, int b, int jugadorId);
int     rejilla_hayJugadasDisponibles(Rejilla* r);
int     rejilla_tieneCurvaCerrada(Rejilla* r, int jugadorId);
void    rejilla_imprimir(Rejilla* r);

#endif