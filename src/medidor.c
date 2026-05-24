#include <stdio.h>
#include <omp.h>
#include "medidor.h"

// ══════════════════════════════════════════
// TEMPORIZADOR DE ALTA RESOLUCION
// ══════════════════════════════════════════

double obtenerTiempo() {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
#else
    return omp_get_wtime();
#endif
}

// ══════════════════════════════════════════
// MEDIDOR
// ══════════════════════════════════════════

void medidor_init(MedidorRendimiento* m, int hilos) {
    m->numeroHilos  = hilos;
    m->totalJugadas = 0;
    m->tiempoInicio = 0;
    m->tiempoFin    = 0;
}

void medidor_iniciar(MedidorRendimiento* m) {
    m->tiempoInicio = obtenerTiempo();
}

void medidor_finalizar(MedidorRendimiento* m) {
    m->tiempoFin = obtenerTiempo();
}

void medidor_registrarTurno(MedidorRendimiento* m, double tiempoSeg) {
    if (m->totalJugadas < MAX_TURNOS) {
        m->tiemposPorTurno[m->totalJugadas] = tiempoSeg;
        m->totalJugadas++;
    }
}

double medidor_getTiempoTotalMs(MedidorRendimiento* m) {
    return (m->tiempoFin - m->tiempoInicio) * 1000.0;
}

double medidor_getPromedioTurnoMs(MedidorRendimiento* m) {
    if (m->totalJugadas == 0) return 0;
    double suma = 0;
    for (int i = 0; i < m->totalJugadas; i++)
        suma += m->tiemposPorTurno[i];
    return (suma / m->totalJugadas) * 1000.0;
}

double medidor_getThroughput(MedidorRendimiento* m) {
    double total = m->tiempoFin - m->tiempoInicio;
    if (total == 0) return 0;
    return m->totalJugadas / total;
}

void medidor_imprimirReporte(MedidorRendimiento* m) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║      REPORTE DE RENDIMIENTO          ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Hilos configurados  : %-14d║\n", m->numeroHilos);
    printf("║  Total jugadas       : %-14d║\n", m->totalJugadas);
    printf("║  Tiempo total (ms)   : %-14.3f║\n", medidor_getTiempoTotalMs(m));
    printf("║  Promedio/turno (ms) : %-14.3f║\n", medidor_getPromedioTurnoMs(m));
    printf("║  Throughput (j/s)    : %-14.2f║\n", medidor_getThroughput(m));
    printf("╚══════════════════════════════════════╝\n");
}