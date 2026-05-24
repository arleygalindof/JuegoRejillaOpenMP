# 🎮 Juego de la Rejilla — C + OpenMP

Simulación concurrente de un juego sobre una rejilla donde múltiples jugadores compiten por cerrar una curva sobre el tablero. Implementado en **C** usando **OpenMP** como parte de un análisis comparativo de rendimiento y escalabilidad entre modelos de concurrencia.

> Proyecto académico — Maestría en Ingeniería de Software Concurrente  
> Comparativa: Java Threads vs C + OpenMP

---

## 📐 Descripción del Juego

La rejilla es un **grafo de adyacencia** donde cada nodo es un punto y cada arista es una conexión entre dos puntos vecinos. Los jugadores compiten por ocupar aristas hasta que uno logra formar un **ciclo cerrado** con sus conexiones.

- Cada turno, el jugador activo aplica una jugada oficial
- Mientras tanto, los jugadores inactivos **piensan en paralelo** usando `H` hilos OpenMP cada uno
- Gana el primero en cerrar una curva
- Si el tablero se llena sin ganador, el resultado es **empate**

---

## 🏗️ Arquitectura del Proyecto

```
JuegoRejillaC/
│
├── src/
│   ├── main.c                  # Orquestador principal + PoolJugadas
│   ├── rejilla.h / rejilla.c   # Grafo de adyacencia + visualización
│   ├── jugador.h / jugador.c   # Jugador con color ANSI y candidatas
│   ├── jugada.h  / jugada.c    # Modelo de una jugada (puntoA, puntoB)
│   ├── validador.h / validador.c  # Validación de parámetros
│   └── medidor.h / medidor.c   # Métricas de rendimiento y throughput
│
├── bin/
│   └── juego.exe               # Ejecutable compilado
│
└── Makefile                    # Compilación con gcc + OpenMP
```

---

## 📊 Diagrama de Módulos

```
┌─────────────────────────────────────────┐
│                 main.c                  │
│─────────────────────────────────────────│
│ + main(argc, argv)                      │
│ + pensarCandidatas()   ← OpenMP         │
│ + PoolJugadas (struct) ← thread-safe    │
│                                         │
│ Orquesta ciclo principal, turnos,       │
│ fase paralela OpenMP y métricas         │
└──────────┬──────────────────────────────┘
           │ usa
    ┌──────┴──────────────────────────────────────┐
    │                                             │
    ▼                                             ▼
┌──────────────────────┐         ┌───────────────────────────┐
│      rejilla.h/c     │         │       jugador.h/c          │
│──────────────────────│         │───────────────────────────│
│ - filas: int         │         │ - id: int                  │
│ - columnas: int      │         │ - nombre: char[]           │
│ - totalPuntos: int   │         │ - esTurnoActual: int       │
│ - conexiones[][]     │         │ - candidatas[]: Jugada     │
│ - lock: omp_lock_t   │         │ - totalCandidatas: int     │
│──────────────────────│         │───────────────────────────│
│ + rejilla_init()     │         │ + jugador_init()           │
│ + esConexionValida() │         │ + jugador_activar()        │
│ + estaLibre()        │         │ + jugador_desactivar()     │
│ + conectar()         │         │ + jugador_agregarCandidata │
│ + hayJugadasDisp()   │         │ + jugador_limpiarCandidatas│
│ + tieneCurvaCerrada()│         │ + jugador_imprimirNombre() │
│ + rejilla_imprimir() │         └───────────────────────────┘
│ + rejilla_destruir() │
└──────────────────────┘

┌──────────────────────┐         ┌───────────────────────────┐
│      jugada.h/c      │         │       medidor.h/c          │
│──────────────────────│         │───────────────────────────│
│ - puntoA: int        │         │ - tiempoInicio: double     │
│ - puntoB: int        │         │ - tiempoFin: double        │
│ - jugadorId: int     │         │ - tiemposPorTurno[]        │
│──────────────────────│         │ - totalJugadas: int        │
│ + crearJugada()      │         │ - numeroHilos: int         │
└──────────────────────┘         │───────────────────────────│
                                 │ + medidor_init()           │
┌──────────────────────┐         │ + medidor_iniciar()        │
│     validador.h/c    │         │ + medidor_finalizar()      │
│──────────────────────│         │ + medidor_registrarTurno() │
│ + validarParametros()│         │ + medidor_getTiempoTotalMs │
└──────────────────────┘         │ + medidor_getThroughput()  │
                                 │ + medidor_imprimirReporte()│
                                 └───────────────────────────┘
```

---

## ⚙️ Modelo Concurrente

```
TURNO DE J1 (activo):
  ├── J1 → aplica jugada oficial              (secuencial)
  ├── J2 → #pragma omp parallel num_threads(H) (paralelo OpenMP)
  ├── J3 → #pragma omp parallel num_threads(H) (paralelo OpenMP)
  └── J4 → #pragma omp parallel num_threads(H) (paralelo OpenMP)

TURNO DE J2 (activo):
  ├── J2 → aplica candidatas precalculadas    (secuencial)
  ├── J1 → #pragma omp parallel num_threads(H) (paralelo OpenMP)
  ├── J3 → #pragma omp parallel num_threads(H) (paralelo OpenMP)
  └── J4 → #pragma omp parallel num_threads(H) (paralelo OpenMP)
```

### Sincronización OpenMP

| Recurso | Mecanismo OpenMP |
|---|---|
| `PoolJugadas` (pool global) | `omp_lock_t` — lock explícito |
| `Rejilla.conexiones` | `omp_lock_t` — lock explícito |
| `jugador.candidatas` | `#pragma omp critical(candidatas_section)` |
| Generación aleatoria | `#pragma omp critical(rand_section)` |
| Tiempo de medición | `omp_get_wtime()` — precisión de microsegundos |

### Diferencia clave con Java Threads

| Aspecto | Java Threads | C + OpenMP |
|---|---|---|
| Creación de hilos | `new Thread()` explícito | `#pragma omp parallel` implícito |
| Sincronización | `synchronized` / `Lock` | `omp_lock_t` / `critical` |
| Aleatoriedad | `new Random()` por hilo | `rand()` con `critical` |
| Tiempo | `System.nanoTime()` | `omp_get_wtime()` |
| Memoria | Heap JVM + GC | Manual, sin recolector |
| Overhead hilos | Mayor (objetos JVM) | Menor (hilos POSIX nativos) |

---

## 🚀 Requisitos

### Windows
- **MSYS2** con MinGW-w64 instalado
- **GCC** con soporte OpenMP (`-fopenmp`)
- **make** disponible en PATH

### Linux / macOS
- GCC con OpenMP (`gcc-12` o superior)
- make

### Verificar instalación

```cmd
gcc --version
make --version
```

Si no responden en CMD, agrega al PATH del sistema:
```
C:\msys64\ucrt64\bin
C:\msys64\usr\bin
```

---

## 🔧 Instalación

### Clonar el repositorio

```bash
git clone https://github.com/tu-usuario/JuegoRejillaC.git
cd JuegoRejillaC
```

### Compilar

```cmd
make
```

El `Makefile` ejecuta internamente:

```bash
gcc -Wall -Wextra -O2 -fopenmp src/main.c src/rejilla.c src/jugador.c src/jugada.c src/validador.c src/medidor.c -o bin/juego.exe
```

### Limpiar binarios

```cmd
make clean
```

---

## ▶️ Ejecución

```cmd
bin\juego.exe <filas> <columnas> <jugadores> <hilos>
```

### Ejemplos

```cmd
rem Rejilla 4x4, 2 jugadores, 3 hilos
bin\juego.exe 4 4 2 3

rem Rejilla 5x5, 3 jugadores, 4 hilos
bin\juego.exe 5 5 3 4

rem Rejilla 6x6, 4 jugadores, 8 hilos
bin\juego.exe 6 6 4 8

rem Rejilla minima, 2 jugadores, 1 hilo
bin\juego.exe 3 3 2 1
```

---

## 📋 Parámetros

| Parámetro | Descripción | Mínimo | Máximo |
|---|---|---|---|
| `filas` | Número de filas de la rejilla | 3 | 10 |
| `columnas` | Número de columnas de la rejilla | 3 | 10 |
| `jugadores` | Número de jugadores | 2 | 6 |
| `hilos` | Hilos OpenMP por jugador inactivo por turno | 1 | sin límite |

### Jugadores y colores ANSI

| Jugador | Color ANSI | Código |
|---|---|---|
| Jugador 1 | 🔴 Rojo | `\033[31m` |
| Jugador 2 | 🔵 Azul | `\033[34m` |
| Jugador 3 | 🟢 Verde | `\033[32m` |
| Jugador 4 | 🟡 Amarillo | `\033[33m` |
| Jugador 5 | 🟣 Magenta | `\033[35m` |
| Jugador 6 | 🩵 Cyan | `\033[36m` |

---

## 📈 Métricas de Rendimiento

Al finalizar cada partida se imprime un reporte:
<img width="369" height="168" alt="image" src="https://github.com/user-attachments/assets/1c11ac89-77eb-41c7-b56e-72f9e0add3b0" />

### Análisis de escalabilidad recomendado

Ejecutar con la misma rejilla variando únicamente el número de hilos:

```cmd
bin\juego.exe 6 6 2 1
bin\juego.exe 6 6 2 2
bin\juego.exe 6 6 2 4
bin\juego.exe 6 6 2 8
```

Comparar el throughput resultante para analizar el impacto del paralelismo OpenMP.

---

## 📁 Visualización del Tablero

<img width="826" height="824" alt="image" src="https://github.com/user-attachments/assets/ea42f36b-223d-4ef4-9550-807a8463dd37" />


Las conexiones ocupadas se muestran en el color del jugador:
- Horizontales ocupadas: `═════` en color del jugador
- Verticales ocupadas: `║` en color del jugador

---

## 🔬 Contexto Académico

Este proyecto es la **Implementación 2** de un análisis comparativo de concurrencia:

| Implementación | Tecnología | Modelo de hilos |
|---|---|---|
| Implementación 1 | Java Threads | Hilos explícitos con `Thread` |
| Implementación 2 | C + OpenMP | Hilos implícitos con `#pragma omp parallel` |

Ambas implementaciones ejecutan exactamente la misma lógica para permitir comparación objetiva de:

- Tiempo de ejecución total
- Throughput (jugadas por segundo)
- Escalabilidad al aumentar hilos
- Overhead de sincronización
- Comportamiento cuando el tablero se llena

### Notas sobre OpenMP en Windows

Para que los colores ANSI y caracteres UTF-8 se rendericen correctamente en CMD, el programa activa automáticamente al inicio:

```c
SetConsoleOutputCP(65001);                    // UTF-8
SetConsoleMode(hOut, dwMode |
    ENABLE_VIRTUAL_TERMINAL_PROCESSING);      // ANSI
```

Esto no es necesario en Linux o macOS donde el soporte es nativo.
