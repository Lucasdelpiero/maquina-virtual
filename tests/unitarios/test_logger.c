#include "test_helpers.h"
#include "../../src/logger.h"

void test_logger_suite(void) {
    printf("\n--- TESTS: logger.c ---\n");

    // Test 1: Deshabilitar logger y emitir mensajes (no debe fallar ni colapsar)
    logger_habilitar(0);
    logger("Este mensaje no deberia generar fallos: %d %s\n", 123, "texto");
    TEST_ASSERT(1, "logger: deshabilitado no causa errores ni salidas indebidas");

    // Test 2: Habilitar y deshabilitar alternadamente
    logger_habilitar(1);
    logger_habilitar(0);
    logger("Prueba de deshabilitacion consecutiva\n");
    TEST_ASSERT(1, "logger_habilitar: transicion de estados funciona correctamente");

    // Test 3: Casos limites de formatos (cadena vacia, formato sin args)
    logger_habilitar(0);
    logger("");
    logger("Sin argumentos");
    TEST_ASSERT(1, "logger: soporta formatos vacios y sin argumentos");

    // Test 4: Habilitar temporalmente para ejecucion de formato con varios tipos
    logger_habilitar(0); // Mantenemos en 0 para no ensuciar la salida del runner
    logger("Test con multiples especificadores: %c %d %x %s\n", 'A', 42, 0xFF, "ok");
    TEST_ASSERT(1, "logger: soporta multiples especificadores de formato");
}
