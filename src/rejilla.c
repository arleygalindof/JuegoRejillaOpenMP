// rejilla.c
#include <stdio.h>
#include <string.h>
#include "rejilla.h"

// ══════════════════════════════════════════
// INICIALIZACION
// ══════════════════════════════════════════

void rejilla_init(Rejilla* r, int filas, int columnas) {
    r->filas       = filas;
    r->columnas    = columnas;
    r->totalPuntos = filas * columnas;

    // Inicializar todas las conexiones como inexistentes
    for (int i = 0; i < r->totalPuntos; i++)
        for (int j = 0; j < r->totalPuntos; j++)
            r->conexiones[i][j] = -1;

    // Habilitar conexiones validas (derecha y abajo)
    for (int fila = 0; fila < filas; fila++) {
        for (int col = 0; col < columnas; col++) {
            int actual = fila * columnas + col;

            if (col < columnas - 1) {
                int derecha = actual + 1;
                r->conexiones[actual][derecha] = 0;
                r->conexiones[derecha][actual] = 0;
            }

            if (fila < filas - 1) {
                int abajo = actual + columnas;
                r->conexiones[actual][abajo] = 0;
                r->conexiones[abajo][actual] = 0;
            }
        }
    }

    omp_init_lock(&r->lock);
}

void rejilla_destruir(Rejilla* r) {
    omp_destroy_lock(&r->lock);
}

// ══════════════════════════════════════════
// CONSULTAS (protegidas con lock)
// ══════════════════════════════════════════

int rejilla_esConexionValida(Rejilla* r, int a, int b) {
    if (a < 0 || b < 0)               return 0;
    if (a >= r->totalPuntos)          return 0;
    if (b >= r->totalPuntos)          return 0;
    return r->conexiones[a][b] != -1;
}

int rejilla_estaLibre(Rejilla* r, int a, int b) {
    if (!rejilla_esConexionValida(r, a, b)) return 0;
    omp_set_lock(&r->lock);
    int libre = r->conexiones[a][b] == 0;
    omp_unset_lock(&r->lock);
    return libre;
}

int rejilla_conectar(Rejilla* r, int a, int b, int jugadorId) {
    omp_set_lock(&r->lock);
    int resultado = 0;
    if (r->conexiones[a][b] == 0) {
        r->conexiones[a][b] = jugadorId;
        r->conexiones[b][a] = jugadorId;
        resultado = 1;
    }
    omp_unset_lock(&r->lock);
    return resultado;
}

int rejilla_hayJugadasDisponibles(Rejilla* r) {
    omp_set_lock(&r->lock);
    int hay = 0;
    for (int i = 0; i < r->totalPuntos && !hay; i++)
        for (int j = i + 1; j < r->totalPuntos && !hay; j++)
            if (r->conexiones[i][j] == 0) hay = 1;
    omp_unset_lock(&r->lock);
    return hay;
}

// ══════════════════════════════════════════
// DETECCION DE VICTORIA (DFS)
// ══════════════════════════════════════════

static int dfs(Rejilla* r, int actual, int padre,
               int inicio, int jugadorId, int* visitado) {
    visitado[actual] = 1;
    for (int vecino = 0; vecino < r->totalPuntos; vecino++) {
        if (r->conexiones[actual][vecino] != jugadorId) continue;
        if (!visitado[vecino]) {
            if (dfs(r, vecino, actual, inicio, jugadorId, visitado))
                return 1;
        } else if (vecino != padre && vecino == inicio) {
            return 1;
        }
    }
    return 0;
}

int rejilla_tieneCurvaCerrada(Rejilla* r, int jugadorId) {
    int visitado[MAX_PUNTOS];
    for (int inicio = 0; inicio < r->totalPuntos; inicio++) {
        memset(visitado, 0, sizeof(int) * r->totalPuntos);
        if (dfs(r, inicio, -1, inicio, jugadorId, visitado))
            return 1;
    }
    return 0;
}

// ══════════════════════════════════════════
// VISUALIZACION
// ══════════════════════════════════════════

static void imprimirHorizontal(int estado) {
    if (estado == 0) {
        printf("\u2500\u2500\u2500\u2500\u2500");
    } else {
        printf("%s\u2550\u2550\u2550\u2550\u2550%s",
               COLORES[estado - 1], RESET);
    }
}

static void imprimirVertical(int estado) {
    if (estado == 0) {
        printf("\u2502");
    } else {
        printf("%s\u2551%s", COLORES[estado - 1], RESET);
    }
}

void rejilla_imprimir(Rejilla* r) {
    printf("\n");
    for (int fila = 0; fila < r->filas; fila++) {

        printf("  ");
        for (int col = 0; col < r->columnas; col++) {
            int punto = fila * r->columnas + col;
            printf("(%2d)", punto);
            if (col < r->columnas - 1) {
                int estado = r->conexiones[punto][punto + 1];
                imprimirHorizontal(estado);
            }
        }
        printf("\n");

        if (fila < r->filas - 1) {
            for (int rep = 0; rep < 3; rep++) {
                printf("  ");
                for (int col = 0; col < r->columnas; col++) {
                    int punto = fila * r->columnas + col;
                    int abajo = punto + r->columnas;
                    int estado = r->conexiones[punto][abajo];
                    printf("  ");
                    imprimirVertical(estado);
                    if (col < r->columnas - 1)
                        printf("      ");
                }
                printf("\n");
            }
        }
    }
    printf("\n");
}