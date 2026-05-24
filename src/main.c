// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#include "rejilla.h"
#include "jugador.h"
#include "jugada.h"
#include "validador.h"
#include "medidor.h"

#ifdef _WIN32
#include <windows.h>
#endif

// Pool global de candidatas (equivalente a ListaJugadas.java)
#define MAX_POOL 1000

typedef struct {
    Jugada  jugadas[MAX_POOL];
    int     total;
    omp_lock_t lock;
} PoolJugadas;

void pool_init(PoolJugadas* p) {
    p->total = 0;
    omp_init_lock(&p->lock);
}

void pool_agregar(PoolJugadas* p, Jugada j) {
    omp_set_lock(&p->lock);
    if (p->total < MAX_POOL)
        p->jugadas[p->total++] = j;
    omp_unset_lock(&p->lock);
}

void pool_limpiar(PoolJugadas* p) {
    omp_set_lock(&p->lock);
    p->total = 0;
    omp_unset_lock(&p->lock);
}

void pool_destruir(PoolJugadas* p) {
    omp_destroy_lock(&p->lock);
}

// ══════════════════════════════════════════
// FASE PARALELA — OpenMP
// Equivalente a MotorConcurrente.java
// ══════════════════════════════════════════

void pensarCandidatas(Jugador* jugadorInactivo,
                      PoolJugadas* pool,
                      Rejilla* rejilla,
                      int hilos,
                      int candidatasPorHilo) {

    printf("\n  >> %s%s%s pensando con %d hilo(s)...\n",
           COLORES[jugadorInactivo->id - 1],
           jugadorInactivo->nombre,
           RESET,
           hilos);

    #pragma omp parallel num_threads(hilos)
    {
        int tid = omp_get_thread_num();

        // Semilla unica por hilo usando tiempo + id de hilo
        srand((unsigned int)(time(NULL) + tid * 31337));

        int generadas = 0;

        while (generadas < candidatasPorHilo) {

            int puntoA, puntoB;

            // Generar posicion aleatoria de forma thread-safe
            #pragma omp critical(rand_section)
            {
                puntoA = rand() % rejilla->totalPuntos;
                puntoB = rand() % rejilla->totalPuntos;
            }

            if (puntoA == puntoB) continue;
            if (!rejilla_esConexionValida(rejilla, puntoA, puntoB)) continue;
            if (!rejilla_estaLibre(rejilla, puntoA, puntoB)) continue;

            Jugada j = crearJugada(puntoA, puntoB, jugadorInactivo->id);

            pool_agregar(pool, j);

            #pragma omp critical(candidatas_section)
            {
                jugador_agregarCandidata(jugadorInactivo, j);
            }

            printf("  %s%s%s | Hilo %d | candidata: %d -> %d\n",
                   COLORES[jugadorInactivo->id - 1],
                   jugadorInactivo->nombre,
                   RESET,
                   tid,
                   puntoA,
                   puntoB);

            generadas++;
        }
    }
}

// ══════════════════════════════════════════
// MAIN
// ══════════════════════════════════════════

int main(int argc, char* argv[]) {

    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif    

    // ── 1. PARAMETROS ──────────────────────

    if (argc != 5) {
        printf("[ERROR] Uso correcto:\n");
        printf("  ./juego <filas> <columnas> <jugadores> <hilos>\n");
        printf("  Ejemplo: ./juego 4 4 2 3\n");
        return 1;
    }

    int filas     = atoi(argv[1]);
    int columnas  = atoi(argv[2]);
    int jugadores = atoi(argv[3]);
    int hilos     = atoi(argv[4]);

    // ── 2. VALIDACION ──────────────────────

    if (!validarParametros(filas, columnas, jugadores, hilos)) {
        return 1;
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║       JUEGO DE LA REJILLA (C+OMP)    ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Rejilla    : %dx%-22d║\n", filas, columnas);
    printf("║  Jugadores  : %-23d║\n", jugadores);
    printf("║  Hilos      : %-23d║\n", hilos);
    printf("╚══════════════════════════════════════╝\n");

    // ── 3. INICIALIZACION ──────────────────

    Rejilla rejilla;
    rejilla_init(&rejilla, filas, columnas);

    Jugador listaJugadores[MAX_JUGADORES];
    char nombreBuf[32];
    for (int i = 0; i < jugadores; i++) {
        snprintf(nombreBuf, 32, "Jugador%d", i + 1);
        jugador_init(&listaJugadores[i], i + 1, nombreBuf);
    }

    PoolJugadas pool;
    pool_init(&pool);

    MedidorRendimiento medidor;
    medidor_init(&medidor, hilos);

    int turnoActual      = 0;
    int juegoTerminado   = 0;
    int jugadorGanadorId = -1;

    // ── 4. CICLO PRINCIPAL ─────────────────

    medidor_iniciar(&medidor);

    while (!juegoTerminado) {

        Jugador* jugadorActivo = &listaJugadores[turnoActual % jugadores];
        jugador_activar(jugadorActivo);

        printf("\n╔══════════════════════════════════════╗\n");
        printf("║  TURNO %-2d - %s%-24s%s║\n",
               turnoActual + 1,
               COLORES[jugadorActivo->id - 1],
               jugadorActivo->nombre,
               RESET);
        printf("╚══════════════════════════════════════╝\n");

        double inicioTurno = obtenerTiempo();

        // ── 4.1 FASE PARALELA ──────────────
        // Jugadores INACTIVOS piensan con OpenMP

        pool_limpiar(&pool);

        for (int i = 0; i < jugadores; i++) {

            if (listaJugadores[i].id == jugadorActivo->id) continue;

            jugador_limpiarCandidatas(&listaJugadores[i]);

            pensarCandidatas(
                &listaJugadores[i],
                &pool,
                &rejilla,
                hilos,
                3   // candidatas por hilo
            );
        }

        // ── 4.2 FASE SECUENCIAL ────────────
        // Jugador activo aplica candidatas precalculadas

        int jugadaAplicada = 0;

        // Usar candidatas propias precalculadas
        // Si no tiene, usar el pool global como fallback
        Jugada* candidatas  = jugadorActivo->candidatas;
        int     totalCandidatas = jugadorActivo->totalCandidatas;

        if (totalCandidatas == 0) {
            candidatas      = pool.jugadas;
            totalCandidatas = pool.total;
        }

        printf("\n  >> %s%s%s aplica su jugada oficial...\n",
               COLORES[jugadorActivo->id - 1],
               jugadorActivo->nombre,
               RESET);

        for (int i = 0; i < totalCandidatas && !jugadaAplicada; i++) {

            Jugada c = candidatas[i];

            if (rejilla_estaLibre(&rejilla, c.puntoA, c.puntoB)) {

                if (rejilla_conectar(&rejilla,
                                     c.puntoA,
                                     c.puntoB,
                                     jugadorActivo->id)) {

                    printf("  Jugada oficial: %d -> %d por %s%s%s\n",
                           c.puntoA, c.puntoB,
                           COLORES[jugadorActivo->id - 1],
                           jugadorActivo->nombre,
                           RESET);

                    jugadaAplicada = 1;
                }
            }
        }

        pool_limpiar(&pool);

        // ── 4.3 VERIFICACION ───────────────

        double finTurno = obtenerTiempo();
        medidor_registrarTurno(&medidor, finTurno - inicioTurno);

        rejilla_imprimir(&rejilla);

        if (!jugadaAplicada || !rejilla_hayJugadasDisponibles(&rejilla)) {
            printf("\n  EMPATE: no hay mas jugadas disponibles.\n");
            juegoTerminado = 1;
            break;
        }

        if (rejilla_tieneCurvaCerrada(&rejilla, jugadorActivo->id)) {
            printf("\n  VICTORIA: %s%s%s cerro una curva!\n",
                   COLORES[jugadorActivo->id - 1],
                   jugadorActivo->nombre,
                   RESET);
            jugadorGanadorId = jugadorActivo->id;
            juegoTerminado   = 1;
            break;
        }

        jugador_desactivar(jugadorActivo);
        turnoActual++;
    }

    // ── 5. RESULTADO Y METRICAS ────────────

    medidor_finalizar(&medidor);

    printf("\n╔══════════════════════════════════════╗\n");
    if (jugadorGanadorId != -1) {
        printf("║  GANADOR: %s%-27s%s║\n",
               COLORES[jugadorGanadorId - 1],
               listaJugadores[jugadorGanadorId - 1].nombre,
               RESET);
    } else {
        printf("║  RESULTADO: EMPATE                   ║\n");
    }
    printf("╚══════════════════════════════════════╝\n");

    medidor_imprimirReporte(&medidor);

    // Liberar recursos
    rejilla_destruir(&rejilla);
    pool_destruir(&pool);

    return 0;
}