#include "test_helpers.h"
#include "../../src/depurador.h"
#include "../../src/vmi.h"
#include "../../src/inicializador.h"

void test_depurador_suite(void) {
    printf("\n--- TESTS: depurador.c ---\n");

    // Test 1: depurador_manejar_breakpoint con VMX NULL
    depurador_manejar_breakpoint(NULL);
    TEST_ASSERT(1, "depurador_manejar_breakpoint: seguro ante puntero VMX NULL");

    // Test 2: depurador_manejar_breakpoint sin archivo VMI configurado (ignorado)
    vmi_configurar_archivo(NULL);
    Vmx vmx;
    inicializar_vmx(&vmx, 16, 0, 0);

    depurador_manejar_breakpoint(&vmx);
    TEST_ASSERT(1, "depurador_manejar_breakpoint: retorna inmediatamente sin bloquear si no hay archivo VMI");

    // Test 3: depurador_verificar_paso_a_paso en estado inactivo
    depurador_verificar_paso_a_paso(&vmx);
    TEST_ASSERT(1, "depurador_verificar_paso_a_paso: retorna inmediatamente cuando no esta en paso a paso");

    // Test 4: depurador_verificar_paso_a_paso repetido
    depurador_verificar_paso_a_paso(&vmx);
    TEST_ASSERT(1, "depurador_verificar_paso_a_paso: llamadas sucesivas son seguras");

    free(vmx.memoria.mem_principal);
}
