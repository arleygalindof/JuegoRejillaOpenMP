#ifndef MEDIDOR_H
#define MEDIDOR_H

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_TURNOS 1000

typedef struct {
    double  tiempoInicio;
    double  tiempoFin;
    double  tiemposPorTurno[MAX_TURNOS];
    int     totalJugadas;
    int     numeroHilos;
} MedidorRendimiento;

double  obtenerTiempo();
void    medidor_init(MedidorRendimiento* m, int hilos);
void    medidor_iniciar(MedidorRendimiento* m);
void    medidor_finalizar(MedidorRendimiento* m);
void    medidor_registrarTurno(MedidorRendimiento* m, double tiempoSeg);
double  medidor_getTiempoTotalMs(MedidorRendimiento* m);
double  medidor_getPromedioTurnoMs(MedidorRendimiento* m);
double  medidor_getThroughput(MedidorRendimiento* m);
void    medidor_imprimirReporte(MedidorRendimiento* m);

#endif