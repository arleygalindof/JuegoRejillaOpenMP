#include <stdio.h>
#include "validador.h"

#define MAX_FILAS     10
#define MAX_COLUMNAS  10
#define MAX_JUGADORES  6

int validarParametros(int filas, int columnas,
                      int jugadores, int hilos) {

    if (filas < 3) {
        printf("[ERROR] Filas invalidas: minimo 3. Recibido: %d\n", filas);
        return 0;
    }
    if (filas > MAX_FILAS) {
        printf("[ERROR] Filas invalidas: maximo %d. Recibido: %d\n",
               MAX_FILAS, filas);
        return 0;
    }
    if (columnas < 3) {
        printf("[ERROR] Columnas invalidas: minimo 3. Recibido: %d\n", columnas);
        return 0;
    }
    if (columnas > MAX_COLUMNAS) {
        printf("[ERROR] Columnas invalidas: maximo %d. Recibido: %d\n",
               MAX_COLUMNAS, columnas);
        return 0;
    }
    if (jugadores < 2) {
        printf("[ERROR] Jugadores invalidos: minimo 2. Recibido: %d\n", jugadores);
        return 0;
    }
    if (jugadores > MAX_JUGADORES) {
        printf("[ERROR] Jugadores invalidos: maximo %d. Recibido: %d\n",
               MAX_JUGADORES, jugadores);
        return 0;
    }
    if (hilos < 1) {
        printf("[ERROR] Hilos invalidos: minimo 1. Recibido: %d\n", hilos);
        return 0;
    }
    return 1;
}