#include "test_helpers.h"
#include "../../src/errores.h"

static int s_custom_handler_llamado = 0;
static char s_ultimo_mensaje_recibido[256] = "";

static void mi_handler_prueba(char mensaje[]) {
    s_custom_handler_llamado = 1;
    if (mensaje != NULL) {
        strncpy(s_ultimo_mensaje_recibido, mensaje, sizeof(s_ultimo_mensaje_recibido) - 1);
        s_ultimo_mensaje_recibido[sizeof(s_ultimo_mensaje_recibido) - 1] = '\0';
    } else {
        s_ultimo_mensaje_recibido[0] = '\0';
    }
}

void test_errores_suite(void) {
    printf("\n--- TESTS: errores.c ---\n");

    // Test 1: Handler personalizado recibe mensaje normal
    s_custom_handler_llamado = 0;
    s_ultimo_mensaje_recibido[0] = '\0';
    set_abortar_handler(mi_handler_prueba);
    abortar("Fallo critico de prueba");
    TEST_ASSERT_EQUAL(s_custom_handler_llamado, 1, "set_abortar_handler: handler personalizado es invocado por abortar");
    TEST_ASSERT_STR_EQUAL(s_ultimo_mensaje_recibido, "Fallo critico de prueba", "abortar: transmite el mensaje correcto al handler");

    // Test 2: Handler personalizado recibe mensaje vacio
    s_custom_handler_llamado = 0;
    abortar("");
    TEST_ASSERT_EQUAL(s_custom_handler_llamado, 1, "abortar: mensaje vacio invoca al handler");
    TEST_ASSERT_STR_EQUAL(s_ultimo_mensaje_recibido, "", "abortar: mensaje vacio transmitido correctamente");

    // Test 3: Handler personalizado recibe NULL
    s_custom_handler_llamado = 0;
    s_ultimo_mensaje_recibido[0] = 'X';
    abortar(NULL);
    TEST_ASSERT_EQUAL(s_custom_handler_llamado, 1, "abortar: mensaje NULL invoca al handler");
    TEST_ASSERT_EQUAL((int)s_ultimo_mensaje_recibido[0], 0, "abortar: mensaje NULL tratado de forma segura");

    // Test 4: Restaurar handler por defecto
    set_abortar_handler(NULL);
    // Verificamos con TEST_ASSERT_ABORTA que el capturador funciona correctamente
    TEST_ASSERT_ABORTA(abortar("Prueba de macro aborta"), "TEST_ASSERT_ABORTA: macro captura correctamente la excepcion");
    TEST_ASSERT_STR_EQUAL(g_ultimo_mensaje_aborto, "Prueba de macro aborta", "TEST_ASSERT_ABORTA: mensaje de aborto registrado");
}
