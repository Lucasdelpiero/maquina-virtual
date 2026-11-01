#include <stdio.h>
#include "test_helpers.h"

// Declaraciones de suites de pruebas paralelas
void test_errores_suite(void);
void test_logger_suite(void);
void test_memoria_suite(void);
void test_operandos_suite(void);
void test_operadores_suite(void);
void test_decodificador_suite(void);
void test_param_segment_suite(void);
void test_sys_suite(void);
void test_vmi_suite(void);
void test_inicializador_suite(void);
void test_disassembler_suite(void);
void test_depurador_suite(void);
void test_vmx_suite(void);

int main(void) {
    printf("======================================================================\n");
    printf("           SUITE DE PRUEBAS UNITARIAS PARALELAS - PROYECTO VM        \n");
    printf("======================================================================\n");

    test_errores_suite();
    test_logger_suite();
    test_memoria_suite();
    test_operandos_suite();
    test_operadores_suite();
    test_decodificador_suite();
    test_param_segment_suite();
    test_sys_suite();
    test_vmi_suite();
    test_inicializador_suite();
    test_disassembler_suite();
    test_depurador_suite();
    test_vmx_suite();

    printf("\n======================================================================\n");
    printf("RESUMEN DE PRUEBAS UNITARIAS PARALELAS:\n");
    printf("  Total ejecutadas: %d\n", g_tests_ejecutados);
    printf("  Aprobadas:        %d\n", g_tests_aprobados);
    printf("  Falladas:         %d\n", g_tests_fallados);
    printf("======================================================================\n");

    if (g_tests_fallados == 0) {
        printf("RESULTADO: TODAS LAS PRUEBAS UNITARIAS PASARON EXITOSAMENTE (100%%).\n\n");
        return 0;
    } else {
        printf("RESULTADO: SE DETECTARON PRUEBAS FALLIDAS.\n\n");
        return 1;
    }
}
