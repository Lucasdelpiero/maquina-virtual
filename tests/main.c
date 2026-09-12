#include <stdio.h>
#include "src/tests_helpers.h"

// Definicion de contadores globales
int tests_ejecutados = 0;
int tests_aprobados = 0;
int tests_fallados = 0;

// Declaracion de las suites de prueba
void correr_tests_inicializador(void);
void correr_tests_operandos(void);
void correr_tests_operadores(void);
void correr_tests_etapa7(void);
void correr_tests_binarios(void);

int main(void) {
    printf("==================================================\n");
    printf("     VMX TEST RUNNER - SUITE DE PRUEBAS\n");
    printf("==================================================\n");

    // Ejecucion de suites
    correr_tests_inicializador();
    correr_tests_operandos();
    correr_tests_operadores();
    correr_tests_etapa7();
    correr_tests_binarios();

    // Resumen final
    printf("\n==================================================\n");
    printf("RESUMEN DE PRUEBAS:\n");
    printf("  Total ejecutadas: %d\n", tests_ejecutados);
    printf("  Aprobadas:        %d\n", tests_aprobados);
    printf("  Falladas:         %d\n", tests_fallados);
    printf("==================================================\n");

    if (tests_fallados == 0) {
        printf("RESULTADO: TODOS LOS TESTS PASARON EXITOSAMENTE.\n\n");
        return 0;
    } else {
        printf("RESULTADO: SE DETECTARON FALLOS EN LA SUITE.\n\n");
        return 1;
    }
}
