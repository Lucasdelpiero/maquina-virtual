#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "../../src/errores.h"

// Variables globales para contabilidad de pruebas
extern int g_tests_ejecutados;
extern int g_tests_aprobados;
extern int g_tests_fallados;

// Buffer y estado para capturar llamadas a abortar()
extern jmp_buf g_salto_abortar;
extern int g_aborto_disparado;
extern char g_ultimo_mensaje_aborto[512];

// Manejador para atrapar abortos en pruebas unitarias
void capturador_abortos(char mensaje[]);

// Macro basica de asercion booleana
#define TEST_ASSERT(condicion, nombre_prueba) do { \
    g_tests_ejecutados++; \
    if (condicion) { \
        g_tests_aprobados++; \
        printf("  [PASS] %s\n", nombre_prueba); \
    } else { \
        g_tests_fallados++; \
        printf("  [FAIL] %s (Linea %d)\n", nombre_prueba, __LINE__); \
    } \
} while (0)

// Macro de igualdad numerica entera
#define TEST_ASSERT_EQUAL(obtenido, esperado, nombre_prueba) do { \
    g_tests_ejecutados++; \
    if ((int)(obtenido) == (int)(esperado)) { \
        g_tests_aprobados++; \
        printf("  [PASS] %s\n", nombre_prueba); \
    } else { \
        g_tests_fallados++; \
        printf("  [FAIL] %s: esperado %d (0x%X), obtenido %d (0x%X) (Linea %d)\n", \
               nombre_prueba, (int)(esperado), (int)(esperado), (int)(obtenido), (int)(obtenido), __LINE__); \
    } \
} while (0)

// Macro de desigualdad numerica entera
#define TEST_ASSERT_NOT_EQUAL(obtenido, no_esperado, nombre_prueba) do { \
    g_tests_ejecutados++; \
    if ((int)(obtenido) != (int)(no_esperado)) { \
        g_tests_aprobados++; \
        printf("  [PASS] %s\n", nombre_prueba); \
    } else { \
        g_tests_fallados++; \
        printf("  [FAIL] %s: valor inesperado %d (Linea %d)\n", \
               nombre_prueba, (int)(obtenido), __LINE__); \
    } \
} while (0)

// Macro de igualdad de cadenas
#define TEST_ASSERT_STR_EQUAL(obtenido, esperado, nombre_prueba) do { \
    g_tests_ejecutados++; \
    const char *s_obt = (const char*)(obtenido); \
    const char *s_esp = (const char*)(esperado); \
    if (s_obt != NULL && s_esp != NULL && strcmp(s_obt, s_esp) == 0) { \
        g_tests_aprobados++; \
        printf("  [PASS] %s\n", nombre_prueba); \
    } else { \
        g_tests_fallados++; \
        printf("  [FAIL] %s: esperado \"%s\", obtenido \"%s\" (Linea %d)\n", \
               nombre_prueba, (s_esp ? s_esp : "NULL"), (s_obt ? s_obt : "NULL"), __LINE__); \
    } \
} while (0)

// Macro para verificar que un bloque de codigo aborta adecuadamente (edge cases de error)
#define TEST_ASSERT_ABORTA(bloque_codigo, nombre_prueba) do { \
    g_aborto_disparado = 0; \
    g_ultimo_mensaje_aborto[0] = '\0'; \
    set_abortar_handler(capturador_abortos); \
    if (setjmp(g_salto_abortar) == 0) { \
        bloque_codigo; \
        set_abortar_handler(NULL); \
        TEST_ASSERT(0, nombre_prueba); \
    } else { \
        set_abortar_handler(NULL); \
        TEST_ASSERT(g_aborto_disparado == 1, nombre_prueba); \
    } \
} while (0)

#endif // TEST_HELPERS_H
