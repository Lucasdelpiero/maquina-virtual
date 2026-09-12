#ifndef TESTS_HELPERS_H
#define TESTS_HELPERS_H

#include <stdio.h>
#include "../../src/vmx.h"

// Contadores globales del runner de pruebas
extern int tests_ejecutados;
extern int tests_aprobados;
extern int tests_fallados;

// Macro basica de asercion
#define ASSERT(cond, nombre_test) do { \
    tests_ejecutados++; \
    if (cond) { \
        tests_aprobados++; \
        printf("  [PASS] %s\n", nombre_test); \
    } else { \
        tests_fallados++; \
        printf("  [FAIL] %s (Linea %d)\n", nombre_test, __LINE__); \
    } \
} while (0)

// Macro de igualdad numerica entera
#define ASSERT_EQUAL(obtenido, esperado, nombre_test) do { \
    tests_ejecutados++; \
    if ((obtenido) == (esperado)) { \
        tests_aprobados++; \
        printf("  [PASS] %s\n", nombre_test); \
    } else { \
        tests_fallados++; \
        printf("  [FAIL] %s: esperado %d (0x%X), obtenido %d (0x%X) (Linea %d)\n", \
               nombre_test, (int)(esperado), (int)(esperado), (int)(obtenido), (int)(obtenido), __LINE__); \
    } \
} while (0)

// Comprueba el valor de un registro de la maquina virtual
static inline void assert_registro(Vmx *vmx, int reg, int esperado, char nombre[]) {
    ASSERT_EQUAL(vmx->registros[reg], esperado, nombre);
}

// Comprueba el valor de un byte de la memoria fisica
static inline void assert_memoria(Vmx *vmx, int dir, int esperado, char nombre[]) {
    ASSERT_EQUAL((int)(vmx->memoria[dir] & 0xFF), (esperado & 0xFF), nombre);
}

// Comprueba los 4 bits del registro de condicion (CC: N, Z, C, V)
static inline void assert_cc(Vmx *vmx, int esperado_n, int esperado_z, int esperado_c, int esperado_v, char nombre[]) {
    uint32_t cc = (uint32_t)vmx->registros[CC];
    int n = (int)((cc >> 31) & 1);
    int z = (int)((cc >> 30) & 1);
    int c = (int)((cc >> 29) & 1);
    int v = (int)((cc >> 28) & 1);

    tests_ejecutados++;
    if (n == esperado_n && z == esperado_z && c == esperado_c && v == esperado_v) {
        tests_aprobados++;
        printf("  [PASS] %s (CC: N=%d, Z=%d, C=%d, V=%d)\n", nombre, n, z, c, v);
    } else {
        tests_fallados++;
        printf("  [FAIL] %s: CC esperado (N=%d, Z=%d, C=%d, V=%d), obtenido (N=%d, Z=%d, C=%d, V=%d)\n",
               nombre, esperado_n, esperado_z, esperado_c, esperado_v, n, z, c, v);
    }
}

#endif // TESTS_HELPERS_H
